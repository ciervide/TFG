#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

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

  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Content-Type: application/json" + "\r\n" +
               "Content-Length: 133" + "\r\n\r\n" +
               "{\"id\":\"T00010003\",\"id_trip\":\"0001\",\"id_driv\":\"0001\",\"dist_act\":\"37.5\",\"coord_lat\":\"43.6\",\"coord_lon\":\"-1.71\",\"spd\":\"12\",\"t\":\"103421\"}" + "\r\n" +
               "Connection: close\r\n\r\n");

  /*client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Content-Type: application/json\r\n" +
               "{\"id\":\"T00010002\",\"id_trip\":\"0001\",\"id_driv\":\"0001\",\"dist_act\":\"25\",\"coord_lat\":\"42.6\",\"coord_lon\":\"-1.70\",\"spd\":\"10\",\"t\":\"102956\"}" +
               "\r\n\r\n");*/

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
}
