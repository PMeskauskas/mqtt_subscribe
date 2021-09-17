#include <stdio.h>
#include <mosquitto.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <uci.h>
#include <argp.h>
#include "mqtt_func.h"
#include "uci_func.h"
#include "mqtt_sub.h"

volatile int interrupt = 0;
struct uci_context *ctx = NULL;
struct uci_package *package;
const char *argp_program_version ="mqtt_sub 1.0.0";
const char *argp_program_bug_address = "<TavoDraugas154@one.lt>";
static char args_doc[] = "ARG1 ARG2";
static char doc[] = "mqtt_sub -- A program to subscribe to topics via the mosquitto broker";

static struct argp_option options[] ={
	{ "remoteAddress", 'r', "ADDRESS", 0, "Specify the address" },
	{ "port",          'p', "PORT",    0, "Specify the port" }
};

void sigHandler(int signo) 
{
    signal(SIGINT, NULL);
    syslog(LOG_INFO, "Received signal: %d", signo);
    interrupt = 1;
}

void cleanup(int sig)
{
    closelog();
    exit(sig);
}

void usage(){
	printf("Usage: -p port -r remoteAddress");
	syslog(LOG_ERR, "Usage: -p port -r remoteAddress");
	cleanup(1);
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	uci_element_checkMessage(msg, package, ctx);
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
		
		default:
			return ARGP_ERR_UNKNOWN;
    }
	return 0;
}

void mqtt_init(struct mosquitto *mosq, struct arguments args, struct uci_package *package)
{
	mosquitto_lib_init();

	mqtt_new(&mosq);

	mosquitto_message_callback_set(mosq, on_message);

	mqtt_connect(&mosq, args.remoteAddress, args.port);

	uci_element_subscribe(package,&mosq);
	
	mosquitto_loop_forever(mosq, -1, 1);

	mosquitto_lib_cleanup();
}
static struct argp argp = {options, parse_opt, args_doc, doc};
int main(int argc, char *argv[]) 
{
    struct mosquitto *mosq;
	struct arguments arguments;
	int rc;
	arguments.remoteAddress = "";
  	arguments.port = "";
	if(argc != 5)
		usage();
	argp_parse (&argp, argc, argv, 0, 0, &arguments); //Segmentation fault ties --help ir --usage
	openlog(NULL, LOG_CONS, LOG_USER);

    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);
	uci_init(ctx, CONFIG, &package);
	mqtt_init(mosq, arguments, package);
	uci_free_context(ctx);
	cleanup(0);

}