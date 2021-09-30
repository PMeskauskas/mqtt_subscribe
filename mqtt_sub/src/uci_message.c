#include <uci.h>
#include <syslog.h>
#include <mosquitto.h>
#include <ctype.h>
#include <string.h>
#include <json-c/json.h>
#include "mqtt_sub.h"
#include "curl_messageSend.h"
void uci_element_messageOutputInt(int messageValue, int topicValue, struct topic *topic, struct sender *sender)
{
	char message[1023];
	switch(atoi(topic->comparison)){
		case EQUAL: // ==
		if(messageValue == topicValue){
			syslog(LOG_INFO, "Value is == %d", topicValue);
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %d",
				topic->name, topic->key, messageValue);

			sprintf(message, "Subject: Received message from: %s\r\nValue is == %d\r\nKey: %s, value: %d\n",
				topic->name, topicValue, topic->key, messageValue);
			curl_sendEmail(topic->recEmail, sender, message);
		}
		break;
		
		case NOT_EQUAL: // !=
		if(messageValue != topicValue){
			syslog(LOG_INFO, "Value is != %d", topicValue);
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %d",
				topic->name, topic->key, messageValue);
			
			sprintf(message, "Subject: Received message from: %s\r\nValue is != %d\r\nKey: %s, value: %d\n",
				topic->name, topicValue, topic->key, messageValue);
			curl_sendEmail(topic->recEmail, sender, message);
		}
		break;
		
		case LESS: // <
		if(messageValue < topicValue){
			syslog(LOG_INFO, "Value is < %d", topicValue);
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %d",
				topic->name, topic->key, messageValue);

			sprintf(message, "Subject: Received message from: %s\r\nValue is < %d\r\nKey: %s, value: %d\n",
				topic->name, topicValue, topic->key, messageValue);
			curl_sendEmail(topic->recEmail, sender, message);
		}
		break;
		
		case LESS_EQUAL: // <=
		if(messageValue <= topicValue){
			syslog(LOG_INFO, "Value is <= %d", topicValue);
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %d",
				topic->name, topic->key, messageValue);

			sprintf(message, "Subject: Received message from: %s\r\nValue is <= %d\r\nKey: %s, value: %d\n",
			topic->name, topicValue, topic->key, messageValue);
			curl_sendEmail(topic->recEmail, sender, message);
		}
		break;
		
		case MORE: // >
		if(messageValue > topicValue){
			syslog(LOG_INFO, "Value is > %d", topicValue);
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %d",
				topic->name, topic->key, messageValue);

			sprintf(message, "Subject: Received message from: %s\r\nValue is > %d\r\nKey: %s, value: %d\n",
			topic->name, topicValue, topic->key, messageValue);
			curl_sendEmail(topic->recEmail, sender, message);
		}
		break;
		
		case MORE_EQUAL: // >=
		if(messageValue >= topicValue){
			syslog(LOG_INFO, "Value is >= %d", topicValue);
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %d",
				topic->name, topic->key, messageValue);

			sprintf(message, "Subject: Received message from: %s\r\nValue is >= %d\r\nKey: %s, value: %d\n",
			topic->name, topicValue, topic->key, messageValue);
			curl_sendEmail(topic->recEmail, sender, message);
		}
		break;
					
		default:
		syslog(LOG_ERR, "Wrong comparison specified(1-6)");
		break;
	}
	memset(message, 0, sizeof message);
}

