// ADC CHANNEL ESP32
// ADC2 channel 0 is GPIO4 (ESP32) = ADC2_CHANNEL_0
// ADC2 channel 1 is GPIO0 (ESP32) = ADC2_CHANNEL_1
// ADC2 channel 2 is GPIO2 (ESP32) = ADC2_CHANNEL_2
// ADC2 channel 3 is GPIO15 (ESP32) = ADC2_CHANNEL_3
// ADC2 channel 4 is GPIO13 (ESP32) = ADC2_CHANNEL_4
// ADC2 channel 5 is GPIO12 (ESP32) = ADC2_CHANNEL_5
// ADC2 channel 6 is GPIO14 (ESP32) = ADC2_CHANNEL_6
// ADC2 channel 7 is GPIO27 (ESP32) = ADC2_CHANNEL_7
// ADC2 channel 8 is GPIO25 (ESP32) = ADC2_CHANNEL_8
// ADC2 channel 9 is GPIO26 (ESP32) = ADC2_CHANNEL_9

// 0 dB attenuation gives full-scale voltage 1.1 V = ADC_ATTEN_0db
// 2.5 dB attenuation gives full-scale voltage 1.5 V = ADC_ATTEN_2_5db
// 6 dB attenuation gives full-scale voltage 2.2 V = ADC_ATTEN_6db
// 11 dB attenuation gives full-scale voltage 3.9 V = ADC_ATTEN_11db

#include "EmonLib.h"                   // Include Emon Library
#include <driver/adc.h>
EnergyMonitor emon1;                   // Create an instance
EnergyMonitor emon2;                   // Create an instance
EnergyMonitor emon3;                   // Create an instance

double Irms1;
double Irms2;
double Irms3;

double Vrms1;
double Vrms2;
double Vrms3;

//protótipos
void startEnergyMonitor();
void calculateEnergyMonitor();

void startEnergyMonitor(){
  adc2_config_channel_atten(ADC2_CHANNEL_3, ADC_ATTEN_0db);
  adc2_config_channel_atten(ADC2_CHANNEL_4, ADC_ATTEN_0db);
  adc2_config_channel_atten(ADC2_CHANNEL_5, ADC_ATTEN_0db);
  // adc2_config_channel_atten(ADC2_CHANNEL_6, ADC_ATTEN_11db); 
  // adc2_config_channel_atten(ADC2_CHANNEL_2, ADC_ATTEN_11db); 
  // adc2_config_channel_atten(ADC2_CHANNEL_0, ADC_ATTEN_11db);

  emon1.current(15, 115);             // Current: input pin, calibration.
  emon1.voltage(14, 448, 1.7);  // Voltage: input pin, calibration, phase_shift
  emon2.current(13, 123);             // Current: input pin, calibration.
  emon2.voltage(2, 356, 1.7);  // Voltage: input pin, calibration, phase_shift
  emon3.current(12, 123);             // Current: input pin, calibration.
  emon3.voltage(0, 532, 1.7);  // Voltage: input pin, calibration, phase_shift
}

void calculateEnergyMonitor(){
  emon1.calcVI(20,2000);
  emon2.calcVI(20,2000);
  emon3.calcVI(20,2000);

  Irms1 = emon1.Irms;  // Calculate Irms only
  Irms2 = emon2.Irms;  // Calculate Irms only
  Irms3 = emon3.Irms;  // Calculate Irms only

  Irms1= Irms1 -3.4;
  if(Irms1<0){
    Irms1 = 0;
  }
  Irms2= Irms2 -3.4;
  if(Irms2<0){
    Irms2 = 0;
  }
  Irms3= Irms3 -3.4;
  if(Irms3<0){
    Irms3 = 0;
  }

  Vrms1 = emon1.Vrms;  // Calculate Irms only
  Vrms2 = emon2.Vrms;  // Calculate Irms only
  Vrms3 = emon3.Vrms;  // Calculate Irms only
  
  if(Vrms1<3){
    Vrms1 = 0;
  }
  if(Vrms2<3){
    Vrms2 = 0;
  }
  if(Vrms3<3){
    Vrms3 = 0;
  }
}
