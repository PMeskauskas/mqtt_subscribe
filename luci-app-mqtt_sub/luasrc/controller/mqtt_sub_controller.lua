module("luci.controller.mqtt_sub_controller", package.seeall)

function index()
    entry( { "admin", "services", "mqtt", "subscriber" }, cbi("mqtt_sub"), _("Subscriber"), 100).leaf = true
    entry( { "admin", "services", "mqtt", "publisher" }, cbi("mqtt_pub"), _("Publisher"), 50).leaf = true
end