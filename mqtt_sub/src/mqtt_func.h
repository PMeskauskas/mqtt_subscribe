void mqtt_new(struct mosquitto **mosq);
void mqtt_connect(struct mosquitto **mosq, char *address, char* port);
void mqtt_subscribe(struct mosquitto **mosq, char* topicName, int qos);