#include <uci.h>
#include <mosquitto.h>
#include <ctype.h>
#include <string.h>
#include <json-c/json.h>
#include <syslog.h>
#include "mqtt_func.h"
#include "mqtt_sub.h"

void uci_element_parseCheckOption(struct uci_option *option, char* type, char** value)
{
	if(strcmp(option->e.name, type) == 0)
		*value=option->v.string;
}

bool checkParameter(char* topicParameter, char *message)
{
	bool checkTopic = true;
	if(topicParameter == NULL){
		syslog(LOG_ERR, "%s", message);
		checkTopic = false;
	}
	return checkTopic;
}

int uci_element_parseSender(struct uci_package *package, struct sender *s)
{
	struct uci_element *i, *j;
	uci_foreach_element(&package->sections, i){
		struct uci_section *section = uci_to_section(i);
		char *section_type = section->type;
		if(strcmp(section_type,"mqtt_sub") == 0){
			uci_foreach_element(&section->options, j){
				struct uci_option *option = uci_to_option(j);
				uci_element_parseCheckOption(option,"senderEmail", &s->email);
				uci_element_parseCheckOption(option,"userName", &s->username);
				uci_element_parseCheckOption(option,"password", &s->password);
				uci_element_parseCheckOption(option,"smtpIP", &s->smtpIP);
				uci_element_parseCheckOption(option,"smtpPort", &s->smtpPort);

			}
			if((checkParameter(s->email,"Sender email not specified..."))    && 
			(checkParameter(s->username,"Sender username not specified...")) && 
			(checkParameter(s->password,"Sender password not specified...")) && 
			(checkParameter(s->smtpIP,"Sender smtpIP not specified...")))
				return 0;
		}
	}
    return -1;
}

void uci_element_parseSubscriber(struct uci_package *package, struct mosquitto **mosq)
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
				uci_element_parseCheckOption(option,"topic", &topic);
				uci_element_parseCheckOption(option,"qos", &qos);
			}
			if((topic != NULL) && (qos != NULL))
				mqtt_subscribe(mosq, topic, atoi(qos));
		}
	}
}
int uci_element_parseTopic(struct uci_package *package,  const struct mosquitto_message *msg, struct topic *t)
{
	struct uci_element *i, *j;
	uci_foreach_element(&package->sections, i){
		struct uci_section *section = uci_to_section(i);
		char *section_type = section->type;
		if(strcmp(section_type,"topic") == 0){
			uci_foreach_element(&section->options, j){
				struct uci_option *option = uci_to_option(j);
				uci_element_parseCheckOption(option,"topic", &t->name);
				uci_element_parseCheckOption(option,"key", &t->key);
				uci_element_parseCheckOption(option,"type", &t->type);
				uci_element_parseCheckOption(option,"comparison", &t->comparison);
				uci_element_parseCheckOption(option,"value", &t->value);
                uci_element_parseCheckOption(option,"recEmail", &t->recEmail);
			}
			if(strcmp(msg->topic, t->name) == 0)
				if((checkParameter(t->key,"Topic key not specified..."))&& 
				(checkParameter(t->type,"Topic type not specified..."))	&&
				(checkParameter(t->comparison,"Topic comparison not specified...")) &&
				(checkParameter(t->value,"Topic value not specified..."))&&
				(checkParameter(t->recEmail,"Topic recEmail not specified...")) )
					return 0;
		}
	}
    return -1;
}
