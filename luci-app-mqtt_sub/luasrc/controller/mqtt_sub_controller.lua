module("luci.controller.mqtt_sub_controller", package.seeall)

function index()
    entry( { "admin", "services", "mqtt", "subscriber" }, cbi("mqtt_sub"), _("Subscriber"), 100).leaf = true
end