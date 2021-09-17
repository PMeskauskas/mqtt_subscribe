#include <uci.h>
#include <syslog.h>
#include <mosquitto.h>
#include <json-c/json.h>
#include "mqtt_sub.h"
#include "mqtt_func.h"

void uci_init(struct uci_context *ctx, char* configName, struct uci_package **package)
{
	ctx = uci_alloc_context();
	if(ctx == NULL){
		syslog(LOG_ERR, "Failed to allocate memory to uci context");
		cleanup(1);
	}
	uci_load_package(ctx, CONFIG, package);
}

void uci_load_package(struct uci_context *ctx, const char *config_name, struct uci_package **package)
{
    if(uci_load(ctx, config_name, package) !=UCI_OK){
		syslog(LOG_ERR, "Failed to load uci");
		uci_free_context(ctx);
		cleanup(1);
	}
}

void uci_element_checkOption(struct uci_option *option, char* type, char** value)
{
	if(strcmp(option->e.name, type) == 0)
		*value=option->v.string;
}

void uci_element_subscribe(struct uci_package *package, struct mosquitto **mosq)
{
    struct uci_element *i, *j;
    uci_foreach_element(&package->sections, i){
		struct uci_section *section = uci_to_section(i);
        char *section_type = section->type;
        char *topic = NULL;
        char *qos = NULL;

		if(strcmp(section_type,"topic") == 0){
            uci_foreach_element(&section->options, j){
                struct uci_option *option = uci_to_option(j);
                uci_element_checkOption(option,"topic", &topic);
                uci_element_checkOption(option,"qos", &qos);
            }
			if(topic != NULL && qos != NULL)
				mqtt_subscribe(mosq, topic, atoi(qos));
		}
	}
}

int uci_element_checkMessage(const struct mosquitto_message *msg, struct uci_package *package, struct uci_context *ctx)
{
	struct json_object *parsed_json = json_tokener_parse((char*)msg->payload);
	if(parsed_json == NULL){
		syslog(LOG_ERR, "Failed to parse message...");
		syslog(LOG_INFO, "Message received: topic: %s message: %s", msg->topic, (char*)msg->payload);
		return;
	}
		uci_element_parseMessage(package, parsed_json, msg);
		return;
}

void uci_element_parseMessage(struct uci_package *package, struct json_object *parsed_json, const struct mosquitto_message *msg)
{
	struct uci_element *i, *j;
	uci_foreach_element(&package->sections, i){
		struct topic t = {NULL, NULL, NULL, NULL, NULL};
		struct uci_section *section = uci_to_section(i);
        char *section_type = section->type;
		if(strcmp(section_type,"topic") == 0){
            uci_foreach_element(&section->options, j){
                struct uci_option *option = uci_to_option(j);
				uci_element_checkOption(option,"topic", &t.name);
				uci_element_checkOption(option,"key", &t.key);
				uci_element_checkOption(option,"type", &t.type);
				uci_element_checkOption(option,"comparison", &t.comparison);
				uci_element_checkOption(option,"value", &t.value);
			}
			if(strcmp(msg->topic, t.name) == 0){
				uci_element_outputMessage(t, parsed_json, msg);
				break;
			}
		}
	}
}

int uci_element_outputMessage(struct topic t, struct json_object *parsed_json, const struct mosquitto_message *msg)
{
	struct json_object *objectValue;
	json_object_object_get_ex(parsed_json, t.key, &objectValue);
	char *messageValue = json_object_get_string(objectValue);
	int topicNum; 
	int messageNum; 

	if(t.key == NULL){
		syslog(LOG_ERR, "No key, outputting full message...");
		syslog(LOG_INFO, "Message received: topic: %s message: %s", msg->topic, (char*)msg->payload);
		return;
	}
	
	if(objectValue == NULL){
		syslog(LOG_ERR, "Message value doesn't have the specified key...");
		syslog(LOG_INFO, "Message received: topic: %s message: %s", msg->topic, (char*)msg->payload);
		return;
	}
	
	if(t.type == NULL && t.comparison == NULL && t.value == NULL){
		syslog(LOG_ERR, "Only key specified");
		syslog(LOG_INFO, "Message received: topic: %s value: %s", msg->topic, messageValue);
		return;
	}
	
	switch(atoi(t.type)){
		case STRINGTYPE:
			if(atoi(t.value)){
				syslog(LOG_ERR,"Type chosen as string, but value from input is a number.");
				syslog(LOG_INFO, "Message received: topic: %s message: %s", msg->topic, (char*)msg->payload);
				break;
			}

			if(atoi(messageValue)){
				syslog(LOG_ERR,"Type chosen as string, but value from message is a number.");
				syslog(LOG_INFO, "Message received: topic: %s message: %s", msg->topic, (char*)msg->payload);
				break;
			}

			uci_element_outputMessageString(messageValue, t);
		break;
		
		case DECIMALTYPE:
				if(!(topicNum = atoi(t.value))){
					syslog(LOG_ERR,"Type chosen as decimal, but value from input is a string.");
					syslog(LOG_INFO, "Message received: topic: %s message: %s", msg->topic, (char*)msg->payload);
					break;
				}
				
				if(!(messageNum=atoi(messageValue))){
					syslog(LOG_ERR,"Type chosen as decimal, but value from message is a string.");
					syslog(LOG_INFO, "Message received: topic: %s message: %s", msg->topic, (char*)msg->payload);
					break;
				}
				uci_element_outputMessageInt(messageNum, topicNum, t);
		break;
	}
	return;
}


void uci_element_outputMessageInt(int messageValue, int topicValue, struct topic t)
{
	switch(atoi(t.comparison)){
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

void uci_element_outputMessageString(char *messageValue, struct topic t)
{
	switch(atoi(t.comparison)){
		case EQUAL: // ==
		if(strcmp(messageValue, t.name) == 0)
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %s", t.name, t.key, messageValue);
		break;
		
		case NOT_EQUAL: // !=
		if(strcmp(messageValue, t.name) != 0)
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %s", t.name, t.key, messageValue);
		break;
		
		case LESS: // <
		if(strcmp(messageValue, t.name) < 0)
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %s", t.name, t.key, messageValue);
		break;
		
		case LESS_EQUAL: // <=
		if(strcmp(messageValue, t.name) <= 0)
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %s", t.name, t.key, messageValue);
		break;
		
		case MORE: // >
		if(strcmp(messageValue, t.name) > 0)
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %s", t.name, t.key, messageValue);
		break;
		
		case MORE_EQUAL: // >=
		if(strcmp(messageValue, t.name) >= 0)
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %s", t.name, t.key, messageValue);
		break;
					
		default:
		break;
	}
}
