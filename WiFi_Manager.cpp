/*
 Name:		WiFi_Manager.cpp
 Created:	9/19/2018 1:05:18 PM
 Author:	Lucas
*/

#include "WiFi_Manager.h"

const byte DNS_PORT = 53;

IPAddress netMsk(255, 255, 255, 0);
DNSServer dnsServer;
AsyncWebSocket ws("/");
bool tryConnect = false;
bool checkCon = false;

String response;

wifiManager_config currentConfig;

void WiFiManager::taskAPTimeout(void *params)
{
	bool ap_on = true;
	int count = 0;
	WiFiManager *instancia = static_cast<WiFiManager *>(params);
	while (ap_on)
	{
		delay(instancia->_timeout_ap);
		if (WiFi.softAPgetStationNum() < 1)
		{
			ap_on = false;
		}
		if (count > 5)
			ap_on = false;
		count++;
	}
	WiFi.softAPdisconnect();
	if (WiFi.getMode() != WIFI_AP_STA)
		WiFi.mode(WIFI_OFF);
	else
		WiFi.mode(WIFI_STA);
	instancia->apIsActive = false;
	vTaskDelete(NULL);
}

void WiFiManager::StartSoftAP()
{
	if (WiFi.getMode() == WIFI_AP_STA || WiFi.getMode() == WIFI_AP)
	{
		DEBUG_WIFI_MANAGERLN("AP mode already");
		return;
	}
	WiFi.mode(WIFI_AP_STA);
	DEBUG_WIFI_MANAGERLN("Starting AP mode");

	char *ssidAp = reinterpret_cast<char *>(currentConfig.ssid);
	char *passAp = reinterpret_cast<char *>(currentConfig.password);
	IPAddress ip;
	const char *ipStr = (char *)currentConfig.ipAP;
	ip.fromString(ipStr);

	WiFi.softAP(ssidAp, passAp); // Call to 2 time to fix bug

	WiFi.softAPConfig(ip, ip, netMsk);

	delay(500);

	if (WiFi.softAP(ssidAp, passAp))
	{
		PersistConfig(NULL, NULL, NULL, NULL, NULL, -1, NULL, NULL, NULL, 1);
		DEBUG_WIFI_MANAGERLN("AP mode started");
		DEBUG_WIFI_MANAGER("SSID: " + WiFi.softAPSSID() + " PASSWORD: " + passAp);
		DEBUG_WIFI_MANAGERLN(" IP: " + ip.toString());
	}
}

void WiFiManager::StartLittleFS()
{
	LittleFS.begin(true);
	if (!Read(&currentConfig))
	{
		String defaultSSID = String(DEFAULTSSID) + "_" + String(WIFI_getChipId(), HEX);
		defaultSSID.toUpperCase();
		char *ssid = (char *)defaultSSID.c_str();
		char *passAP = DEFAULTPASS;
		char *wwwUser = DEFAULT_W_USER;
		char *wwwPass = DEFAULT_W_PASS;
		char *IPAP = DEFAULT_IP;
		PersistConfig(ssid, passAP, IPAP, wwwUser, wwwPass, 0, NULL, NULL, NULL, 1);
	}
}

void WiFiManager::StartWIFI()
{
	WiFi.persistent(true);
	WiFi.begin();
	WiFi.setAutoReconnect(true);
	if (WiFi.waitForConnectResult() != WL_CONNECTED)
	{
		StartSoftAP();
	}
	if (currentConfig.enableAP == 1)
	{
		StartSoftAP();
	}
	if (currentConfig.DHCP == 1)
	{ // Check if IP Static is ative
		IPAddress ipsta;
		IPAddress gate;
		IPAddress mask;
		ipsta.fromString((char *)currentConfig.STAIP);
		gate.fromString((char *)currentConfig.STAGWY);
		mask.fromString((char *)currentConfig.STAMASK);
		WiFi.config(ipsta, gate, mask);
	}

	if (WiFi.getMode() == WIFI_AP_STA || WiFi.getMode() == WIFI_AP)
	{
		IPAddress ip;
		const char *ipStr = (char *)currentConfig.ipAP;
		ip.fromString(ipStr);
		WiFi.softAPConfig(ip, ip, netMsk);
	}
	if (this->_timeout_ap > 0)
	{
		xTaskCreate(
			taskAPTimeout,	 /* Task function. */
			"wifiApTimeout", /* String with name of task. */
			1024,			 /* Stack size in bytes. */
			this,			 /* Parameter passed as input of the task */
			1,				 /* Priority of the task. */
			NULL);			 /* Task handle. */
	}
}

