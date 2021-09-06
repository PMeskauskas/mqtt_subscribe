include $(TOPDIR)/rules.mk

PKG_NAME:=mqtt_pub
PKG_VERSION:=1

include $(INCLUDE_DIR)/package.mk

define Package/mqtt_pub
	SECTION:=net
	CATEGORY:=Base system
	TITLE:=Modbus daemon by Teltonika
	DEPENDS:=+libmosquitto
endef

define Package/mqtt_pub/description
	MQTT publisher daemon by Teltonika
endef

define Package/mqtt_pub/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_CONF) ./files/mqtt_pub.config $(1)/etc/config/mqtt_pub
	$(INSTALL_BIN) ./files/mqtt_pub.init $(1)/etc/init.d/mqtt_pub
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mqtt_pub $(1)/usr/sbin/mqtt_pub
endef

$(eval $(call BuildPackage,mqtt_pub))
