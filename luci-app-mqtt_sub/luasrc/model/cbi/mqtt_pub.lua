map = Map("mqtt_pub")

section = map:section(NamedSection, "mqtt_pub_sct", "mqtt_pub", "Publisher settings")

flag = section:option(Flag, "enable", "Enable", "Enable program:")

remoteaddress = section:option(Value, "remoteaddress", "Remote adress:")
remoteaddress.datatype = "string"
remoteaddress.maxlength = 65536
port = section:option(Value, "port", "port:")

topic = section:option(Value, "topic", "topic:")

topic.datatype = "string"
topic.maxlength = 65536

message = section:option(Value, "message", "Message(payload):")

message.datatype = "string"
message.maxlength = 65536

qos = section:option(ListValue, "qos", "QoS level")
qos:value("0", "At most once (0)")                                                                               
qos:value("1", "At least once (1)")                                                                              
qos:value("2", "Exactly once (2)")                                                                               
qos.rmempty=false 
qos.default="0"
return map