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
wifi_handle_t * wifiHandle = NULL;

uint32_t wifiTick = 0;
// Tick set at init before falling in AP mode in case of unsuccessfull client mode
uint32_t APFallbackTick     = UINT32_MAX;
uint32_t isAPConfigToDoTick = UINT32_MAX; // Tick use to end AP config after a delay
uint32_t isScanToStartTick  = UINT32_MAX; // Tick use to start a scan
uint32_t lastScanTick       = 0;          // Tick use to save the instant of the last scan

wifi_fast_reconnect_t wifiFastReconnect = { 0 };

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
		.ip = WIFI_DEFAULT_IP,
		.gateway = WIFI_DEFAULT_GATEWAY,
		.subnet = WIFI_DEFAULT_SUBNET
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
		log_info("Channel    : %d", WiFi.channel());
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

	WiFi.softAP(wifiHandle->ap.ssid,
				wifiHandle->ap.password,
				wifiHandle->ap.channel,
				wifiHandle->ap.isHidden,
				wifiHandle->ap.maxConnection);

	/** This line does magic, keep it here */
	WiFi.persistent(false);

	// We have to wait at least 100ms after WiFi.softAP() before calling WiFi.softAPConfig()
	// so we moved this call into the main function with the flag isAPConfigToDo
	// More info : https://github.com/espressif/arduino-esp32/issues/985#issuecomment-359157428
	isAPConfigToDoTick = tick + WIFI_DELAYED_CONFIG_MS;
	return 0;
}

static void wifi_ap_init_later(void)
{
	WiFi.softAPConfig(IPAddress(wifiHandle->ap.ip), IPAddress(wifiHandle->ap.gateway), IPAddress(wifiHandle->ap.subnet));
}

static int wifi_client_init(void)
{
	WiFi.mode(WIFI_STA);
	WiFi.setAutoReconnect(true);
	log_info("Trying to connect to %s...", wifiHandle->client.ssid);

	if (wifiFastReconnect.isValid) {
		// Use reconnect settings (Faster)
		WiFi.begin(wifiHandle->client.ssid,
				   wifiHandle->client.password,
				   wifiFastReconnect.channel,
				   wifiFastReconnect.bssid,
				   true);
	} else {
		// Connect with channel scanning (Longer)
		WiFi.begin(wifiHandle->client.ssid, wifiHandle->client.password);
	}

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
static bool wifi_is_handle_valid(wifi_handle_t * pWifiHandle, String & reason)
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

static int32_t wifi_end_scan(void)
{
	int32_t ret = -1;

	// Check if not scanning
	if (wifiHandle->mode != MODE_SCAN) {
		return -1;
	}

	log_info("Wifi scan done (%d discovered)", WiFi.scanComplete());

	wifiHandle->mode = wifiFastReconnect.mode;

	// Re-init
	if (wifiHandle->mode == MODE_AP) {
		ret = wifi_ap_init();
	} else if (wifiHandle->mode == MODE_CLIENT) {
		ret = wifi_client_init();
	} else {
		// Don't panic and do something
		log_error("Unknown wifi mode after scan ended: %d", wifiHandle->mode);
		wifi_fallback_as_ap();
	}

	// Disable fastReconnect
	wifiFastReconnect.isValid = false;
	wifiFastReconnect.mode    = MODE_NONE;

	return ret;
}

static int32_t wifi_start_scan(void)
{
	// Check if already scanning
	if (wifiHandle->mode == MODE_SCAN) {
		return -1;
	}

	// Check if it is too soon to ask for a new scan
	if ((lastScanTick != 0) && ((tick - lastScanTick) < WIFI_SCAN_MIN_INTERVAL_MS)) {
		return -2;
	}

	log_info("Starting wifi scan");

	// Save current time
	lastScanTick = tick;

	// If in client mode, copy some settings to speed up reconnection
	if (wifiHandle->mode == MODE_CLIENT) {
		wifiFastReconnect.isValid = true;
		wifiFastReconnect.channel = WiFi.channel();
		memcpy(wifiFastReconnect.bssid, WiFi.BSSID(), sizeof(wifiFastReconnect.bssid));
	}

	// Save current mode for reconnection
	wifiFastReconnect.mode = wifiHandle->mode;
	wifiHandle->mode       = MODE_SCAN;

	// Disconnect wifi to start scan
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();

	// This is really bad but it makes life pretty easier though
	delay(100);

	// Start the scan with async=true
	WiFi.scanNetworks(true);

	return 0;
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
int32_t wifi_use_new_settings(wifi_handle_t * pWifiHandle, String & reason)
{
	int32_t ret;

	if (wifi_is_handle_valid(pWifiHandle, reason)) {
		// Copy new data to handle
		memcpy(wifiHandle, pWifiHandle, sizeof(wifi_handle_t));
		wifiHandle->forcedMode = MODE_NONE;
		wifiHandle->mode       = MODE_NONE;

		// Save handle to flash
		ret = flash_write();

		// Resetting with new configuration
		script_delayed_reset(1000);

		return ret;
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

/**
 * @brief Either trigger a new scan or tell caller we can read
 * scan with WiFi.___()
 * @return -1: Scan have been trigger, call latter for result
 */
int32_t wifi_start_scan_req(void)
{
	// Check if there is result we can use and if they are not too old
	if ((WiFi.scanComplete() >= 0) && ((tick - lastScanTick) < WIFI_SCAN_MIN_INTERVAL_MS)) {
		// Use previous results
		return WiFi.scanComplete();
	}

	// A scan is already pending, just wait it
	if (isScanToStartTick != UINT32_MAX) {
		return -1;
	}

	log_info("Wifi scan requested, starting in %ds", WIFI_DELAYED_SCAN_MS / 1000);

	// Trigger a new scan
	isScanToStartTick = tick + WIFI_DELAYED_SCAN_MS;
	return -1;
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
	} else if (wifiHandle->mode == MODE_CLIENT) {
		ret = wifi_client_init();
	} else {
		log_error("Wifi mode not supported: %d", wifiHandle->mode);

		// Somehow, we managed to save a wrong configuration
		// Use default for next reset
		wifi_use_default_settings();
	}

	// Print settings
	wifi_print();

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
			// We have to wait a bit before calling WiFi.softAPConfig()
			// so do it here 1 time after one WIFI_CHECK_PERIOD
			if (tick > isAPConfigToDoTick) {
				isAPConfigToDoTick = UINT32_MAX;
				wifi_ap_init_later();
			}

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

	if (tick > isScanToStartTick) {
		isScanToStartTick = UINT32_MAX;
		wifi_start_scan();
	}

	// Poll for scan results
	if (wifiHandle->mode == MODE_SCAN) {
		if (WiFi.scanComplete() >= 0) {
			wifi_end_scan();
		}
	}
}