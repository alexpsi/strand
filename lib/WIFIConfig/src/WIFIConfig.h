
#include "Arduino.h"
#include <EEPROMSettings.h>

#if defined(ESP8266)
#define BOARD_ESP 8266
#elif defined(ESP32)
#define BOARD_ESP 32
#elif defined(__AVR__)
#define BOARD_ESP 0
#error Architecture is AVR instead of ESP8266 OR ESP32
#else
#define BOARD_ESP 0
#error Architecture is NOT ESP8266 OR ESP32
#endif

#if BOARD_ESP == 32
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFiMulti.h>
#elif BOARD_ESP == 8266
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#endif

#include <DNSServer.h>

#define ESP_settings_size 4
#define EEPROM_SIZE 4096

class ESPWifiConfig;

#define AP_MODE_NAME "AP_MODE"
#define CLIENT_MODE_NAME "CLIENT_MODE"

enum ESP_MODES
{
	AP_MODE,
	CLIENT_MODE
};

class ESPWifiConfig
{
	const char *sys_name;
	boolean show_debug = false;
	int reset_btn = 0;
#if BOARD_ESP == 32
	WebServer server;
#elif BOARD_ESP == 8266
	ESP8266WebServer server;
#endif

public:
	ESPWifiConfig(const char *sys_namex, int port, int thisreset_btn, boolean debug) : server(port)
	{
		sys_name = sys_namex;
		reset_btn = thisreset_btn;
		show_debug = debug;
	};
	IPAddress ESP_IP;
	const byte DNS_PORT = 53;
	String get_AP_name();
	IPAddress apIP = {192, 168, 1, 1};
	DNSServer dnsServer;
	int ESP_mode = 0;
	boolean isHTTPserverRunning = false;
	unsigned long last_conn_to_http = 0;


	boolean is_reset_pressed(int);
	unsigned long getmacID(void);
	int initialize(void);
	boolean wifi_connected = false;
	String error_msg = "";
	unsigned char reset_pressed_count = 0;
	void handle(unsigned long);
	void Start_HTTP_Server(unsigned long);
	void ESP_debug(String);
	String debug_log = "";
	String input = "";

private:
	boolean try_wifi_connect();
#if BOARD_ESP == 32
	WiFiMulti wifiMulti;
#elif BOARD_ESP == 8266
	ESP8266WiFiMulti wifiMulti;
#endif
	String ESP_IP_addresss = "";
	unsigned long last_try_wifi_connect = 0;
	unsigned long wild_scan_timer = 60000;
	unsigned long client_mode_active_time = 60000;
	unsigned int fails_try_wifi_connect = 0;

	void handleNotFound(void);
	void handle_setup(void);
};