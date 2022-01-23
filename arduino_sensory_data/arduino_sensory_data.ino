#include <ArduinoJson.h>
#include <Wire.h>
#include <BH1750.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>
#include "GravityTDS.h"

#define ONE_WIRE_BUS 5
#define pHSensorPin A0
#define Offset 0.29 // pH deviation compensate
#define TDSSensorPin A1
#define TSSSensorPin A2

GravityTDS gravityTds;
BH1750 lightMeter;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

String message = "";
bool messageReady = false;

void setup() {
  Serial.begin(9600);
  
  // BH1750
  // Initialize the I2C bus (BH1750 library doesn't do this automatically)
  // On esp8266 devices you can select SCL and SDA pins using Wire.begin(D4, D3);
  Wire.begin();
  lightMeter.begin();

  // DS18B20
  sensors.begin();

  // SEN0244
  gravityTds.setPin(TDSSensorPin);
  gravityTds.setAref(5.0);  //reference voltage on ADC, default 5.0V on Arduino UNO
  gravityTds.setAdcRange(1024);  //1024 for 10bit ADC; 4096 for 12bit ADC
  gravityTds.begin();
}

void loop() {
  // Wait for message
  while(Serial.available()) {
    message = Serial.readString();
    messageReady = true;
  }
  if(messageReady) {
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc,message);
    if(error) {
      Serial.print(F("Arduino: deserializeJson() failed: "));
      Serial.println(error.c_str());
      messageReady = false;
      return;
    }
    if(doc["type"] == "request") {
      doc["type"] = "response";
      
      // DS18B20
      sensors.requestTemperatures();
      float temperature = sensors.getTempCByIndex(0);
      float temperatureTDS = 15;
      if (temperature > 0)
        temperatureTDS = temperature;
        
      // BH1750
      float lux = lightMeter.readLightLevel();

      // SEN0161
      float pH = 3.5*analogRead(pHSensorPin)*5.0/1024+Offset;

      // SEN0189
      float tssVoltage = analogRead(TSSSensorPin) * (5.0 / 1024.0); // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
      float NTU;
      if (tssVoltage < 2.5)
        NTU = 3000;
      else if (tssVoltage > 4.2)
        NTU = 0;
      else
        NTU = -1120.4 * tssVoltage * tssVoltage + 5742.3 * tssVoltage - 4352.9;

      // SEN0244
      gravityTds.setTemperature(temperatureTDS);  // set the temperature and execute temperature compensation
      gravityTds.update();  //sample and calculate 
      float TDS = gravityTds.getTdsValue();
      
      doc["temp"] = temperature;
      doc["light"] = lux;
      doc["ph"] = pH;
      doc["tss"] = NTU;
      doc["tds"] = TDS;
      serializeJson(doc,Serial);
    }
    messageReady = false;
  }
}