void uci_element_messageOutputString(char *messageValue, struct topic *topic, struct sender *sender)
{
	char message[1023];
	switch(atoi(topic->comparison)){
		case EQUAL: // ==
		if(strcmp(messageValue, topic->value) == 0){
			syslog(LOG_INFO, "Value is == %s", topic->value);
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %s",
			 topic->name, topic->key, messageValue);

			sprintf(message, "Subject: Received message from: %s\r\nValue is == %s\r\nKey: %s, value: %s\n",
				topic->name, topic->value, topic->key, messageValue);
			curl_sendEmail(topic->recEmail, sender, message);
		}
		break;
		
		case NOT_EQUAL: // !=
		if(strcmp(messageValue, topic->value) != 0){
			syslog(LOG_INFO, "Value is != %s", topic->value);
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %s",
				topic->name, topic->key, messageValue);

			sprintf(message, "Subject: Received message from: %s\r\nValue is != %s\r\nKey: %s, value: %s\n",
				topic->name, topic->value, topic->key, messageValue);
			curl_sendEmail(topic->recEmail, sender, message);
		}
		break;
		
		case LESS: // <
		if(strcmp(messageValue, topic->value) < 0){
			syslog(LOG_INFO, "Value is < %s", topic->value);
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %s",
				topic->name, topic->key, messageValue);
			
			sprintf(message, "Subject: Received message from: %s\r\nValue is < %s\r\nKey: %s, value: %s\n",
				topic->name, topic->value, topic->key, messageValue);
			curl_sendEmail(topic->recEmail, sender, message);
		}
		break;
		
		case LESS_EQUAL: // <=
		if(strcmp(messageValue, topic->value) <= 0){
			syslog(LOG_INFO, "Value is <= %s", topic->value);
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %s",
				topic->name, topic->key, messageValue);

			sprintf(message, "Subject: Received message from: %s\r\nValue is <= %s\r\nKey: %s, value: %s\n",
				topic->name, topic->value, topic->key, messageValue);
			curl_sendEmail(topic->recEmail, sender, message);
		}
		break;
		
		case MORE: // >
		if(strcmp(messageValue, topic->value) > 0){
			syslog(LOG_INFO, "Value is > %s", topic->value);
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %s",
				topic->name, topic->key, messageValue);

			sprintf(message, "Subject: Received message from: %s\r\nValue is > %s\r\nKey: %s, value: %s\n",
				topic->name, topic->value, topic->key, messageValue);
			curl_sendEmail(topic->recEmail, sender, message);
		}
		break;
		
		case MORE_EQUAL: // >=
		if(strcmp(messageValue, topic->value) >= 0){
			syslog(LOG_INFO, "Value is >= %s", topic->value);
			syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %s", topic->name, topic->key, messageValue);

			sprintf(message, "Subject: Received message from: %s\r\nValue is >= %s\r\nKey: %s, value: %s\n",
				topic->name, topic->value, topic->key, messageValue);
			curl_sendEmail(topic->recEmail, sender, message);
		}
		break;
					
		default:
		syslog(LOG_ERR, "Wrong comparison specified(1-6)");
		break;
	}
	memset(message, 0, sizeof message);
}

void uci_element_messageOutputCheckType(struct topic *topic, struct sender *sender, char *messageValue)
{
	
	char *topicPtr;
	char *messagePtr;
	long topicNum = strtol(topic->value, &topicPtr, 10);
	long messageNum = strtol(messageValue, &messagePtr, 10);
	switch(atoi(topic->type)){
		case STRINGTYPE:
			if(topicPtr != topic->value){
				syslog(LOG_ERR,"Type chosen as string, but value from input is a number...");
				syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %s",
					topic->name, topic->key, messageValue);
				break;
			}

			if(messagePtr != messageValue){
				syslog(LOG_ERR,"Type chosen as string, but value from message is a number...");
				syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %s",
					topic->name, topic->key, messageValue);
				break;
			}

			uci_element_messageOutputString(messageValue, topic, sender);
		break;
		
		case DECIMALTYPE:
				if(topicPtr == topic->value){
					syslog(LOG_ERR,"Type chosen as decimal, but value from input is a string...");
					syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %s",
						topic->name, topic->key, messageValue);
					break;
				}
				
				if(messagePtr == messageValue){
					syslog(LOG_ERR,"Type chosen as decimal, but value from message is a string...");
					syslog(LOG_INFO, "Message received: topic: %s key: %s, value: %s",
						topic->name, topic->key, messageValue);
					break;
				}
				uci_element_messageOutputInt(messageNum, topicNum, topic, sender);
		break;

		default:
		syslog(LOG_ERR, "Wrong type specified(1-2)");
		break;
	}
}
