/*
 Name:		WiFi_Manager.h
 Created:	9/19/2018 1:05:18 PM
 Author:	Lucas
*/
#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include "esp_task_wdt.h"
#include "index.h"
#include "wifi_svg.h"
// #include "index.h"
#define WIFI_getChipId() (uint32_t) ESP.getEfuseMac()
#define FW_VERSION "1.0.0"
#define DEBUG 1

#ifndef FW_VERSION
#define FW_VERSION "Undefined firmware version"
#endif // !FIRM_VERSION

#ifndef _WiFi_Manager_h
#define _WiFi_Manager_h

#ifdef DEBUG
#define DEBUG_WIFI_MANAGER(...) Serial.print(__VA_ARGS__)
#define DEBUG_WIFI_MANAGERLN(...) Serial.println(__VA_ARGS__)
#define DEBUG_WIFI_MANAGERF(...) Serial.printf(__VA_ARGS__)
#endif

#ifndef DEBUG
#define DEBUG_WIFI_MANAGER(...)
#define DEBUG_WIFI_MANAGERLN(...)
#define DEBUG_WIFI_MANAGERF(...)
#endif

#ifndef DEFAULTSSID
#define DEFAULTSSID "PEDRA" // default SSID when not configured
#define DEFAULTPASS ""
#endif

#ifndef DEFAULT_W_USER
#define DEFAULT_W_USER "admin"
#endif

#ifndef DEFAULT_W_PASS
#define DEFAULT_W_PASS "admin"
#endif
#ifndef DEFAULT_IP
#define DEFAULT_IP "192.168.4.1"
#endif
#ifndef TIMERCONNECT
#define TIMERCONNECT 10000 // timer in milliseconds wait for connect in Access Point
#endif

struct wifiManager_config
{
	uint8_t ssid[32];
	uint8_t password[64];
	uint8_t ipAP[16];
	uint8_t wwwuser[32];
	uint8_t wwwpass[64];
	int DHCP;
	int enableAP = 1;
	uint8_t STAIP[16];
	uint8_t STAMASK[16];
	uint8_t STAGWY[16];
};

struct custom_config
{
	String id;
	String name;
	String value;
};
class WiFiManager
{

protected:
	AsyncWebServer *webServer;
	custom_config *_custom_opt;
	int _custom_opt_length;
	void StartLittleFS();
	void StartWIFI();
	void hardreset();
	void CommandHandler(char *command, uint32_t num);
	void WebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
						void *arg, uint8_t *data, size_t len);
	String getContentType(String filename);
	bool handleFileRead(AsyncWebServerRequest *request);
	String Status();
	void PersistConfig(char *SSIDAP = NULL, char *passwordAP = NULL, char *APIP = NULL, char *wwwuser = NULL, char *wwwpass = NULL, int DHCP = -1, char *STAIP = NULL, char *STAMASK = NULL, char *STAGWY = NULL, int enableAP = -1);
	void WriteConfig(wifiManager_config *config);
	bool Read(wifiManager_config *conf_return);
	void handlerSocketMessage(void *arg, uint8_t *data, size_t len, uint32_t client_id);

	static void taskAPTimeout(void *params);

public:
	bool apIsActive = true;
	uint32_t _timeout_ap = 0;

	void StartSoftAP();
	void Start(AsyncWebServer *server, custom_config *custom_opt, int custom_opt_length, uint32_t timeoutAP = 0);
	void loop();
	void offWiFi();
	void onWiFi(bool startAP = false);
	String WifiScan();
};
extern wifiManager_config currentConfig;
#endif