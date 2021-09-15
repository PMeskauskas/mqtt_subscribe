m = Map("mqtt_sub")

s = m:section(NamedSection, "mqtt_sub_sct", "mqtt_sub", "Subscriber settings")

flag = s:option(Flag, "enable", "Enable", "Enable program")

remoteaddress = s:option(Value, "remoteaddress", "Remote adress:")
remoteaddress.datatype = "string"
remoteaddress.maxlength = 65536

port = s:option(Value, "port", "port:")

st = m:section(TypedSection, "topic","Subscribed topics")
st.addremove = true
st.anonymous = true
st.novaluetext = "There are no topics created yet."

topic = st:option(Value, "topic", "Topic name", "")
topic.datatype = "string"
topic.maxlength = 65536
topic.placeholder = "Topic"
topic.rmempty = false
topic.parse = function(self, section, novld, ...)
	local value = self:formvalue(section)
	if value == nil or value == "" then
		self.map:error_msg("Topic name can not be empty")
		self.map.save = false
	end
	Value.parse(self, section, novld, ...)
end
qos = st:option(ListValue, "qos", "QoS level")
qos:value("0", "At most once (0)")
qos:value("1", "At least once (1)")
qos:value("2", "Exactly once (2)")
qos.rmempty=false
qos.default="0"

key = st:option(Value, "key", "Key")
key.datatype = "string"
key.maxlength = 65536

type = st:option(ListValue, "type", "Type")
type:value("", "None")
type:value("string", "String")
type:value("decimal", "Decimal")
type.default=""

comparison = st:option(ListValue, "comparison", "Comparison")
comparison:value("", "None")
comparison:value("1", "equal(==)")
comparison:value("2", "not equal(!=)")
comparison:value("3", "less(<)")
comparison:value("4", "less equal(<=)")
comparison:value("5",  "more(>)")
comparison:value("6", "more equal(>=)")
comparison.default=""

value = st:option(Value, "value", "Value")
value.datatype = "string"
value.maxlength = 65536
return m