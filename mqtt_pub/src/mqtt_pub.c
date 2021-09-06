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
    syslog(LOG_ERR, "Usage: remoteAddress port topicName \"payload\" QoS");
    cleanup();
}
void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
	syslog(LOG_INFO,"on_connect: %s\n", mosquitto_connack_string(reason_code));
	if(reason_code != 0){
		mosquitto_disconnect(mosq);
        cleanup();
	}
}

void on_publish(struct mosquitto *mosq, void *obj)
{
	syslog(LOG_INFO,"Message has been published.\n");
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

void mqtt_loop_start(struct mosquitto **mosq)
{
    int rc = MOSQ_ERR_SUCCESS;
    rc = mosquitto_loop_start(*mosq);
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(*mosq);
		syslog(LOG_ERR, "Error starting loop: %s\n", mosquitto_strerror(rc));
		cleanup();
	}
}

void publish_data(struct mosquitto *mosq, char *topic, char *payload, int QoS)
{
	int rc;
	rc = mosquitto_publish(mosq, NULL, topic, strlen(payload), payload, QoS, false);
	if(rc != MOSQ_ERR_SUCCESS){
		syslog(LOG_ERR, "Error publishing: %s\n", mosquitto_strerror(rc));
	}
}

int main(int argc, char *argv[]) {
    struct mosquitto *mosq;
	int rc;
    openlog(NULL, LOG_CONS, LOG_USER);
    if(argc != 6)
        usage();
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);
    
    mosquitto_lib_init();

    mqtt_new(&mosq);

    mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_publish_callback_set(mosq, on_publish);

	mqtt_connect(&mosq,argv[1],atoi(argv[2]));
	
	mqtt_loop_start(&mosq);
	
	publish_data(mosq, argv[3], argv[4],atoi(argv[5]));	
	
	cleanup();
	
	return 0;
}