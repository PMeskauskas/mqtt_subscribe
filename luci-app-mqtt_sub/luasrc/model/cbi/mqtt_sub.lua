m = Map("mqtt_sub")

s = m:section(NamedSection, "mqtt_sub_sct", "mqtt_sub", "Subscriber settings")

flag = s:option(Flag, "enable", "Enable", "Enable program")

remoteaddress = s:option(Value, "remoteaddress", "Remote adress:")
remoteaddress.datatype = "string"
remoteaddress.maxlength = 512
remoteaddress.rmempty = false

port = s:option(Value, "port", "port:")
port.rmempty = false

local is_group = false
mailGroup = s:option(ListValue, "emailgroup", "Email account", "Recipient's email configuration <br/>(<a href=\"/cgi-bin/luci/admin/system/admin/group/email\" class=\"link\">configure it here</a>)")
m.uci:foreach("user_groups", "email", function(s)
	if s.senderemail then
		mailGroup:value(s.name, s.name)
		is_group = true
	end
end)
if not is_group then
	mailGroup:value(0, "No email accounts created")
end

function mailGroup.parse(self, section, novld, ...)
	local val = self:formvalue(section)
	if val and val == "0" then
		self:add_error(section, "invalid", "No email accounts selected")
	end
	Value.parse(self, section, novld, ...)
end



st = m:section(TypedSection, "topic","Subscribed topics")
st.addremove = true
st.anonymous = true
st.novaluetext = "There are no topics created yet."

topic = st:option(Value, "topic", "Topic name", "")
topic.datatype = "string"
topic.maxlength = 512
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
key.maxlength = 512
key.rmempty = false

type = st:option(ListValue, "type", "Type")
type:value("", "None")
type:value("1", "String")
type:value("2", "Decimal")
type.default=""
type.rmempty = false

comparison = st:option(ListValue, "comparison", "Comparison")
comparison:value("", "None")
comparison:value("1", "equal(==)")
comparison:value("2", "not equal(!=)")
comparison:value("3", "less(<)")
comparison:value("4", "less equal(<=)")
comparison:value("5",  "more(>)")
comparison:value("6", "more equal(>=)")
comparison.default=""
comparison.rmempty = false

value = st:option(Value, "value", "Value")
value.datatype = "string"
value.maxlength = 512
value.rmempty = false

recEmail = st:option(Value, "recEmail", "Recipient's email address", "For whom you want to send an email to. Allowed characters (a-zA-Z0-9._%+@-)")
recEmail.datatype = "email"
recEmail.placeholder = "mail@domain.com"
recEmail.rmempty = false

function m.on_save(self)
	local group_name = m:formvalue("cbid.mqtt_sub." .."mqtt_sub_sct".. ".emailgroup")
	local group
	m.uci:foreach("user_groups", "email", function(s)
		if s.name == group_name then
			group = s[".name"]
		end
	end)
	local smtpIP = m.uci:get("user_groups", group, "smtp_ip") 
	local smtpPort = m.uci:get("user_groups", group, "smtp_port") 
	local username = m.uci:get("user_groups", group, "username") 
	local passwd = m.uci:get("user_groups", group, "password") 
	local senderEmail = m.uci:get("user_groups", group, "senderemail") 
	m.uci:set("mqtt_sub","mqtt_sub_sct","senderEmail", senderEmail)
	m.uci:set("mqtt_sub","mqtt_sub_sct","userName", username)
	m.uci:set("mqtt_sub","mqtt_sub_sct","password", passwd)
	m.uci:set("mqtt_sub","mqtt_sub_sct","smtpIP", smtpIP)
	m.uci:set("mqtt_sub","mqtt_sub_sct","smtpPort", smtpPort)
end

return m