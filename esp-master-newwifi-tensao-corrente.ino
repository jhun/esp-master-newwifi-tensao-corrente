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

esp_reset_reason_t BootReason;
String motivoReboot;
String horaReboot;

WiFiManager wifi_manager;

void setup()
{
    Serial.begin(115200);
    startDisplay();
    BootReason = esp_reset_reason();
    WiFi.begin("MR.ROBOT","Senh@1q2w3e");
    startConnection();
    startEnergyMonitor();

    struct tm timeinfo;
    getLocalTime(&timeinfo);
    char datetimeReboot[30];
    size_t s = strftime(datetimeReboot, sizeof(datetimeReboot), "%Y-%m-%dT%H:%M:%S", &timeinfo);
    horaReboot= String(datetimeReboot);
    Serial.print(horaReboot+": ");
    
    switch (BootReason) {
        case ESP_RST_UNKNOWN:
          motivoReboot = "Reset reason can not be determined";
          Serial.println(motivoReboot);
        break;

        case ESP_RST_POWERON:

          motivoReboot = "Reset due to power-on event";
          Serial.println(motivoReboot);
        break;

        case ESP_RST_EXT:
          motivoReboot = "Reset by external pin (not applicable for ESP32)";
          Serial.println(motivoReboot);
        break;

        case ESP_RST_SW:
          motivoReboot = "Software reset via esp_restart";
          Serial.println(motivoReboot);
        break;

        case ESP_RST_PANIC:
          motivoReboot = "Software reset due to exception/panic";
          Serial.println(motivoReboot);
        break;

        case ESP_RST_INT_WDT:
          motivoReboot = "Reset (software or hardware) due to interrupt watchdog";
          Serial.println(motivoReboot);
        break;

        case ESP_RST_TASK_WDT:
          motivoReboot = "Reset due to task watchdog";
          Serial.println(motivoReboot);
        break;

        case ESP_RST_WDT:
          motivoReboot = "Reset due to other watchdogs";
          Serial.println(motivoReboot);
        break;                                

        case ESP_RST_DEEPSLEEP:
          motivoReboot = "Reset after exiting deep sleep mode";
          Serial.println(motivoReboot);
        break;

        case ESP_RST_BROWNOUT:
          motivoReboot = "Brownout reset (software or hardware)";
          Serial.println(motivoReboot);
        break;
        
        case ESP_RST_SDIO:
          motivoReboot = "Reset over SDIO";
          Serial.println(motivoReboot);
        break;
        
        default:
        break;
    }

   

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
