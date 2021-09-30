#include <stdio.h>
#include <mosquitto.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <uci.h>
#include <argp.h>
#include <json-c/json.h>
#include "mqtt_func.h"
#include "uci_message.h"
#include "mqtt_sub.h"
#include "uci_parse.h"
#include "uci_init.h"

static struct arguments
{
	char *args[2];     
	char *port, *remoteAddress;
};

volatile int interrupt = 0;
struct uci_context *ctx = NULL;
struct uci_package *package = NULL;
struct sender sender = {NULL, NULL, NULL, NULL, NULL};
const char *argp_program_version ="mqtt_sub 1.0.0";
const char *argp_program_bug_address = "<TavoDraugas154@one.lt>";
static char args_doc[] = "-p port -r remoteAddress";
static char doc[] = "mqtt_sub -- A program to subscribe to topics via the mosquitto broker";

static struct argp_option options[] ={
	{ "remoteAddress", 'r', "ADDRESS", 0, "Specify the address" },
	{ "port",          'p', "PORT",    0, "Specify the port" },
	{0}
};

void sigHandler(int signo) 
{
	signal(SIGINT, NULL);
	syslog(LOG_INFO, "Received signal: %d", signo);
	interrupt = 1;
}

void cleanup(int sig)
{
	syslog(LOG_INFO, "Closing program...");
	closelog();
	exit(sig);
}

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = state->input;
	switch (key){
		case 'p':
			arguments->port = arg;
		break;

		case 'r':
			arguments->remoteAddress = arg;
		break;

		case ARGP_KEY_ARG:
			arguments->args[state->arg_num] = arg;
		break;
		case ARGP_KEY_END:
		if ((!arguments->remoteAddress) || (!arguments->port))
		{
			argp_usage (state);
		}
		break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}
void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	int rc;
	struct json_object *objectJson = NULL;
	struct json_object *parsedJson = NULL;
	struct topic topic = { NULL, NULL, NULL, NULL, NULL, NULL };
	char *messageValue = NULL;
	
	rc = uci_element_parseTopic(package, msg, &topic);
	if(rc != 0){
		syslog(LOG_ERR, "Failed to parse information about topic");
		syslog(LOG_INFO, "Message received: topic: %s message: %s", msg->topic, (char*)msg->payload);
		return;
	}
	syslog(LOG_INFO, "Topic parsed succesfully");

	parsedJson = json_tokener_parse((char*)msg->payload);
	if(parsedJson == NULL){
		syslog(LOG_ERR, "Failed to parse a JSON message");
		syslog(LOG_INFO, "Message received: topic: %s message: %s", msg->topic, (char*)msg->payload);
		return;
	}
	syslog(LOG_INFO, "JSON parsed succesfully");	
	
	json_object_object_get_ex(parsedJson, topic.key, &objectJson);
	if(objectJson == NULL){
		syslog(LOG_ERR, "Message value doesn't have the specified key");
		syslog(LOG_INFO, "Message received: topic: %s message: %s", msg->topic, (char*)msg->payload);
		return;
	}

	messageValue = json_object_get_string(objectJson);
	if(messageValue == NULL){
		syslog(LOG_ERR, "Failed to get string from JSON object");
		syslog(LOG_INFO, "Message received: topic: %s message: %s", msg->topic, (char*)msg->payload);
		return;
	}
	uci_element_messageOutputCheckType(&topic, &sender, messageValue);

}


void mqtt_init(struct mosquitto *mosq, struct arguments args, struct uci_package *package)
{
	int rc;

	rc = uci_element_parseSender(package, &sender);
	if(rc != 0){
		syslog(LOG_ERR, "Failed to parse sender");
		cleanup(1);
	}
	syslog(LOG_INFO, "Sender parsed succesfully");
	mosquitto_lib_init();

	mqtt_new(&mosq);

	mosquitto_message_callback_set(mosq, on_message);

	mqtt_connect(&mosq, args.remoteAddress, args.port);

	uci_element_parseSubscriber(package, &mosq);
	
	mosquitto_loop_forever(mosq, -1, 1);
	
}
static struct argp argp = { options, parse_opt, args_doc, doc };

int main(int argc, char *argv[]) 
{
	struct mosquitto *mosq;
	struct arguments arguments;

	arguments.remoteAddress = NULL;
	arguments.port = NULL;

	argp_parse (&argp, argc, argv, 0, 0, &arguments);
	openlog(NULL, LOG_CONS, LOG_USER);

	signal(SIGINT, sigHandler);
	signal(SIGTERM, sigHandler);

	uci_init(ctx, CONFIG, &package);
	mqtt_init(mosq, arguments, package);

	uci_free_context(ctx);
	mosquitto_lib_cleanup();
	cleanup(0);
	return 0;
}