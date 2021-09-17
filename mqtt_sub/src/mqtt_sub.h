#define CONFIG "mqtt_sub"
struct topic {
	char *name;
	char *key;
	char *type;
	char *comparison;
	char *value;
};
enum comparison{
	EQUAL = 1, 
	NOT_EQUAL, 
	LESS,
	LESS_EQUAL, 
	MORE, 
	MORE_EQUAL
};
enum types{
	STRINGTYPE = 1,
	DECIMALTYPE,
};
struct arguments
{
	char *args[2]; 
	int verbose;       
	char *outfile;     
	char *port, *remoteAddress;
};
void cleanup(int sig);
