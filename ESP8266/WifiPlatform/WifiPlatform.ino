#include <ESP8266WiFi.h>

const char* inSSID = "MIWIFI_CEaF";
const char* inPass = "YQKC3FqR";
const char* outSSID = "NotYetNamedAccessPoint";
const char* outPass = "Admin1234";

void setup() {
  
  delay(1000);
  Serial.begin(115200);
 
  WiFi.begin(inSSID, inPass);
  Serial.println(); Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Success!");
  Serial.print("IP Address is: "); Serial.println(WiFi.localIP());

  Serial.println(); Serial.print("Configuring WiFi access point...");
  boolean result = WiFi.softAP(outSSID, outPass);
  if (result == true) {
    IPAddress myIP = WiFi.softAPIP();
    Serial.println("done!");
    Serial.println("");
    Serial.print("WiFi network name: "); Serial.println(outSSID);
    Serial.print("WiFi network password: "); Serial.println(outPass);
    Serial.print("Host IP Address: "); Serial.println(myIP);
    Serial.println("");
  } else {
    Serial.println("error! Something went wrong...");
  }
  
}

void loop() {
  Serial.printf("Number of connected devices (stations) = %d\n", WiFi.softAPgetStationNum());
  delay(3000);
}
