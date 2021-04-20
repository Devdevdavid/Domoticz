/**
  * @file   term.cpp
  * @brief  Terminal management
  * @author David DEVANT
  * @date   19/12/2020
  */

#include "term.hpp"
#include "cmd.hpp"
#include "global.hpp"
#include "relay/relay.hpp"
#include "serial.hpp"
#include "telnet.hpp"
#include "wifi/wifi.hpp"

extern "C" {
#include "cli.h"
}

#ifdef MODULE_TERM

//========================
//	  STATIC FUNCTIONS
//========================

/**
 * @brief Default callback for leaf tokens
 *
 * @param argc Argument count
 * @param argv Argument values
 *
 * @return The status of the function
 */
int print_args(uint8_t argc, char * argv[])
{
	printf("print_args() Found %d args:\n\r", argc);
	for (uint8_t i = 0; i < argc; ++i) {
		printf("\t%s\n\r", argv[i]);
	}
	return 0;
}

// GENERIC TOOLS

/**
 * @brief Search for "on"/"off/"toggle" inside
 * argv[] and update according boolean
 *
 * @param flag Pointer to boolean
 * @param argc Argument count
 * @param argv Argument value
 * @return 0: bool updated, -1: error
 */
static int call_set_bool(bool * flag, uint8_t argc, char * argv[])
{
	if (argc != 1) {
		term_print("Missing argument");
		return -1;
	} else if (strcmp(argv[0], "on") == 0) {
		(*flag) = true;
	} else if (strcmp(argv[0], "off") == 0) {
		(*flag) = false;
	} else if (strcmp(argv[0], "toggle") == 0) {
		(*flag) = !(*flag);
	} else {
		term_print("Unknown argument: " + String(argv[0]));
		return -1;
	}
	return 0;
}

static int call_reset(uint8_t argc, char * argv[])
{
	cmd_reset_module();
	return 0;
}

static int call_print_status(uint8_t argc, char * argv[])
{
	cmd_print_status();
	return 0;
}

static int call_flash_setting_reset(uint8_t argc, char * argv[])
{
	return cmd_flash_setting_reset();
}

#ifdef MODULE_STRIPLED
static int call_stripled_set_brightness(uint8_t argc, char * argv[])
{
	if (strcmp(argv[0], "auto") == 0) {
		cmd_set_brightness_auto(true);
	} else {
		uint8_t brightness = strtol(argv[0], NULL, 10);
		if (brightness > 100) {
			term_print("Bad brightness level: " + String(brightness));
			return -1;
		}
		cmd_set_brightness(brightness);
	}
	return 0;
}

static int call_stripled_set_state(uint8_t argc, char * argv[])
{
	int  ret;
	bool flag;

	flag = cmd_get_state();
	ret  = call_set_bool(&flag, argc, argv);
	if (ret == 0) {
		cmd_set_state(flag);
	}
	return ret;
}

static int call_stripled_set_demo_mode(uint8_t argc, char * argv[])
{
	int  ret;
	bool flag;

	flag = cmd_get_demo_mode();
	ret  = call_set_bool(&flag, argc, argv);
	if (ret == 0) {
		cmd_set_demo_mode(flag);
	}
	return ret;
}

static int call_stripled_set_anim(uint8_t argc, char * argv[])
{
	if (strcmp(argv[0], "off") == 0) {
		return cmd_set_animation(0);
	} else {
		uint8_t animID = strtol(argv[0], NULL, 10);
		return cmd_set_animation(animID);
	}
}
#endif

#ifdef MODULE_RELAY
static int call_relay_set_state(uint8_t argc, char * argv[])
{
	int  ret;
	bool flag;

	flag = relay_get_state();
	ret  = call_set_bool(&flag, argc, argv);
	if (ret == 0) {
		relay_set_state(flag);
	}
	return ret;
}
#endif

#ifdef MODULE_STATUS_LED
static int call_status_led_set_state(uint8_t argc, char * argv[])
{
	int  ret;
	bool flag;

	flag = _isset(STATUS_APPLI, STATUS_APPLI_STATUS_LED);
	ret  = call_set_bool(&flag, argc, argv);
	if (ret == 0) {
		cmd_set_status_led(flag);
	}
	return ret;
}
#endif

// ===============
// WIFI
// ===============

static wifi_handle_t wifiHandleTmp;

static int call_wifi_show_config(uint8_t argc, char * argv[])
{
	wifi_print_config(&wifiHandleTmp);
	return 0;
}