// void WiFiManager::hardreset()
// {
// 	if (!webServer.authenticate((char *)currentConfig.wwwuser, (char *)currentConfig.wwwpass))
// 	{
// 		return webServer.requestAuthentication();
// 	}
// 	webServer.send(200, "text/plain", "Reiniciando");
// 	LittleFS.remove("/config.conf");
// 	DEBUG_WIFI_MANAGER("Resetando....");
// 	ESP.eraseConfig();
// 	DEBUG_WIFI_MANAGER("OK....");
// 	DEBUG_WIFI_MANAGERLN("Reiniciando");
// 	char *ssid = DEFAULTSSID;
// 	char *passAP = DEFAULTPASS;
// 	WiFi.softAP(ssid, passAP); // Start AP
// 	ESP.restart();
// }

void WiFiManager::CommandHandler(char *command, uint32_t num)
{
	char *cmd = command;
	char *arg1;
	char *arg2;
	cmd = strtok(cmd, "|");
	arg1 = strtok(NULL, "|");
	arg2 = strtok(NULL, "|");

	Serial.println(command);
	if (String(command) == "wifiScan")
	{
		String redes = WifiScan();
		ws.text(num, redes);
	}

	else if (String(command) == "wifiConnect")
	{
		WiFi.begin(arg1, arg2);
		tryConnect = true;
	}

	else if (String(command) == "deviceStatus")
	{
		String status = Status();
		ws.text(num, status);
	}

	else if (String(command) == "wifiConfigAP")
	{
		String response = "{\"apConfig\":{\"success\": false}}";
		arg2 = (String(arg2) == "NULL") ? (char *)"" : arg2;
		if (WiFi.softAP(arg1, arg2))
		{
			char *arg3 = strtok(NULL, "|");
			IPAddress ip;
			ip.fromString(arg3);
			WiFi.softAPConfig(ip, ip, netMsk);
			response = "{\"apConfig\":{\"success\": true}}";
			PersistConfig(arg1, arg2, arg3, NULL, NULL, -1, NULL, NULL, NULL, 1); // Store SSID AP , PASS AP, IP AP
		};
		ws.text(num, response);
	}

	else if (String(command) == "desativeAP")
	{
		WiFi.softAPdisconnect();
		WiFi.mode(WIFI_STA);
		PersistConfig(NULL, NULL, NULL, NULL, NULL, -1, NULL, NULL, NULL, 0);
		String rpTrue = "{\"apConfig\":{\"success\": true}}";
		ws.text(num, rpTrue);
	}

	else if (String(command) == "STAIP")
	{
		char *arg3 = strtok(NULL, "|");
		char *arg4 = strtok(NULL, "|");
		int DHCP = atoi(arg4);
		PersistConfig(NULL, NULL, NULL, NULL, NULL, DHCP, arg1, arg2, arg3);
		if (DHCP != 0)
		{
			IPAddress ip;
			IPAddress mask;
			IPAddress gateway;
			ip.fromString(arg1);
			gateway.fromString(arg3);
			mask.fromString(arg2);
			WiFi.config(ip, gateway, mask);
		}
	}

	else if (String(command) == "wwwC")
	{
		PersistConfig(NULL, NULL, NULL, arg1, arg2);
	}
	else if (String(command) == "customConfig")
	{
		String opts = "{\"customConfig\":[";
		for (size_t i = 0; i < _custom_opt_length; i++)
		{
			opts += (i == 0 ? "" : ",") + String("{\"name\":\"" + _custom_opt[i].name + "\",\"id\":\"" + _custom_opt[i].id + "\",\"value\":\"" + _custom_opt[i].value + "\"}");
		}

		opts += "]}";
		ws.text(num, opts);
	}
}

