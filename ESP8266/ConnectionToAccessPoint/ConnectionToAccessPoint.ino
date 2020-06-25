#include <ESP8266WiFi.h>

const char* ssid = "MIWIFI_CEaF";
const char* password = "YQKC3FqR";

void setup() {
  delay(1000);
  Serial.begin(115200);
 
  WiFi.begin(ssid, password);

  Serial.println();
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("success!");
  Serial.print("IP Address is: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  
}
