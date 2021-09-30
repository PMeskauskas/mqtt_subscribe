#include <mosquitto.h>
#include <syslog.h>
#include <stdlib.h>
#include <ctype.h>
#include <json-c/json.h>
#include "mqtt_sub.h"

void mqtt_new(struct mosquitto **mosq)
{
	*mosq = mosquitto_new(NULL, true, NULL);
	if(*mosq == NULL){
		syslog(LOG_ERR, "Error: Out of memory.\n");
		cleanup(1);
	}
	syslog(LOG_INFO, "Created new mosquitto successfully");
}

void mqtt_connect(struct mosquitto **mosq, char *address, char* port)
{
	int rc = MOSQ_ERR_SUCCESS;
	long portNum;
	char *portPtr;

	if(address == NULL && port == NULL){
		syslog(LOG_ERR, "Must specify port and address...");
		mosquitto_lib_cleanup();
		cleanup(1);
	}

	portNum = strtol(port, &portPtr, 10);
	if((portPtr == port)){
		syslog(LOG_ERR, "Port must be an integer...");
		mosquitto_lib_cleanup();
		cleanup(1);
	}
	
	rc = mosquitto_connect(*mosq, address, portNum, 60);
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(*mosq);
		syslog(LOG_ERR, "Error connecting: %s\n", mosquitto_strerror(rc));
		mosquitto_lib_cleanup();
		cleanup(1);
	}
	syslog(LOG_ERR, "Connected to a broker successfully.");
}
void mqtt_subscribe(struct mosquitto **mosq, char* topicName, int qos)
{
	int rc = MOSQ_ERR_SUCCESS;
	rc = mosquitto_subscribe(*mosq, NULL, topicName, qos);
	if(rc != MOSQ_ERR_SUCCESS) {
		syslog(LOG_ERR, "Error subscribing: %s\n", mosquitto_strerror(rc));
		mosquitto_disconnect(*mosq);
		cleanup(1);
	}
	syslog(LOG_INFO, "subscribed to a topic: %s, QoS: %d", topicName, qos);
}