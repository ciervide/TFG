#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#ifndef STASSID
#define STASSID "MIWIFI_CEaF"
#define STAPSK  "YQKC3FqR"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

const char* host = "192.168.1.196";
const int port = 3000;

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char fingerprint[] PROGMEM = "B9 D2 AA 9A 81 5E 9A 47 2C 62 8E 2F 5E 5D EF B1 30 FC 9E 25";

void setup() {

  Serial.begin(115200);

  Serial.print("\nConnecting to "); Serial.print(ssid);
  WiFi.mode(WIFI_STA); WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWiFi connected to IP address: "); Serial.println(WiFi.localIP());

  // WiFiClientSecure client; // Use WiFiClientSecure class to create TLS connection
  WiFiClient client;
  Serial.print("Connecting to "); Serial.println(host);

  /*Serial.printf("Using fingerprint '%s'\n", fingerprint);
  client.setFingerprint(fingerprint);*/

  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    return;
  }

  String url = "/api/measures";
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");
}

void loop() {
}z
