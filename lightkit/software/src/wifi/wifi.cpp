/**
 * @file wifi.cpp
 * @description Manage all wifi
 * settings (AP and client)
 */

#include "wifi.hpp"
#include "flash/flash.hpp"
#include "script/script.hpp"

// Externals
extern uint32_t         tick;
extern flash_settings_t flashSettings;

// Internals
wifi_handle_t * wifiHandle     = NULL;
uint32_t        wifiTick       = 0;
uint32_t        APFallbackTick = 0; // Tick set at init before falling in AP mode in case of unsuccessfull client mode

/**
 * We need some compiler tricks here
 * See: https://pcbartists.com/firmware/esp32-firmware/designator-outside-aggregate-initializer-solved/
 */
// clang-format off
wifi_handle_t defaultWifiSettings = {
	.mode       = MODE_NONE,
	.userMode   = MODE_AP,
	.forcedMode = MODE_NONE,

	.ap = {
		{ .ssid = WIFI_DEFAULT_SSID },
		{ .password = WIFI_DEFAULT_PASSWORD },
		.channel = WIFI_DEFAULT_CHANNEL,
		.maxConnection = WIFI_DEFAULT_MAXCO,
		.isHidden = false,
		{ .ip = IPAddress(WIFI_DEFAULT_IP) },
		{ .gateway = IPAddress(WIFI_DEFAULT_GATEWAY) },
		{ .subnet = IPAddress(WIFI_DEFAULT_SUBNET) }
	},
	.client = {
		{ .ssid = P_WIFI_CLIENT_SSID },
		{ .password = P_WIFI_CLIENT_PWD },
		.delayBeforeAPFallbackMs = WIFI_DEFAULT_DELAY_AP_FALLBACK
	}
};
// clang-format on

// =====================
// STATIC FUNCTIONS
// =====================

static void wifi_print(void)
{
	if (wifiHandle->mode == MODE_AP) {
		log_info("Wifi mode  : Access Point");
		log_info("SSID       : %s", wifiHandle->ap.ssid);
		log_info("Mac address: %s", WiFi.softAPmacAddress().c_str());
		log_info("IP         : %s", WiFi.softAPIP().toString().c_str());
	} else if (wifiHandle->mode == MODE_CLIENT) {
		log_info("Wifi mode  : Client");
		log_info("SSID       : %s", wifiHandle->client.ssid);
		log_info("Mac address: %s", WiFi.macAddress().c_str());
		log_info("IP         : %s", WiFi.localIP().toString().c_str());
	}
	if (_isset(STATUS_WIFI, STATUS_WIFI_USING_FORCED_MODE)) {
		log_info("Wifi is using a FORCED MODE");
	}
}

static void wifi_fallback_as_ap(void)
{
	log_warn("Falling back to AP mode !");
	wifiHandle->forcedMode = MODE_AP;
	flash_write();

	// Resetting with new configuration
	script_delayed_reset(1000);
}

static int wifi_ap_init(void)
{
	WiFi.mode(WIFI_AP);
	WiFi.softAP(wifiHandle->ap.ssid, wifiHandle->ap.password, wifiHandle->ap.channel, wifiHandle->ap.isHidden, wifiHandle->ap.maxConnection);
	/** This line does magic, keep it here */
	WiFi.persistent(false);
	WiFi.softAPConfig(wifiHandle->ap.ip, wifiHandle->ap.gateway, wifiHandle->ap.subnet);
	return 0;
}

static int wifi_client_init(void)
{
	WiFi.mode(WIFI_STA);
	WiFi.setAutoReconnect(true);
	log_info("Trying to connect to %s...", wifiHandle->client.ssid);
	WiFi.begin(wifiHandle->client.ssid, wifiHandle->client.password);

	APFallbackTick = tick + wifiHandle->client.delayBeforeAPFallbackMs;
	return 0;
}

int wifi_init(void)
{
	int ret = -1;

	// Get Handle from flash
	wifiHandle = &flashSettings.wifiHandle;

	// Choose wich mode setting we use
	if (wifiHandle->forcedMode != MODE_NONE) {
		// Use forced mode
		wifiHandle->mode = wifiHandle->forcedMode;

		// ACK this 1 time setting
		wifiHandle->forcedMode = MODE_NONE;
		flash_write();

		// Notify we are in forced mode
		_set(STATUS_WIFI, STATUS_WIFI_USING_FORCED_MODE);

	} else {
		wifiHandle->mode = wifiHandle->userMode;
	}

	// Init wifi according to mode
	if (wifiHandle->mode == MODE_AP) {
		ret = wifi_ap_init();

		// In AP, we are ready to print now
		wifi_print();
	} else if (wifiHandle->mode == MODE_CLIENT) {
		ret = wifi_client_init();
		wifi_print();
	} else {
		log_error("Wifi mode not supported: %d", wifiHandle->mode);
	}

	// Check error
	if (ret) {
		log_error("Wifi init failed !");
		return -1;
	}

	return 0;
}

void wifi_main(void)
{
	// Periodically check wifi status
	if (tick >= wifiTick) {
		wifiTick = tick + WIFI_CHECK_PERIOD;

		if (wifiHandle->mode == MODE_AP) {
			if (WiFi.softAPgetStationNum() <= 0) {
				_unset(STATUS_WIFI, STATUS_WIFI_DEVICE_CO);
			} else {
				_set(STATUS_WIFI, STATUS_WIFI_DEVICE_CO);
			}
		} else if (wifiHandle->mode == MODE_CLIENT) {
			if (WiFi.status() != WL_CONNECTED) {
				_unset(STATUS_WIFI, STATUS_WIFI_IS_CO);

				// If couldn't connect as client after some time, reboot in AP mode
				if (tick >= APFallbackTick) {
					APFallbackTick = UINT32_MAX;
					wifi_fallback_as_ap();
				}
			} else {
				if (_isunset(STATUS_WIFI, STATUS_WIFI_IS_CO)) {
					_set(STATUS_WIFI, STATUS_WIFI_IS_CO);
					wifi_print();
				}
			}
		}
	}
}