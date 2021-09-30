void uci_element_parseCheckOption(struct uci_option *option, char* type, char** value);
void uci_element_parseSubscriber(struct uci_package *package, struct mosquitto **mosq);
int uci_element_parseSender(struct uci_package *package, struct sender *s);
int uci_element_parseTopic(struct uci_package *package,  const struct mosquitto_message *msg, struct topic *t);