void WiFiManager::handlerSocketMessage(void *arg, uint8_t *data, size_t len, uint32_t client_id)
{
	AwsFrameInfo *info = (AwsFrameInfo *)arg;
	if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
	{
		data[len] = 0;
		DEBUG_WIFI_MANAGERF("Get Text: %s\n", data);
		WiFiManager::CommandHandler((char *)data, client_id);
	}
}

void WiFiManager::WebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
								 void *arg, uint8_t *data, size_t len)
{
	switch (type)
	{
#ifdef DEBUG
	case WS_EVT_DISCONNECT:
		DEBUG_WIFI_MANAGERF("WebSocket client #%u disconnected\n", client->id());
		break;

	case WS_EVT_CONNECT:
		DEBUG_WIFI_MANAGERF("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
		break;
#endif
	case WS_EVT_DATA: // if new text data is received
		WiFiManager::handlerSocketMessage(arg, data, len, client->id());
		break;
	default:
		break;
	}
}

String WiFiManager::getContentType(String filename)
{ // convert the file extension to the MIME type
	if (filename.endsWith(".html"))
		return "text/html";
	else if (filename.endsWith(".css"))
		return "text/css";
	else if (filename.endsWith(".js"))
		return "application/javascript";
	else if (filename.endsWith(".ico"))
		return "image/x-icon";
	else if (filename.endsWith(".svg"))
		return "image/svg+xml";
	else if (filename.endsWith(".gz"))
		return "application/x-gzip";
	return "text/plain";
}

bool WiFiManager::handleFileRead(AsyncWebServerRequest *request)
{ // send the right file to the client (if it exists)
	String path = request->url();
	DEBUG_WIFI_MANAGERLN("handleFileRead: " + path);

	if (path.endsWith("/"))
	{
		AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html_gz, index_html_gz_len);
		response->addHeader("Content-Encoding", "gzip");
		request->send(response);
		return true;
	}
	else if (path.endsWith("wifi.svg"))
	{
		AsyncWebServerResponse *response = request->beginResponse_P(200, "image/svg+xml", wifi_svg_gz, wifi_svg_gz_len);
		response->addHeader("Content-Encoding", "gzip");
		request->send(response);
		return true;
	}

	request->send(LittleFS, path); // Send it to the client
	DEBUG_WIFI_MANAGERLN(String("\tSent file: ") + path);
	return true; // If the file doesn't exist, return false
}

String WiFiManager::WifiScan()
{ // Return JSON format of scanned wifi
	WiFi.scanNetworks(true);
	int n = WIFI_SCAN_RUNNING;
	while (n == WIFI_SCAN_RUNNING)
	{
		n = WiFi.scanComplete();
		esp_task_wdt_reset();
	}
	String connectedBSSID = "";
	if (WiFi.status() == WL_CONNECTED)
		connectedBSSID = WiFi.BSSIDstr();
	String serialize = "{\"wifiList\":[";
	for (int i = 0; i < n; ++i)
	{
		String rede = String((i > 0) ? "," : " ") +
					  "{\"name\":\"" + WiFi.SSID(i) +
					  "\",\"dBm\":\"" + WiFi.RSSI(i) +
					  "\",\"bssid\":\"" + WiFi.BSSIDstr(i) +
					  "\",\"enc_type\":\"" +
					  ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "open" : "close") +
					  "\",\"connected\":" + ((connectedBSSID == WiFi.BSSIDstr(i)) ? "true" : "false") +
					  "}";
		serialize += rede;

		// Print SSID and RSSI for each network found
		DEBUG_WIFI_MANAGER(i + 1);
		DEBUG_WIFI_MANAGER(": ");
		DEBUG_WIFI_MANAGER(WiFi.SSID(i));
		DEBUG_WIFI_MANAGER(" (");
		DEBUG_WIFI_MANAGER(WiFi.RSSI(i));
		DEBUG_WIFI_MANAGER(")");
		DEBUG_WIFI_MANAGERLN((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "open" : "*");
	}
	WiFi.scanDelete();
	serialize += "]}";
	// TODO Remove
	return serialize;
}

