#include <stdio.h>
#include <mosquitto.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <uci.h>
#include "cJSON.h"
volatile int interrupt = 0;
struct topic
{
	char *name;
	char *key;
	char *type;
	char *comparison;
	char *value;
};
enum day 
{
	EQUAL = 1, 
	NOT_EQUAL, 
	LESS,
	LESS_EQUAL, 
	MORE, 
	MORE_EQUAL
};
void sigHandler(int signo) 
{
    signal(SIGINT, NULL);
    syslog(LOG_INFO, "Received signal: %d", signo);
    interrupt = 1;
}

void cleanup()
{
    closelog();
	mosquitto_lib_cleanup();
    exit(1);
}

void usage(void) 
{
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
        char *topic = NULL;
        char *qos = NULL;

		if(strcmp(section_type,"topic") == 0){
            uci_foreach_element(&section->options, j)
            {
                struct uci_option *option = uci_to_option(j);
                checkOption(option,"topic", &topic);
                checkOption(option,"qos", &qos);
            }
			if(topic != NULL && qos != NULL)
				mqtt_subscribe(mosq, topic, atoi(qos));
		}
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

void checkOption(struct uci_option *option, char* type, char** value)
{
	if(strcmp(option->e.name, type) == 0)
		*value=option->v.string;
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	uci_element_checkMessage(msg);
}

void uci_element_checkMessage(const struct mosquitto_message *msg)
{
	cJSON *payload_json = cJSON_Parse((char*)msg->payload);
	if(payload_json != NULL){
		const char *config_name = "mqtt_sub";
		struct uci_context *ctx = uci_alloc_context();
		struct uci_package *package;
		uci_load_package(ctx, config_name, &package);
		uci_element_parseMessage(package, payload_json, msg);
		uci_free_context(ctx);
		cJSON_Delete(payload_json);
	}
	else
	{
		syslog(LOG_ERR, "Failed to parse message...");
		syslog(LOG_INFO, "Message received: topic: %s message: %s", msg->topic, (char*)msg->payload);
	}
}
void uci_element_parseMessage(struct uci_package *package, cJSON *payload_json, const struct mosquitto_message *msg)
{
	struct uci_element *i, *j;
	uci_foreach_element(&package->sections, i)
	{
		struct topic t = {NULL, NULL, NULL, NULL, NULL};
		struct uci_section *section = uci_to_section(i);
        char *section_type = section->type;
		if(strcmp(section_type,"topic") == 0){
            uci_foreach_element(&section->options, j)
            {
                struct uci_option *option = uci_to_option(j);
				checkOption(option,"topic", &t.name);
				checkOption(option,"key", &t.key);
				checkOption(option,"type", &t.type);
				checkOption(option,"comparison", &t.comparison);
				checkOption(option,"value", &t.value);
			}
			if(strcmp(msg->topic, t.name) == 0){
				uci_element_outputMessage(t, payload_json, msg);
				break;
			}
		}
	}
}
void uci_element_outputMessage(struct topic t, cJSON *payload_json, const struct mosquitto_message *msg)
{
	if(t.key != NULL){
		const cJSON *msgValue = NULL;
		if(msgValue = cJSON_GetObjectItemCaseSensitive(payload_json, t.key)){
			if(t.type != NULL && t.comparison != NULL && t.value != NULL){
				int topicValue;
				int messageValue;
				if(strcmp(t.type, "string") == 0){
					uci_element_outputMessageString(atoi(t.comparison),msgValue->valuestring, t);
				}
				else{
					if((topicValue = atoi(t.value)) && (messageValue = atoi(msgValue->valuestring))){
						uci_element_outputMessageInt(atoi(t.comparison), messageValue, topicValue, t);
					}
					else {
						syslog(LOG_ERR, "Type chosen as integer, but value isn't an integer");
						syslog(LOG_INFO, "Message received: topic: %s message: %s", msg->topic, (char*)msg->payload);
					}
				}
			}
			else{
				syslog(LOG_WARNING, "Only key specified");
				syslog(LOG_INFO, "Message received: topic: %s message: %s", msg->topic, (char*)msg->payload);
			}
		}
		else{
				syslog(LOG_ERR, "Message value doesn't have the specified key...");
				syslog(LOG_INFO, "Message received: topic: %s message: %s", msg->topic, (char*)msg->payload);
		}
	}
	else{
		syslog(LOG_INFO, "No key, outputting full message...");
		syslog(LOG_INFO, "Message received: topic: %s message: %s", msg->topic, (char*)msg->payload);
	}
		
	
}
void uci_element_outputMessageInt(int comparison, int messageValue, int topicValue, struct topic t)
{
	switch(comparison){
		case EQUAL: // ==
		if(messageValue == topicValue)
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %d", t.name, t.key, messageValue);
		break;
		
		case NOT_EQUAL: // !=
		if(messageValue != topicValue)
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %d", t.name, t.key, messageValue);
		break;
		
		case LESS: // <
		if(messageValue < topicValue)
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %d", t.name, t.key, messageValue);
		break;
		
		case LESS_EQUAL: // <=
		if(messageValue <= topicValue)
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %d", t.name, t.key, messageValue);
		break;
		
		case MORE: // >
		if(messageValue > topicValue)
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %d", t.name, t.key, messageValue);
		break;
		
		case MORE_EQUAL: // >=
		if(messageValue >= topicValue)
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %d", t.name, t.key, messageValue);
		break;
					
		default:
		break;
	}
}
void uci_element_outputMessageString(int comparison, char *messageValue, struct topic t)
{
	switch(comparison){
		case EQUAL: // ==
		if(strcmp(messageValue, t.name) == 0)
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %d", t.name, t.key, messageValue);
		break;
		
		case NOT_EQUAL: // !=
		if(strcmp(messageValue, t.name) != 0)
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %d", t.name, t.key, messageValue);
		break;
		
		case LESS: // <
		if(strcmp(messageValue, t.name) < 0)
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %d", t.name, t.key, messageValue);
		break;
		
		case LESS_EQUAL: // <=
		if(strcmp(messageValue, t.name) <= 0)
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %d", t.name, t.key, messageValue);
		break;
		
		case MORE: // >
		if(strcmp(messageValue, t.name) > 0)
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %d", t.name, t.key, messageValue);
		break;
		
		case MORE_EQUAL: // >=
		if(strcmp(messageValue, t.name) >= 0)
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %d", t.name, t.key, messageValue);
		break;
					
		default:
		break;
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