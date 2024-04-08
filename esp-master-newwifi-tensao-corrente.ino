#include <Arduino.h>
#include <esp_wifi.h>
#include "WiFi_Manager.h"


extern void startConnection();
extern void sendValues();
extern void startEnergyMonitor();
extern void calculateEnergyMonitor();
extern void startDisplay();
extern void showDisplayData();
extern void showConnectionsData();

WiFiManager wifi_manager;

void setup()
{
    Serial.begin(115200);
    WiFi.begin("pedrabranca_automacao","@utoP3dra");
    startConnection();
    startEnergyMonitor();
    startDisplay();
    while(wifi_manager.apIsActive){
      showConnectionsData(String((char *)currentConfig.ssid),String((char *)currentConfig.password),String((char *)currentConfig.ipAP),String((char *)currentConfig.wwwuser),String((char *)currentConfig.wwwpass));
      delay(1000);
    }
}

void loop()
{
  wifi_manager.loop();
  wifi_manager.offWiFi();
  while(WiFi.status() == WL_CONNECTED){
    Serial.println("tentando desconectar para calcular sensores...");
    delay(100);
  }
  calculateEnergyMonitor();
  showDisplayData();
  sendValues();
}