String WiFiManager::Status()
{ // Return JSON format of devices status
	return "{\"deviceStatus\":{\"firmware\": { \"version\": \"" + String(FW_VERSION) + "\"},\"mode\":" + String(WiFi.getMode()) +
		   ((WiFi.getMode() != 2) ? ",\"STA\":{\"connected\":" + ((WiFi.isConnected()) ? "true ,\"SSID\":\"" + WiFi.SSID() +
																							 "\",\"BSSID\":\"" + WiFi.BSSIDstr() +
																							 "\",\"signal\":\"" + WiFi.RSSI() +
																							 "\",\"IP\":\"" + WiFi.localIP().toString() +
																							 "\",\"channel\":\"" + WiFi.channel() + "\"}"
																					   : "false }")
								  : "") +
		   ",\"AP\":{\"SSID\":\"" + String((char *)currentConfig.ssid) +
		   "\",\"BSSID\":\"" + WiFi.softAPmacAddress() +
		   "\",\"IP\":\"" + String((char *)currentConfig.ipAP) +
		   "\",\"channel\":\"" + WiFi.channel() + "\"}}}";
}

void WiFiManager::PersistConfig(char *SSIDAP, char *passwordAP, char *IPAP, char *wwwuser, char *wwwpass, int DHCP, char *STAIP, char *STAMASK, char *STAGWY, int enableAP)
{
	DEBUG_WIFI_MANAGERLN("Persisting info");
	if (SSIDAP != NULL)
	{
		DEBUG_WIFI_MANAGERLN("Persisting sSSID");
		strcpy(reinterpret_cast<char *>(currentConfig.ssid), SSIDAP);
		if (passwordAP != NULL || passwordAP != "")
			strcpy(reinterpret_cast<char *>(currentConfig.password), passwordAP);
		DEBUG_WIFI_MANAGERLN("Persisting IPAP");
		if (IPAP != NULL)
			strcpy(reinterpret_cast<char *>(currentConfig.ipAP), IPAP);
	}
	DEBUG_WIFI_MANAGERLN("Persisting WWW");
	if (wwwuser != NULL)
	{
		strcpy(reinterpret_cast<char *>(currentConfig.wwwuser), wwwuser);
		strcpy(reinterpret_cast<char *>(currentConfig.wwwpass), wwwpass);
	}
	DEBUG_WIFI_MANAGERLN("Deffinindo DHCP");
	if (DHCP != -1)
		currentConfig.DHCP = DHCP;
	if (enableAP != -1)
		currentConfig.enableAP = enableAP;
	if (STAIP != NULL)
	{
		strcpy(reinterpret_cast<char *>(currentConfig.STAIP), STAIP);
		strcpy(reinterpret_cast<char *>(currentConfig.STAMASK), STAMASK);
		strcpy(reinterpret_cast<char *>(currentConfig.STAGWY), STAGWY);
	}
	DEBUG_WIFI_MANAGERLN("Preparto writting");
	WriteConfig(&currentConfig);
	DEBUG_WIFI_MANAGERLN("Config are saved");

	// TODO REMOVE
	wifiManager_config config2;
	if (Read(&config2))
	{
		DEBUG_WIFI_MANAGERF("SSID: %s PASSOWRD:%s\n IP:%s\n", config2.ssid, config2.password, config2.ipAP);
	}
	else
	{
		DEBUG_WIFI_MANAGERLN("CANNOT READ 2");
	}
}

void WiFiManager::WriteConfig(wifiManager_config *config)
{
	DEBUG_WIFI_MANAGERLN("Writing config in system");

	File configFile = LittleFS.open("/config.conf", "w+");

	if (!configFile)
	{
		DEBUG_WIFI_MANAGERLN(F("Failed to open and writing config file"));
	}
	else
	{
		DEBUG_WIFI_MANAGERLN(F("Opened file for update ...."));

		uint8_t *data = reinterpret_cast<uint8_t *>(config);
		size_t bytes = configFile.write(data, sizeof(wifiManager_config));

		DEBUG_WIFI_MANAGERF("Saved");

		configFile.close();
	}
}

