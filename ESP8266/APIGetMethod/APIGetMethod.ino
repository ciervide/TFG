#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "MIWIFI_CEaF";
const char* password = "YQKC3FqR";

void setup() 
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("CONNECTED!");
}

void loop() 
{
  if (WiFi.status() == WL_CONNECTED) 
  {
    HTTPClient http; //Object of class HTTPClient
    http.begin("192.168.1.143:3000/api/measures");
    int httpCode = http.GET();

    if (httpCode > 0) 
    {
      //const size_t bufferSize = JSON_OBJECT_SIZE(6) + 370;
      //DynamicJsonBuffer jsonBuffer(bufferSize);

      Serial.print("Response:");
      Serial.println(http.getString());
      /*
      JsonObject& root = jsonBuffer.parseObject(http.getString());
 
      int id = root["id"]; 
      const char* name = root["name"]; 
      const char* username = root["username"]; 
      const char* email = root["email"]; 

      Serial.print("Name:");
      Serial.println(name);
      Serial.print("Username:");
      Serial.println(username);
      Serial.print("Email:");
      Serial.println(email);*/
    }
    http.end(); //Close connection
  }
  delay(60000);
}
