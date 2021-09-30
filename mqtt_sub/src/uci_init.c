#include <uci.h>
#include <syslog.h>
#include "mqtt_sub.h"

void uci_load_package(struct uci_context *ctx, const char *config_name, struct uci_package **package)
{
	if(uci_load(ctx, config_name, package) !=UCI_OK){
		syslog(LOG_ERR, "Failed to load uci");
		uci_free_context(ctx);
		cleanup(1);
	}
}

void uci_init(struct uci_context *ctx, char* configName, struct uci_package **package)
{
	ctx = uci_alloc_context();
	if(ctx == NULL){
		syslog(LOG_ERR, "Failed to allocate memory to uci context");
		cleanup(1);
	}
	uci_load_package(ctx, CONFIG, package);
}

