/**
 * @file wifi.cpp
 * @description Manage all wifi
 * settings (AP and client)
 */

#define WIFI_CPP

#include "wifi.hpp"
#include "buzzer/buzzer.hpp"
#include "flash/flash.hpp"
#include "ota/ota.hpp"
#include "script/script.hpp"

// Externals
extern uint32_t tick;

// Internals
// Pointer is constant but value isn't
wifi_handle_t * const wifiHandle = &flashSettings.wifiHandle;

uint32_t wifiTick = 0;
// Tick set at init before falling in AP mode in case of unsuccessfull client mode
uint32_t APFallbackTick       = UINT32_MAX;
uint32_t isAPConfigToDoTick   = UINT32_MAX; // Tick use to end AP config after a delay
uint32_t isScanToStartTick    = UINT32_MAX; // Tick use to start a scan
uint32_t lastScanTick         = 0;          // Tick use to save the instant of the last scan
bool     clientConfigSucceded = false;      // Tell if current client config worked at least one time

wifi_fast_reconnect_t wifiFastReconnect = { 0 };

/**
 * We need some compiler tricks here
 * See: https://pcbartists.com/firmware/esp32-firmware/designator-outside-aggregate-initializer-solved/
 */
// clang-format off
wifi_handle_t defaultWifiSettings = {
	.mode       = MODE_NONE,
	.userMode   = WIFI_DEFAULT_MODE,
	.forcedMode = MODE_NONE,

	.ap = {
		{ .ssid = WIFI_DEFAULT_AP_SSID },
		{ .password = WIFI_DEFAULT_AP_PASSWORD },
		.channel = WIFI_DEFAULT_AP_CHANNEL,
		.maxConnection = WIFI_DEFAULT_AP_MAXCO,
		.isHidden = false,
		.ip = WIFI_DEFAULT_AP_IP,
		.gateway = WIFI_DEFAULT_AP_GATEWAY,
		.subnet = WIFI_DEFAULT_AP_SUBNET
	},
	.client = {
		{ .ssid = WIFI_DEFAULT_CLIENT_SSID },
		{ .password = WIFI_DEFAULT_CLIENT_PASSWORD },
		.delayBeforeAPFallbackMs = WIFI_DEFAULT_DELAY_AP_FALLBACK,
		.lastIp = IP_TO_U32(0, 0, 0, 0)
	}
};
// clang-format on

// =====================
// STATIC FUNCTIONS
// =====================

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
				-1, // We don't use wifiHandle->ap.channel, it will use default
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
	clientConfigSucceded = false;

	WiFi.mode(WIFI_STA);
	WiFi.setAutoReconnect(true);

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
	if ((pWifiHandle->ap.channel < WIFI_CHANNEL_MIN) || (pWifiHandle->ap.channel > WIFI_CHANNEL_MAX)) {
		reason = "[AP] Canal invalide: " + String(pWifiHandle->ap.channel) + " [" + String(WIFI_CHANNEL_MIN) + "-" + String(WIFI_CHANNEL_MAX) + "]";
		return false;
	}
	if ((pWifiHandle->ap.maxConnection < WIFI_MAX_CO_MIN) || (pWifiHandle->ap.maxConnection > WIFI_MAX_CO_MAX)) {
		reason = "[AP] Connexion max. invalide [" + String(WIFI_MAX_CO_MIN) + "-" + String(WIFI_MAX_CO_MAX) + "]";
		return false;
	}
	if (strlen(pWifiHandle->client.ssid) < 5) {
		reason = "[Client] SSID trop court (5 car.)";
		return false;
	}
	if (strlen(pWifiHandle->client.password) < 5) {
		reason = "[Client] Mot de passe trop court (5 car.)";
		return false;
	}
	if ((pWifiHandle->client.delayBeforeAPFallbackMs <= 0) || (pWifiHandle->client.delayBeforeAPFallbackMs > 60000)) {
		reason = "[Client] Délais de passage en AP (60s max)";
		return false;
	}

	return true;
}

