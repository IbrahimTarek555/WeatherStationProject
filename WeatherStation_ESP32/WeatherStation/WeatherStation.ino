#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"

#include <LiquidCrystal.h>
#include "DHT.h"
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include "Adafruit_VEML6070.h"

/*Liquid Crystal*/
const int rs = 15, en = 2, d4 = 5, d5 = 18, d6 = 19, d7 = 4;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

/*DHT Sensor*/
#define DHTTYPE DHT11
const int DHTPin = 13;
DHT dht(DHTPin, DHTTYPE);
float DHTTemperature;
float Humidity;

/*MQ135 Sensor*/
int MQ135Data;

/*Rain Sensor*/
#define RainAnalog 35
#define RainDigital 34
int RainAnalogVal;
int RainDigitalVal;

/*BMP Sensor*/
Adafruit_BMP085 bmp;
float BMPTemperature;
float Altitude;
int Pressure;

/*VEML Sensor*/
Adafruit_VEML6070 uv = Adafruit_VEML6070();
int UVLevel;

/*Temperature*/
float Temperature;

void setup()
{
  /*LCD*/
  lcd.begin(20, 4);
  
  /*DHT Sensor*/
  dht.begin();

  /*Rain Sensor*/
  pinMode(RainDigital,INPUT);
  pinMode(RainAnalog,INPUT);

  /*BMP Sensor*/
  if (!bmp.begin()) {
	while (1) {}
  }

  /*VEML Sensor*/
  uv.begin(VEML6070_1_T);
}

void loop()
{
  /*DHT Sensor*/
  DHTTemperature = dht.readTemperature();   //Float Number
  Humidity = dht.readHumidity();  //Percenatage Float Number
  

  /*MQ135 Sensor*/
  /*
   * Less than 400 PPM: Normal background concentration in outdoor ambient Air.
   * From 400 to 1000 PPM: Typical in occupied indoor spaces with good air exchange.
   * More than 1000 PPM: Poor air.    
   */
  MQ135Data = analogRead(A0);   //Integer Number
  
  /*Rain Sensor*/
  RainAnalogVal = analogRead(RainAnalog);   //
  RainDigitalVal = digitalRead(RainDigital);  //0 or 1 Value, 0 for rainy and 1 for dry
  
  /*BMP Sensor*/
  BMPTemperature = bmp.readTemperature();   //Float Number
  Pressure = bmp.readPressure();        //Integer Number
  Altitude = bmp.readAltitude(102000);  //Float Number
  
  /*VEML Sensor*/
  UVLevel = uv.readUV();  //Integer Number
  
  /*Temperature*/
  Temperature = (DHTTemperature + BMPTemperature) / 2;  //Float Number
  
  /*LCD - LCD - LCD - LCD - LCD - LCD - LCD*/
  /*Temperature*/
  lcd.setCursor(0, 0);
  lcd.print("Temp. = ");
  lcd.print(Temperature);
  lcd.print(" *C");
  /*BMP Sensor*/
  lcd.setCursor(0, 1);
  lcd.print("Pressure = ");
  lcd.print(Pressure);
  lcd.print(" Pa");
  lcd.setCursor(0, 2);
  lcd.print("Altitude = ");
  lcd.print(Altitude);
  lcd.print(" m");
  /*VEML Sensor*/
  lcd.setCursor(0, 3);
  lcd.print("UV light level: ");
  lcd.print(UVLevel);
  
  delay(2000);
  lcd.clear();

  /*DHT Sensor*/
  lcd.setCursor(0, 0);
  lcd.print("Humidity = ");
  lcd.print(Humidity);
  lcd.print(" %");
  /*MQ135 Sensor*/
  lcd.setCursor(0, 1);
  lcd.print("AC = ");
  lcd.print(MQ135Data);
  lcd.print(" PPM");
  /*Rain Sensor*/
  lcd.setCursor(0, 2);
  lcd.print("Rain Status: ");
  if(RainDigitalVal == 1)
  {
    lcd.print("Dry");
  }
  else
  {
    lcd.print("Rainy");
  }
  lcd.setCursor(0, 3);
  lcd.print("Water LVL: ");
  lcd.print(RainAnalogVal);
  
  delay(2000);
  lcd.clear();
}