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

#include <NTPClient.h>
#include <WiFiUdp.h>

/*Time Stamp*/
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

String formattedDate;
String dayStamp;
String timeStamp;
int splitT;

#define THINGNAME               "ESP32"
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

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

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);
 
void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID);  
 
  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
 
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
 
  // Create a message handler
  client.setCallback(messageHandler);
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
 
  Serial.println("AWS IoT Connected!");
}
 
void publishMessage()
{
  StaticJsonDocument<200> doc;
  doc["Temp"] = Temperature;
  doc["Pressure"] = Pressure;
  doc["Altitude"] = Altitude;
  doc["UV light level"] = UVLevel;
  doc["Humidity"] = Humidity;
  doc["AC"] = MQ135Data;  
  doc["Rain Status"] = RainDigitalVal;
  doc["Water LVL"] = RainAnalogVal; 
  doc["Time Stamp"] = timeStamp;
  doc["Date Stamp"] = dayStamp;

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}
 
void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("incoming: ");
  Serial.println(topic);
 
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.println(message);
}


void setup() {
  /*LCD*/
  lcd.begin(20, 4);  

  // serial 
  Serial.begin(115200);
  connectAWS();  

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

  /*Time Stamp*/
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(7200);

}

void loop() {
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
  
  /*Serial - Serial - Serial - Serial - Serial - Serial - Serial*/
  Serial.print(F("Temp. = "));
  Serial.println(Temperature);
  Serial.print(F("Pressure = "));
  Serial.println(Pressure);
  Serial.print("Altitude = ");
  Serial.println(Altitude);
  Serial.print("UV light level: ");
  Serial.println(UVLevel);
  Serial.print("Humidity = ");
  Serial.println(Humidity);
  Serial.print("AC = ");
  Serial.println(MQ135Data);
  Serial.print("Rain Status: ");
  if(RainDigitalVal == 1)
  {
    Serial.println("Dry");
  }
  else
  {
    Serial.println("Rainy");
  }  
  Serial.print("Water LVL: ");
  Serial.println(RainAnalogVal);  

  publishMessage();
  client.loop();

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
  
  /*Time Stamp*/
  while(!timeClient.update()) {
   timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
  // Extract date
  splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  lcd.setCursor(0, 1);
  lcd.print("DATE: ");
  lcd.print(dayStamp);
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  lcd.setCursor(0, 2);
  lcd.print("HOUR: ");
  lcd.print(timeStamp);
  delay(2000);

}