#include <SoftwareSerial.h>

#define rxPin 4
#define txPin 5

SoftwareSerial gps = SoftwareSerial(rxPin, txPin);
String str = "";

void setup() {
  
  pinMode(rxPin, INPUT); pinMode(txPin, OUTPUT);
  gps.begin(9600); gps.flush();
  Serial.begin(9600);

  if (gps.isListening()) {
    Serial.print("Reading serial on pin "); Serial.println(rxPin);
  }
}

void loop() {
  char inc;

  if (gps.available()) {
    inc = gps.read();
    if (inc == '$') {
      if (str.indexOf("$GNGLL") >= 0) {
        char arr[str.length()];
        str.toCharArray(arr, str.length() + 1);
        //            printCharArr(arr);
        //            Serial.println("");

//---------------------------------------------------------------------------
        char *token;
        token = strtok(arr, ",");

        while (token != NULL) {
          Serial.print("    ");
          printCharArr(token);
          Serial.println("");
          token = strtok(NULL, ",");
        }
        
//---------------------------------------------------------------------------
      }
      str = "$";
    } else {
      str += inc;
    }
  }
}

void printCharArr(char arr[]) {
  int len = strlen(arr);
  for (int i = 0; i < len; i++) {
    Serial.print(arr[i]);
  }
}
