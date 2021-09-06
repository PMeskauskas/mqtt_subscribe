#include <stdio.h>
#include <mosquitto.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

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
    syslog(LOG_ERR, "Usage: remoteAddress port topicToSubscribe QoS ... ...");
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

void mqtt_subscribe(struct mosquitto **mosq, int argc, char* argv[]){
	int rc = MOSQ_ERR_SUCCESS;
	for (int i = 4; i < argc; i=i+2)
	{
		rc = mosquitto_subscribe(*mosq, NULL, argv[i-1], atoi(argv[i]));
		if(rc != MOSQ_ERR_SUCCESS) {
			syslog(LOG_ERR, "Error subscribing: %s\n", mosquitto_strerror(rc));
			mosquitto_disconnect(*mosq);
		}
	}
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	sleep(1);
	syslog(LOG_INFO,"Message received: %s %d %s\n", msg->topic, msg->qos, (char *)msg->payload);
}


int main(int argc, char *argv[]) 
{
    struct mosquitto *mosq;
	int rc;

    openlog(NULL, LOG_CONS, LOG_USER);

    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

	mosquitto_lib_init();

	mqtt_new(&mosq);

	mosquitto_message_callback_set(mosq, on_message);
	if(argc < 4)
		usage();

	mqtt_connect(&mosq, argv[1], atoi(argv[2]));

	mqtt_subscribe(&mosq, argc, argv);

	mosquitto_loop_forever(mosq, -1, 1);

	cleanup();

	return 0;
}