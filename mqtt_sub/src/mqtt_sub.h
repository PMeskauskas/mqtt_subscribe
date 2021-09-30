#define CONFIG "mqtt_sub"
void cleanup(int sig);
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
struct topic {
	char *name;
	char *key;
	char *type;
	char *comparison;
	char *value;
	char *recEmail;
};

struct sender {
	char *email;
	char *username;
	char *password;
	char *smtpPort;
	char *smtpIP;
};