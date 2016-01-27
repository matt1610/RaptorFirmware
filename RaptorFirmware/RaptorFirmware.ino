/* Create a WiFi access point and provide a web server on it. */

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include "EEPROMAnything.h"

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti WiFiMulti;


/* Set these to your desired credentials. */
const char *ssid = "MattRaptCam";
const char *password = "";
bool pushed = false;

struct config_t
{
    int mode;
    char rid[32];
    char ssid[32];
    char pass[64];
} configuration;

ESP8266WebServer server(80);

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */

void httpResponse(String message) {
  server.sendHeader("Connection", "close");
//  server.sendHeader("Access-Control-Allow-Origin", "0.0.0.0");
  server.send(200, "application/json", "{\"success\" : \"true\", \"message\" : \""+message+"\"}");
}

void handleRoot() {
	httpResponse("Raptor is on");
}

void yellow() {
  digitalWrite(D1, LOW);    
  digitalWrite(D2, LOW);
  digitalWrite(D7, HIGH);
  httpResponse("yellow");
  delay(1000);
}

void red() {
  digitalWrite(D1, LOW);  
  digitalWrite(D2, HIGH);
  digitalWrite(D7, LOW);
  httpResponse("red");
  delay(1000);
}

void blue() {
  digitalWrite(D1, HIGH);    
  digitalWrite(D2, LOW);
  digitalWrite(D7, LOW);
  httpResponse("blue");
  delay(1000);
}

void getId() {
//  httpResponse(ESP.getChipId());
}

void writeToDisk(String rid, String wifiNet, String wifiPass, int mode) {

  EEPROM.begin(512);
  delay(100);

  strncpy(configuration.rid, rid.c_str(), 32);
  strncpy(configuration.ssid, wifiNet.c_str(), 32);
  strncpy(configuration.pass, wifiPass.c_str(), 64);
  configuration.mode = mode;

  EEPROM_writeAnything(0, configuration);

  Serial.println("Object Values");
  Serial.println(configuration.rid);
  Serial.println(configuration.ssid);
  Serial.println(configuration.pass);

  EEPROM.end();
  httpResponse("ID written to Device");
  Serial.println("Data written to disk");
}

void readFromDisk() {
  Serial.println("Reading");
  EEPROM_readAnything(0, configuration);

  Serial.print("Val: ");
  Serial.println(configuration.ssid);
  
  httpResponse(configuration.ssid);
}

void setup() {

  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D3, INPUT);
  
	delay(1000);
	Serial.begin(115200);

  EEPROM_readAnything(0, configuration);

  if (configuration.mode == NULL) {
    configuration.mode = 0; // Setup Mode
  }

  if (configuration.mode < 1) {
      Serial.println(configuration.ssid);
      Serial.print("Configuring access point...");
      WiFi.softAP(ssid, password);
    
      IPAddress myIP = WiFi.softAPIP();
      Serial.print("AP IP address: ");
      Serial.println(myIP);
      server.on("/", handleRoot);
      server.on("/red", red);
      server.on("/yellow", yellow);
      server.on("/blue", blue);
      server.on("/getId", getId);
      
      server.on("/writeId", []() {
        String rId = server.arg("id");
        String wifiNet = server.arg("wifi");
        String wifiPass = server.arg("wipass");
        String mode = server.arg("mode");
        writeToDisk(rId, wifiNet, wifiPass, mode.toInt());
      });
      
      server.on("/readId", readFromDisk);
      server.begin();
      Serial.println("HTTP server started");
      Serial.println(ESP.getChipId());
  } else {
      
      Serial.println("Camera Mode");

      Serial.print("Connecting to ");
      WiFi.begin(configuration.ssid, configuration.pass);

      for(uint8_t t = 4; t > 0; t--) {
          Serial.printf("[SETUP] WAIT %d...\n", t);
          Serial.flush();
          delay(1000);
      }

      Serial.print("WTF?: ");
      Serial.println(configuration.ssid);
      Serial.println(configuration.pass);
//      Serial.println(WiFiMulti.addAP(configuration.ssid, configuration.pass));
  }
    
  

  

//  delay(1000);
//  writeToDisk("sdfsdf", "wiNet", "wiPass");
//  writeToDisk(3, 4, 5);
//  delay(1000);
//  readFromDisk();
}

void loop() {

  int buttonState = digitalRead(D3);

  if (buttonState == 0 && !pushed) {
      pushed = true;
      Serial.println("Push");
      delay(3000);
      if (buttonState == 0) {
        Serial.println("Change config");
        configuration.mode = 0;
        EEPROM_writeAnything(0, configuration);
        delay(10);
      }
  }

  if (buttonState == 1) {
    pushed = false;  
  }

  if (configuration.mode < 1) {
     server.handleClient();
  } else {
    
    Serial.println("Camera Mode");
     if((WiFiMulti.run() == WL_CONNECTED)) {

        HTTPClient http;
  
        Serial.print("[HTTP] begin...\n");
        // configure traged server and url
        //http.begin("192.168.1.12", 443, "/test.html", true, "7a 9c f4 db 40 d3 62 5a 6e 21 bc 5c cc 66 c8 3e a1 45 59 38"); //HTTPS
        http.begin("robocontroller.azurewebsites.net", 80, "/set?message=test"); //HTTP
  
        Serial.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();
        if(httpCode) {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);
  
            // file found at server
            if(httpCode == 200) {
                String payload = http.getString();
                Serial.println(payload);
            }
        } else {
            Serial.print("[HTTP] GET... failed, no connection or no HTTP server\n");
        }
    }
    delay(10000);
  }

  



  
}