static int call_wifi_set_user_mode(uint8_t argc, char * argv[])
{
	if (strcmp(argv[0], "ap") == 0) {
		wifiHandleTmp.userMode = MODE_AP;
	} else if (strcmp(argv[0], "client") == 0) {
		wifiHandleTmp.userMode = MODE_CLIENT;
	} else {
		term_print("Unknown argument: " + String(argv[0]));
		return -1;
	}
	return 0;
}

static int call_wifi_ap_set_ssid(uint8_t argc, char * argv[])
{
	char * ssid = argv[0];
	if (strlen(ssid) >= WIFI_SSID_MAX_LEN) {
		return -1;
	}
	strncpy(wifiHandleTmp.ap.ssid, ssid, WIFI_SSID_MAX_LEN);
	return 0;
}

static int call_wifi_ap_set_password(uint8_t argc, char * argv[])
{
	char * password = argv[0];
	if (strlen(password) >= WIFI_PASSWORD_MAX_LEN) {
		return -1;
	}
	strncpy(wifiHandleTmp.ap.password, password, WIFI_PASSWORD_MAX_LEN);
	return 0;
}

static int call_wifi_ap_set_channel(uint8_t argc, char * argv[])
{
	uint8_t channel = strtol(argv[0], NULL, 10);
	if ((channel < WIFI_CHANNEL_MIN) || (channel > WIFI_CHANNEL_MAX)) {
		return -1;
	}
	wifiHandleTmp.ap.channel = channel;
	return 0;
}

static int call_wifi_ap_set_max_connection(uint8_t argc, char * argv[])
{
	uint8_t maxConnection = strtol(argv[0], NULL, 10);
	if ((maxConnection < WIFI_MAX_CO_MIN) || (maxConnection > WIFI_MAX_CO_MAX)) {
		return -1;
	}
	wifiHandleTmp.ap.maxConnection = maxConnection;
	return 0;
}

static int call_wifi_ap_set_is_hidden(uint8_t argc, char * argv[])
{
	int  ret;
	bool flag;

	flag = wifiHandleTmp.ap.isHidden; // isHidden can't be passed as reference
	ret  = call_set_bool(&flag, argc, argv);
	if (ret == 0) {
		wifiHandleTmp.ap.isHidden = flag;
	}
	return ret;
}

static int call_wifi_ap_set_ip_address(uint8_t argc, char * argv[])
{
	char *    ipAddress = argv[0];
	IPAddress addr;
	if (addr.fromString(ipAddress) == false) {
		return -1;
	}
	wifiHandleTmp.ap.ip = (uint32_t) addr;
	return 0;
}

static int call_wifi_ap_set_gateway(uint8_t argc, char * argv[])
{
	char *    gateway = argv[0];
	IPAddress addr;
	if (addr.fromString(gateway) == false) {
		return -1;
	}
	wifiHandleTmp.ap.gateway = (uint32_t) addr;
	return 0;
}

static int call_wifi_ap_set_subnet(uint8_t argc, char * argv[])
{
	char *    subnet = argv[0];
	IPAddress addr;
	if (addr.fromString(subnet) == false) {
		return -1;
	}
	wifiHandleTmp.ap.subnet = (uint32_t) addr;
	return 0;
}

static int call_wifi_client_set_ssid(uint8_t argc, char * argv[])
{
	char * ssid = argv[0];
	if (strlen(ssid) >= WIFI_SSID_MAX_LEN) {
		return -1;
	}
	strncpy(wifiHandleTmp.client.ssid, ssid, WIFI_SSID_MAX_LEN);
	return 0;
}

static int call_wifi_client_set_password(uint8_t argc, char * argv[])
{
	char * password = argv[0];
	if (strlen(password) >= WIFI_PASSWORD_MAX_LEN) {
		return -1;
	}
	strncpy(wifiHandleTmp.client.password, password, WIFI_PASSWORD_MAX_LEN);
	return 0;
}

static int call_wifi_client_set_delay_before_ap(uint8_t argc, char * argv[])
{
	uint32_t delayBeforeAPFallbackMs = strtol(argv[0], NULL, 10);
	if (delayBeforeAPFallbackMs < WIFI_DELAY_AP_FALLBACK_MIN) {
		return -1;
	}
	wifiHandleTmp.client.delayBeforeAPFallbackMs = delayBeforeAPFallbackMs;
	return 0;
}

