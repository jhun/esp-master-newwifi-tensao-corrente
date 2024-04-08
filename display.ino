#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//protótipos
void startDisplay();
void showDisplayData();
void showConnectionsData();

extern double Irms1;
extern double Irms2;
extern double Irms3;

extern double Vrms1;
extern double Vrms2;
extern double Vrms3;

extern String motivoReboot;
extern String horaReboot;



void startDisplay(){
  Wire.begin(5, 4);// Start I2C Communication SDA = 5 and SCL = 4 on Wemos Lolin32 ESP32 with built-in SSD1306 OLED

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();
}

float incremento = 0.00;
void showDisplayData(){
  incremento += 0.01;
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  // display.println("Corrente 1: " + String(Irms1) + "A");
  // display.setCursor(0, 10);
  // display.println("Corrente 2: " + String(Irms2) + "A");
  // display.setCursor(0, 20);
  // display.println("Corrente 3: " + String(Irms3) + "A");
  // display.setCursor(0, 30);
  // display.println("Tensao 1: " + String(Vrms1) + "V");
  // display.setCursor(0, 40);
  // display.println("Tensao 2: " + String(Vrms2) + "V");
  // display.setCursor(0, 50);
  // display.println("Tensao 3: " + String(Vrms3) + "V");
  display.setCursor(0, 0);
  display.println(horaReboot + ": ");
  display.setCursor(0, 10);
  display.println(motivoReboot);
  display.setCursor(0, 20);
  display.println("Tensao 1: " + String(Vrms1+incremento) + "V");
  display.setCursor(0, 30);
  display.println("Tensao 2: " + String(Vrms2) + "V");
  display.setCursor(0, 40);
  display.println("Tensao 3: " + String(Vrms3) + "V");

 
  display.display();
}


void showConnectionsData(String ssid,String password, String ipAP, String wwwuser, String wwwpass){
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("SSID: " + ssid);
  display.setCursor(0, 10);
  display.println("Password: " + password);
  display.setCursor(0, 20);
  display.println("IP AP: " + ipAP);
  display.setCursor(0, 30);
  display.println("WWW User: " + wwwuser);
  display.setCursor(0, 40);
  display.println("WWW password: " + wwwpass);

  display.display();
}
