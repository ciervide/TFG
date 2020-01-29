#include <ESP8266WiFi.h>

const char* ssid = "ZyXEL NBG-418N v2";
const char* password = "RJFTY69649";

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
