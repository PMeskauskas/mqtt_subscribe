#include <stdio.h>
#include <mosquitto.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <uci.h>
volatile int interrupt = 0;

void sigHandler(int signo) {
    signal(SIGINT, NULL);
    syslog(LOG_INFO, "Received signal: %d", signo);
    interrupt = 1;
}

void cleanup(){
    closelog();
	mosquitto_lib_cleanup();
    exit(1);
}

void usage(void) {
    syslog(LOG_ERR, "Usage: remoteAddress port ");
    cleanup();
}

void mqtt_new(struct mosquitto **mosq)
{
    *mosq = mosquitto_new(NULL, true, NULL);
    if(*mosq == NULL){
		syslog(LOG_ERR, "Error: Out of memory.\n");
		cleanup();
	}
}

void mqtt_connect(struct mosquitto **mosq, char *address, int port)
{
    int rc = MOSQ_ERR_SUCCESS;
    rc = mosquitto_connect(*mosq, address, port, 60);
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(*mosq);
		syslog(LOG_ERR, "Error connecting: %s\n", mosquitto_strerror(rc));
		cleanup();
	}
}

void mqtt_subscribe(struct mosquitto **mosq, char* topicName, int qos)
{
	int rc = MOSQ_ERR_SUCCESS;
	rc = mosquitto_subscribe(*mosq, NULL, topicName, qos);
	if(rc != MOSQ_ERR_SUCCESS) {
		syslog(LOG_ERR, "Error subscribing: %s\n", mosquitto_strerror(rc));
		mosquitto_disconnect(*mosq);
	}
	syslog(LOG_INFO, "subscribed to a topic: %s, QoS: %d", topicName, qos);
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	sleep(1);
	syslog(LOG_INFO,"Message received: %s %d %s\n", msg->topic, msg->qos, (char *)msg->payload);
}
void uci_load_package(struct uci_context *ctx, const char *config_name, struct uci_package **package)
{
    if(uci_load(ctx, config_name, package) !=UCI_OK){
		syslog(LOG_ERR, "Failed to load uci");
		uci_free_context(ctx);
		cleanup();
	}
}
void uci_element_subscribe(struct uci_package *package, struct mosquitto **mosq)
{
    struct uci_element *i, *j;
    uci_foreach_element(&package->sections, i)
	{
		struct uci_section *section = uci_to_section(i);
        char *section_type = section->type;
        bool topicExists = false;
        bool qosExists = false;
        char *topicName = NULL;
        int qos;
		if(strcmp(section_type,"topic")==0){
            uci_foreach_element(&section->options, j)
            {
                struct uci_option *option = uci_to_option(j);
                char *option_name = option->e.name;
                if (!atoi(option->v.string)){
                    topicExists = true;
                    topicName = option->v.string;
                }
                if (qos = atoi(option->v.string)){
                   qosExists = true;
                }
                if(topicExists && qosExists)
                    mqtt_subscribe(mosq, topicName, qos);
            }
		}
	}
}

int main(int argc, char *argv[]) 
{
	const char *config_name = "mqtt_sub";
    struct mosquitto *mosq;
	struct uci_context *ctx = uci_alloc_context();
	struct uci_package *package;
	int rc;
    openlog(NULL, LOG_CONS, LOG_USER);

    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);
	uci_load_package(ctx, config_name, &package);
	
	mosquitto_lib_init();
	mqtt_new(&mosq);

	mosquitto_message_callback_set(mosq, on_message);

	if(argc != 3)
		usage();

	mqtt_connect(&mosq, argv[1], atoi(argv[2]));

	uci_element_subscribe(package,&mosq);

	mosquitto_loop_forever(mosq, -1, 1);
	
	uci_free_context(ctx);
	cleanup();
	return 0;
}