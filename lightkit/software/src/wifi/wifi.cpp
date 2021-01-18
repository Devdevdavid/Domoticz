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
	.userMode   = MODE_CLIENT,
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

/**
 * @brief Check if pWifiHandle contains valid data
 *
 * @param pWifiHandle Data to check
 * @param reason Text message containing the reason of not valid
 * @warning This text message appears in the webserver, it has to be
 * written using the appropriate language
 *
 * @return boolean
 */
static bool wifi_is_handle_valid(wifi_handle_t * pWifiHandle, String &reason)
{
	if (pWifiHandle->userMode >= MODE_MAX) {
		reason = "Mauvais mode wifi";
		return false;
	}

	if (strlen(pWifiHandle->ap.ssid) < 8) {
		reason = "[AP] SSID trop court (8 car.)";
		return false;
	}
	if (strlen(pWifiHandle->ap.password) < 8) {
		reason = "[AP] Mot de passe trop court (8 car.)";
		return false;
	}
	if ((pWifiHandle->ap.channel <= 0) || (pWifiHandle->ap.channel > 16)) {
		reason = "[AP] Canal invalide: " + String(pWifiHandle->ap.channel) + " [1-16]";
		return false;
	}
	if ((pWifiHandle->ap.maxConnection <= 0) || (pWifiHandle->ap.maxConnection > 3)) {
		reason = "[AP] Connexion max. invalide [1-16]";
		return false;
	}
	if (strlen(pWifiHandle->client.ssid) < 8) {
		reason = "[Client] SSID trop court (8 car.)";
		return false;
	}
	if (strlen(pWifiHandle->client.password) < 8) {
		reason = "[Client] Mot de passe trop court (8 car.)";
		return false;
	}
	if ((pWifiHandle->client.delayBeforeAPFallbackMs <= 0) || (pWifiHandle->client.delayBeforeAPFallbackMs > 60000)) {
		reason = "[Client] Délais de passage en AP (60s max)";
		return false;
	}

	return true;
}

/**
 * @brief Provide a pointer to handle for other modules
 */
wifi_handle_t * wifi_get_handle(void)
{
	return wifiHandle;
}

/**
 * @brief Use new settings for wifi module
 * @details Store them in flash after validation check
 */
int32_t wifi_use_new_settings(wifi_handle_t * pWifiHandle, String &reason)
{
	if (wifi_is_handle_valid(pWifiHandle, reason)) {
		// Copy new data to handle
		memcpy(wifiHandle, pWifiHandle, sizeof(wifi_handle_t));
		wifiHandle->forcedMode = MODE_NONE;
		wifiHandle->mode = MODE_NONE;

		// Save handle to flash
		return flash_write();
	} else {
		return -1;
	}
}

/**
 * @brief Use default settings for wifi module
 */
int32_t wifi_use_default_settings(void)
{
	String reason;
	return wifi_use_new_settings(&defaultWifiSettings, reason);
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

		// Somehow, we managed to save a wrong configuration
		// Use default for next reset
		wifi_use_default_settings();
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