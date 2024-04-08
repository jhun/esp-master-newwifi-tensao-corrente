#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <Preferences.h>
#include <HTTPClient.h>


#define len(arr) (sizeof(arr) / sizeof((arr)[0]))

#define NVS_RASP_IP_KEY "r_ip"
#define DEBUG 1

#define BOMBA_PIN LED_BUILTIN

AsyncWebServer webServer(80);
String IPraspberry;
Preferences preferences;
extern WiFiManager wifi_manager;

int64_t lastUpdate;

custom_config custom_opt[] = {{NVS_RASP_IP_KEY, "Raspberry IP", "192.168.0.1"}};

bool StateONOFF = false;

void startConnection();
void sendValues();

extern double Irms1;
extern double Irms2;
extern double Irms3;

extern double Vrms1;
extern double Vrms2;
extern double Vrms3;


void startConnection(){
  pinMode(BOMBA_PIN, OUTPUT);
  loadConfigs();
  webServer.on("/cmd", HTTP_POST, HandlerCmdFromRaspberry);
  webServer.on("/custom_cfg", HTTP_POST, SaveCustomCfg);
  wifi_manager.Start(&webServer, custom_opt, len(custom_opt),45000);
  webServer.begin();
  configTime(-3 * 3600, 0, "pool.ntp.org");
}

void loadConfigs()
{
    preferences.begin("nvs", true);
    IPraspberry = preferences.getString(NVS_RASP_IP_KEY, "");
    preferences.end();
    Serial.println("R:" + IPraspberry);
    custom_opt[0].value = IPraspberry;
}

void SaveCustomCfg(AsyncWebServerRequest *request)
{
    preferences.begin("nvs");
    for (size_t i = 0; i < len(custom_opt); i++)
    {
        if (request->hasParam(custom_opt[i].id, true))
        {
            String value = request->getParam(custom_opt[i].id, true)->value();
            preferences.putString(custom_opt[i].id.c_str(), value);
            request->send(200, "application/json", String("{\"success\":true}"));
        }
    }
    preferences.end();
    loadConfigs();
    request->send(200, "application/json", "{\"success\":true}");
    return;
}

void ResponseToRaspberry(int msg_id)
{
    String serverEndpoint = "http://" + IPraspberry + "/cmd/ack";
    HTTPClient http;
    http.begin(serverEndpoint.c_str());
    int statusCode = http.POST("{\"mac\":\"" + WiFi.macAddress() + "\",\"msg_id\":" + String(msg_id) + "}");
}

void HandlerCmdFromRaspberry(AsyncWebServerRequest *request)
{
    Serial.println("New CMD received");
    if (request->hasParam("msg_id", true))
    {
        if (request->hasParam("setor", true) && request->hasParam("prop", true) && request->hasParam("value", true))
        {
            String setor = request->getParam("setor", true)->value();
            String prop = request->getParam("prop", true)->value();
            String value = request->getParam("value", true)->value();
            if (setor.equals("bomba"))
            {
                if (prop.equals("on_off"))
                {
                    digitalWrite(BOMBA_PIN, value.equals("1"));
                }
            }
        }
        ResponseToRaspberry(request->getParam("msg_id", true)->value().toInt());
    }
    request->send(200, "application/json", "Atualizado");
    return;
}

String deviceStatus()
{

    int tensao = rand() % 500;
    int corrente = rand() % 100;
    int vazao = rand() % 20;
    int abertura = 38;
    String on_off = digitalRead(BOMBA_PIN) == HIGH ? "true" : "false";
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    char datetime[30];
    size_t s = strftime(datetime, sizeof(datetime), "%Y-%m-%dT%H:%M:%S", &timeinfo);
    String status = "{\"bomba\":{\"datetime\":\"" + String(datetime) +
                    "\",\"tensao1\":" + String(Vrms1) +
                    ",\"tensao2\":" + String(Vrms2) +
                    ",\"tensao3\":" + String(Vrms3) +
                    ",\"corrente1\":" + String(Irms1) +
                    ",\"corrente2\":" + String(Irms2) +
                    ",\"corrente3\":" + String(Irms3) +
                   "}}";
    return status;
}

void sendValues(){

  if ((esp_timer_get_time() / 1000ULL) - lastUpdate > 10000)
  {
    wifi_manager.onWiFi();
    lastUpdate = esp_timer_get_time() / 1000ULL;
    while(WiFi.status() != WL_CONNECTED){
      Serial.println("tentando reconectar para enviar status...");
      delay(100);
    }
    send_status();
  }
}

void send_status()
{
        StateONOFF = digitalRead(BOMBA_PIN) == HIGH;
        if (IPraspberry.isEmpty())
            return;
        if (!WiFi.isConnected())
        {
            Serial.println("Not connected");
            return;
        }
        Serial.print("Enviando status...");
        String serverEndpoint = "http://" + IPraspberry + "/device/update";
        Serial.print(serverEndpoint + "...");
        HTTPClient http;
        http.begin(serverEndpoint.c_str());
        int statusCode = http.POST("{\"mac\":\"" + WiFi.macAddress() + "\",\"ip\":\"" + WiFi.localIP().toString() + "\",\"status\":" + deviceStatus() + "}");
        if (statusCode == 200)
        {
            Serial.println("Atualizado no Raspberry");
        }
        else
        {
            Serial.println("Erro: " + String(statusCode));
        }
}