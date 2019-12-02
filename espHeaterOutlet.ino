/**
   BasicHTTPClient.ino

    Created on: 24.05.2015

*/

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PolledTimeout.h>

#include <TZ.h>
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>

ESP8266WiFiMulti WiFiMulti;

volatile float tempF = 50;
volatile int hour = 0;
volatile bool heaterOn = false;

static esp8266::polledTimeout::periodicMs showTimeNow(60000);

void deser(String payload) {
  DynamicJsonDocument doc(1024);
 
  // Parse JSON object
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  // Extract values
  //Serial.println(doc["main"]["temp"].as<float>(), 6);
  //Serial.println(doc["dt"].as<int32>(), 10);
  tempF = doc["main"]["temp"].as<float>();
  
  time_t now;
  unsigned int currentime=time(&now);
  struct tm *timeinfo = localtime(&now);
  //Serial.println(timeinfo->tm_hour);
  //Serial.println(localtime(nullptr)->tm_hour);

  if(timeinfo->tm_year > 70) { 
    hour = timeinfo->tm_hour;
  } else { 
    time_t rtc = doc["dt"].as<int32>(); 
    timeval tv = { rtc, 0 };
    hour = localtime(&rtc)->tm_hour;
  }
}

void showTime() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;

    //Serial.print("[HTTP] begin...\n");
    if (http.begin(client, "http://api.openweathermap.org/data/2.5/weather?zip=43077&appid=23f9d7b0ac23a2357cc5ee3c657654c3&units=imperial")) {  // HTTP

      //Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        //Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          //Serial.println(payload);

          deser(payload);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }
}

void decide() { 
  switch (hour) {
  case 2: 
  case 3: 
    if(tempF <= 10 && !heaterOn) {   
      digitalWrite(12, HIGH);   // turn relay on
      heaterOn = true;
    }
    break;
  case 4: 
  case 5: 
    if(tempF <= 20 && !heaterOn) {   
      digitalWrite(12, HIGH);   // turn relay on
      heaterOn = true;
    }
    break;
  case 6: 
  case 7: 
    if(tempF <= 30 && !heaterOn) {   
      digitalWrite(12, HIGH);   // turn relay on
      heaterOn = true;
    }
    break;
  case 8: 
  case 9: 
    if(tempF <= 40 && !heaterOn) {   
      digitalWrite(12, HIGH);   // turn relay on
      heaterOn = true;
    }
    break;
  default: 
    if(heaterOn) {   
      digitalWrite(12, LOW);   // turn relay off
      heaterOn = false;
    }
    break; 
  }

  
  Serial.print("temp:");
  Serial.print(tempF);
  Serial.print("F hour:"); 
  Serial.print(hour);
  Serial.print(" heaterOn:");
  Serial.println(heaterOn);
}


void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  pinMode(12, OUTPUT); //relay output

  setenv("TZ", "EST5EDT,M3.2.0/2,M11.1.0", 1);
  tzset();

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("SSID", "PASS");

  configTime(0, 0, "pool.ntp.org");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (showTimeNow) {
    showTime();
    decide();
  }
}
