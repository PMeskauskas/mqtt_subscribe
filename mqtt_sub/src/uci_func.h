void uci_init(struct uci_context *ctx, char* configName, struct uci_package **package);
void uci_load_package(struct uci_context *ctx, const char *config_name, struct uci_package **package);
void uci_element_subscribe(struct uci_package *package, struct mosquitto **mosq);
void uci_element_checkOption(struct uci_option *option, char* type, char** value);
int uci_element_checkMessage(const struct mosquitto_message *msg, struct uci_package *package, struct uci_context *ctx);
void uci_element_parseMessage(struct uci_package *package, struct json_object *parsed_json, const struct mosquitto_message *msg);
int  uci_element_outputMessage(struct topic t, struct json_object *parsed_json, const struct mosquitto_message *msg);
void uci_element_outputMessageInt(int messageValue, int topicValue, struct topic t);
void uci_element_outputMessageString(char *messageValue, struct topic t);