static int call_wifi_load_settings(uint8_t argc, char * argv[])
{
	memcpy(&wifiHandleTmp, wifi_get_handle(), sizeof(wifiHandleTmp));
	return 0;
}

static int call_wifi_use_new_settings(uint8_t argc, char * argv[])
{
	int    ret;
	String reason;

	// Check if config changed
	ret = memcmp(&wifiHandleTmp, wifi_get_handle(), sizeof(wifiHandleTmp));
	if (ret == 0) {
		log_error("Wifi configuration unchanged !");
		return -1;
	}

	// Apply new configuration
	ret = wifi_use_new_settings(&wifiHandleTmp, reason);
	if (ret != 0) {
		log_error("Unable to save settings: %s", reason.c_str());
	}
	return ret;
}

/**
 * @brief Create the command line interface
 */
static int term_create_cli_commands(void)
{
	cli_token * tokRoot = cli_get_root_token();
	cli_token * tokLvl1, *tokLvl2, *tokLvl3;
	cli_token * curTok;

	// Create commands
	tokLvl1 = cli_add_token("board", "Board configuration");
	{
		curTok = cli_add_token("reset", "Reset module");
		cli_set_callback(curTok, &call_reset);
		cli_add_children(tokLvl1, curTok);

		curTok = cli_add_token("status", "Print current status");
		cli_set_callback(curTok, &call_print_status);
		cli_add_children(tokLvl1, curTok);
	}
	cli_add_children(tokRoot, tokLvl1);

	tokLvl1 = cli_add_token("flash", "Manage flash memory");
	{
		curTok = cli_add_token("default", "Reset flash setting to default");
		cli_set_callback(curTok, &call_flash_setting_reset);
		cli_add_children(tokLvl1, curTok);
	}
	cli_add_children(tokRoot, tokLvl1);

#ifdef MODULE_STRIPLED
	tokLvl1 = cli_add_token("strip", "Manage stripled");
	{
		curTok = cli_add_token("brightness", "[level|auto] Set the brightness");
		cli_set_callback(curTok, &call_stripled_set_brightness);
		cli_set_argc(curTok, 1, 0);
		cli_add_children(tokLvl1, curTok);

		curTok = cli_add_token("state", "[on|off|toggle] Set state of the stripled");
		cli_set_callback(curTok, &call_stripled_set_state);
		cli_set_argc(curTok, 1, 0);
		cli_add_children(tokLvl1, curTok);

		curTok = cli_add_token("demo", "[on|off|toggle] Set demo mode");
		cli_set_callback(curTok, &call_stripled_set_demo_mode);
		cli_set_argc(curTok, 1, 0);
		cli_add_children(tokLvl1, curTok);

		curTok = cli_add_token("anim", "[animID|off] Set the animation ID");
		cli_set_callback(curTok, &call_stripled_set_anim);
		cli_set_argc(curTok, 1, 0);
		cli_add_children(tokLvl1, curTok);
	}
	cli_add_children(tokRoot, tokLvl1);
#endif

#ifdef MODULE_RELAY
	tokLvl1 = cli_add_token("relay", "Manage relay");
	{
		curTok = cli_add_token("set", "[on|off|toggle] Set the relay state");
		cli_set_callback(curTok, &call_relay_set_state);
		cli_set_argc(curTok, 1, 0);
		cli_add_children(tokLvl1, curTok);
	}
	cli_add_children(tokRoot, tokLvl1);
#endif

#ifdef MODULE_STATUS_LED
	tokLvl1 = cli_add_token("led", "Manage LED");
	{
		curTok = cli_add_token("set", "[on|off|toggle] Set the relay state");
		cli_set_callback(curTok, &call_status_led_set_state);
		cli_set_argc(curTok, 1, 0);
		cli_add_children(tokLvl1, curTok);
	}
	cli_add_children(tokRoot, tokLvl1);
#endif

	tokLvl1 = cli_add_token("wifi", "Wifi configuration");
	{
		curTok = cli_add_token("show", "Show edited configuration");
		cli_set_callback(curTok, &call_wifi_show_config);
		cli_set_argc(curTok, 0, 1);
		cli_add_children(tokLvl1, curTok);

		curTok = cli_add_token("mode", "<ap|client> Set user mode for wifi");
		cli_set_callback(curTok, &call_wifi_set_user_mode);
		cli_set_argc(curTok, 1, 0);
		cli_add_children(tokLvl1, curTok);

		tokLvl2 = cli_add_token("client", "Client configuration");
		{
			tokLvl3 = cli_add_token("set", "Define new configuration");
			{
				curTok = cli_add_token("ssid", "<ssid> Set SSID to connect to");
				cli_set_callback(curTok, &call_wifi_client_set_ssid);
				cli_set_argc(curTok, 1, 0);
				cli_add_children(tokLvl3, curTok);

				curTok = cli_add_token("pwd", "<password> Set password");
				cli_set_callback(curTok, &call_wifi_client_set_password);
				cli_set_argc(curTok, 1, 0);
				cli_add_children(tokLvl3, curTok);

				curTok = cli_add_token("delayBeforeAP", "<ms> Set delay before fallback to AP");
				cli_set_callback(curTok, &call_wifi_client_set_delay_before_ap);
				cli_set_argc(curTok, 1, 0);
				cli_add_children(tokLvl3, curTok);
			}
			cli_add_children(tokLvl2, tokLvl3);
		}
		cli_add_children(tokLvl1, tokLvl2);

		tokLvl2 = cli_add_token("ap", "Access Point configuration");
		{
			tokLvl3 = cli_add_token("set", "Define new configuration");
			{
				curTok = cli_add_token("ssid", "<ssid> Set SSID for access point");
				cli_set_callback(curTok, &call_wifi_ap_set_ssid);
				cli_set_argc(curTok, 1, 0);
				cli_add_children(tokLvl3, curTok);

				curTok = cli_add_token("pwd", "<password> Set password");
				cli_set_callback(curTok, &call_wifi_ap_set_password);
				cli_set_argc(curTok, 1, 0);
				cli_add_children(tokLvl3, curTok);

				curTok = cli_add_token("ip", "<address> Set IP adress");
				cli_set_callback(curTok, &call_wifi_ap_set_ip_address);
				cli_set_argc(curTok, 1, 0);
				cli_add_children(tokLvl3, curTok);

				curTok = cli_add_token("gateway", "<address> Set gateway adress");
				cli_set_callback(curTok, &call_wifi_ap_set_gateway);
				cli_set_argc(curTok, 1, 0);
				cli_add_children(tokLvl3, curTok);

				curTok = cli_add_token("subnet", "<address> Set sub-network mask");
				cli_set_callback(curTok, &call_wifi_ap_set_subnet);
				cli_set_argc(curTok, 1, 0);
				cli_add_children(tokLvl3, curTok);

				curTok = cli_add_token("channel", "<number> Set channel [1-13]");
				cli_set_callback(curTok, &call_wifi_ap_set_channel);
				cli_set_argc(curTok, 1, 0);
				cli_add_children(tokLvl3, curTok);

				curTok = cli_add_token("maxco", "<number> Set max connection [1-3]");
				cli_set_callback(curTok, &call_wifi_ap_set_max_connection);
				cli_set_argc(curTok, 1, 0);
				cli_add_children(tokLvl3, curTok);

				curTok = cli_add_token("hidden", "<on|off> Set if AP is hidden [1-3]");
				cli_set_callback(curTok, &call_wifi_ap_set_is_hidden);
				cli_set_argc(curTok, 1, 0);
				cli_add_children(tokLvl3, curTok);
			}
			cli_add_children(tokLvl2, tokLvl3);
		}
		cli_add_children(tokLvl1, tokLvl2);

		curTok = cli_add_token("load", "Load current configuration");
		cli_set_callback(curTok, &call_wifi_load_settings);
		cli_add_children(tokLvl1, curTok);

		curTok = cli_add_token("save", "Save all configuration updates");
		cli_set_callback(curTok, &call_wifi_use_new_settings);
		cli_add_children(tokLvl1, curTok);
	}
	cli_add_children(tokRoot, tokLvl1);
	return 0;
}

//========================
//	  PUBLIC FUNCTIONS
//========================

int term_init(void)
{
	cli_init();
	term_create_cli_commands();
	return 0;
}

void term_rx(uint8_t byte)
{
	cli_rx(byte);
}

void term_print(String str)
{
	uint32_t  len  = str.length();
	uint8_t * data = (uint8_t *) str.c_str();

#ifdef MODULE_SERIAL
	serial_write(data, len);
#endif
#ifdef MODULE_TELNET
	telnet_write(data, len);
#endif
}

#endif /* MODULE_TERM */