bool WiFiManager::Read(wifiManager_config *conf_return)
{
	DEBUG_WIFI_MANAGERLN();
	DEBUG_WIFI_MANAGERLN("Reading config in system");
	File configFile = LittleFS.open("/config.conf", "r");

	if (!configFile)
	{
		DEBUG_WIFI_MANAGERLN(F("Failed to open config file"));
		return false;
	}
	else
	{
		DEBUG_WIFI_MANAGERLN(" Checksum : " + String(sizeof(wifiManager_config)) + " file: " + String(configFile.size()));
		if ((int)sizeof(wifiManager_config) == (int)configFile.size())
		{
			DEBUG_WIFI_MANAGERLN("Checksum OK!");
		}
		else
		{
			DEBUG_WIFI_MANAGERLN("CONFIGURATION FILE ARE CURRUPTED");
			return false;
		}
		DEBUG_WIFI_MANAGERLN(F("Opening config"));

		uint8_t uconf[sizeof(wifiManager_config)];

		for (int i = 0; i < sizeof(wifiManager_config); i++)
		{
			uconf[i] = configFile.read();
		}
		configFile.read(uconf, sizeof(wifiManager_config));
		DEBUG_WIFI_MANAGER(F("READING....."));

		wifiManager_config *config = (wifiManager_config *)uconf;

		memcpy(conf_return, config, sizeof(wifiManager_config));

		DEBUG_WIFI_MANAGERLN("READY");
		DEBUG_WIFI_MANAGERLN("Load configurations");

		configFile.close();
		return true;
	}
}

void WiFiManager::Start(AsyncWebServer *server, custom_config *custom_opt, int custom_opt_length, uint32_t timeoutAP)
{
	StartLittleFS();
	webServer = server;
	_custom_opt = custom_opt;
	_custom_opt_length = custom_opt_length;
	this->_timeout_ap = timeoutAP;
	ws.onEvent([&](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
				   void *arg, uint8_t *data, size_t len)
			   { this->WebSocketEvent(server, client, type, arg, data, len); });

	webServer->addHandler(&ws);

	// httpUpdater.setup(&webServer, "/update", (char *)currentConfig.wwwuser, (char *)currentConfig.wwwpass);

	DEBUG_WIFI_MANAGERLN("WebSocket server started.");

	IPAddress IPap;
	IPap.fromString((char *)currentConfig.ipAP);

	DEBUG_WIFI_MANAGERLN("DNS Redirect started.");
	webServer->on("/generate_204", HTTP_ANY, [](AsyncWebServerRequest *request)
				  { request->redirect("http://192.168.4.1/"); });

	webServer->onNotFound([&](AsyncWebServerRequest *request) { // Setup file browser LittleFS
		if (!this->handleFileRead(request))
		{
			request->send(404, "text/plain", "404: File Not Found");
		}
	});

	// TODO Remove
	webServer->on("/reset", HTTP_GET, [&](AsyncWebServerRequest *request)
				  { request->send(200, "application/json", String(LittleFS.remove("/config.conf"))); });

	DEBUG_WIFI_MANAGERLN("Web server started.");
	StartWIFI();
	// StartSoftAP();
	dnsServer.start(53, "*", IPAddress(192, 168, 4, 1)); // with "*" to  redirect all to device IP;
}

void WiFiManager::loop()
{
	// ws.cleanupClients();
	dnsServer.processNextRequest();

	if (tryConnect)
	{
		response = "{ \"wifiConnect\":{ \"success\":";
		if (WiFi.status() == WL_CONNECTED)
		{
			response += "true,\"IP\":\"" + WiFi.localIP().toString() + "\"";
			response += "}}";
			ws.textAll(response);
			Serial.println("Resposta : " + response);

			Serial.print(WiFi.localIP());
			Serial.println();

			tryConnect = false;
		}
		else if (WiFi.status() == WL_CONNECT_FAILED)
			Serial.println("Fail to connect");
	}
}

void WiFiManager::offWiFi()
{
	WiFi.disconnect(true);
	WiFi.mode(WIFI_OFF);
}

void WiFiManager::onWiFi(bool startAP)
{
	if (startAP)
	{
		this->StartWIFI();
	}
	else
	{
		WiFi.begin();
	}
}