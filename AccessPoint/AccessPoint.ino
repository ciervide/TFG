#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "MIWIFI_CEaF";
const char* password = "YQKC3FqR";
const char* nwHost = "192.168.1.196";
const int nwPort = 3000;
const String sysUrl = "/api/measures";
const char* outSSID = "NotYetNamedAccessPoint";
const char* outPass = "Admin1234";
IPAddress apHost;
const int apPort = 3001;

WiFiServer server(apPort);

void setup() {

  Serial.begin(115200);

  // Connect to LAN
Serial.print("\nConnecting to "); Serial.print(ssid);
  WiFi.mode(WIFI_STA); WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
Serial.print(".");
  }
Serial.print("\nWiFi connected to IP address: "); Serial.println(WiFi.localIP());

  // Create Access Point
  if (WiFi.softAP(outSSID, outPass)) {
    apHost = WiFi.softAPIP();
Serial.print("Access point created with IP "); Serial.println(apHost);
  } else {
Serial.println("Something went wrong...");
  }

  // Begin the service
  server.begin();

}

void loop() {

  // Attend clients
  WiFiClient apClient = server.available();
  if (apClient) {
    String request = "";
    while (apClient.connected()) {
      if (apClient.available()) {
        
        // Read request
        char c = apClient.read();
        if (c == '\n') {
          Serial.println("\n ----- Request received! ----- ");
          if (request.startsWith("GET")) {
            Serial.println(parseGET());
            sendRequestToSystem(parseGET());
          } else if (request.startsWith("POST")) {
            request = apClient.readStringUntil('\n');
            int i = 0, j = request.indexOf('}');
            while (j != -1) {
              sendRequestToSystem(parsePOST(request.substring(i, j+1)));
              i=j+2; j = request.indexOf('}', i);
            }
          }
          break;
        } else {
          request += c;
        }
               
      }
    }

    Serial.println("Closing connection with client");
    apClient.stop();
  }
}

String parseGET() {
  return String("GET ") + sysUrl + " HTTP/1.1\r\n" +
     "Host: " + nwHost + "\r\n" +
     "User-Agent: BuildFailureDetectorESP8266\r\n" +
     "Connection: close\r\n\r\n";
}

String parsePOST(String JSONContent) {
  return String("POST ") + sysUrl + " HTTP/1.1\r\n" +
     "Host: " + nwHost + "\r\n" +
     "Content-Type: application/json" + "\r\n" +
     "Content-Length: " + JSONContent.length() + "\r\n\r\n" +
     JSONContent + "\r\n" +
     "Connection: close\r\n\r\n";
}

void sendRequestToSystem(String request) {
  WiFiClient sysClient;
  if (!sysClient.connect(nwHost, nwPort)) {
    Serial.println("connection failed");
    return;
  }
  sysClient.print(request);
  while (sysClient.connected()) {
    String line = sysClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("Headers received");
      break;
    }
  }
  String line = sysClient.readStringUntil('\n');
  Serial.println("Response:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
}