static int32_t wifi_init_with_mode(void)
{
	int32_t ret;

	// Init wifi according to mode
	if (wifiHandle->mode == MODE_AP) {
		ret = wifi_ap_init();
	} else if (wifiHandle->mode == MODE_CLIENT) {
		ret = wifi_client_init();
	} else {
		log_error("Wifi mode not supported: %d", wifiHandle->mode);
		return -1;
	}

	return ret;
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
	if (wifi_init_with_mode()) {
		// Don't panic and do something
		log_error("Unknown wifi mode after scan ended: %d", wifiHandle->mode);
		wifi_fallback_as_ap();
	}

	// Disable fastReconnect
	wifiFastReconnect.isValid = false;
	wifiFastReconnect.mode    = MODE_NONE;

#if defined(MODULE_BUZZER) && defined(BUZZERS_ALARM)
	buzzer_set_melody(BUZZERS_ALARM, 3, false);
#endif

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

	// If in connected client mode, copy some settings to speed up reconnection
	if ((wifiHandle->mode == MODE_CLIENT) && (WiFi.status() == WL_CONNECTED)) {
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
 * @brief Check the current IP address of Client mode
 * and save it to flash if this is a new one
 * @return 0: OK, -1: error
 */
static int32_t wifi_save_current_ip(void)
{
	uint32_t  curIp;
	IPAddress lastIp = IPAddress(wifiHandle->client.lastIp);

	// Do nothing if not client
	if (wifiHandle->mode != MODE_CLIENT) {
		return -1;
	}

	// Get current ip adress and compare
	curIp = (uint32_t) WiFi.localIP();
	if (curIp == wifiHandle->client.lastIp) {
		return 0;
	}

	log_info("Saving new IP Address: %s (Previous was %s)",
			 WiFi.localIP().toString().c_str(),
			 lastIp.toString().c_str());

	// Save it in flash
	wifiHandle->client.lastIp = curIp;
	return flash_write();
}

static const char * wifi_get_mode_str(WIFI_MODE_E mode)
{
	if (mode > MODE_MAX) {
		return outOfRangeStr;
	}
	return wifiModeStr[(int) mode];
}

// =====================
// PUBLIC FUNCTIONS
// =====================

void wifi_print(void)
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

void wifi_print_config(wifi_handle_t * pHandle)
{
	// Use current if null specified
	if (pHandle == NULL) {
		pHandle = wifiHandle;
	}

	log_raw("mode:            %s\n\r", wifi_get_mode_str(pHandle->mode));
	log_raw("userMode:        %s\n\r", wifi_get_mode_str(pHandle->userMode));
	log_raw("forcedMode:      %s\n\r", wifi_get_mode_str(pHandle->forcedMode));
	log_raw("AP:\n\r");
	log_raw("\tssid:          %s\n\r", pHandle->ap.ssid);
	log_raw("\tpassord:       <not displayed>\n\r");
	log_raw("\tchannel:       %d\n\r", pHandle->ap.channel);
	log_raw("\tmaxCo:         %d\n\r", pHandle->ap.maxConnection);
	log_raw("\tisHidden:      %s\n\r", pHandle->ap.isHidden ? "on" : "off");
	log_raw("\tip:            %s\n\r", IPAddress(pHandle->ap.ip).toString().c_str());
	log_raw("\tgateway:       %s\n\r", IPAddress(pHandle->ap.gateway).toString().c_str());
	log_raw("\tsubnet:        %s\n\r", IPAddress(pHandle->ap.subnet).toString().c_str());
	log_raw("Client:\n\r");
	log_raw("\tssid:          %s\n\r", pHandle->client.ssid);
	log_raw("\tpassord:       <not displayed>\n\r");
	log_raw("\tdelayBeforeAP: %ds\n\r", pHandle->client.delayBeforeAPFallbackMs / 1000);
	log_raw("\tlastIp:        %s\n\r", IPAddress(pHandle->client.lastIp).toString().c_str());
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
 * @param delay : delay in tick before starting scan request
 * @return -1: Scan have been trigger, call latter for result
 */
int32_t wifi_start_scan_req(uint32_t delay)
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

	log_info("Wifi scan requested, starting in %d ms", delay);

	// Trigger a new scan
	isScanToStartTick = tick + delay;
	return 0;
}

int wifi_init(void)
{
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

	// Test mode
	if (wifi_init_with_mode()) {
		// Somehow, we managed to save a wrong configuration
		// Use default for next reset
		wifi_use_default_settings();
	}

	wifi_print();

	// Scan wifi after startup (50ms delay)
	// Scan cannot be trigger right now because
	// OTA and web server requieres WiFi to be up
	wifi_start_scan_req(50);

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

				/* If couldn't connect as client after some time, reboot in AP mode
				 * Checking if client config worked before avoid fallback during
				 * temporary network down time */
				if ((tick >= APFallbackTick) && (clientConfigSucceded == false)) {
					APFallbackTick = UINT32_MAX;
					wifi_fallback_as_ap();
				}
			} else {
				// Did we just get connected ?
				if (_isunset(STATUS_WIFI, STATUS_WIFI_IS_CO)) {
					_set(STATUS_WIFI, STATUS_WIFI_IS_CO);

					clientConfigSucceded = true;
					wifi_print();
					wifi_save_current_ip();
					ota_configure_mdns();
				}
			}
		}
	}

	// Check is a wifi scan is requested
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