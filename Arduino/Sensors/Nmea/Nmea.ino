#include <SoftwareSerial.h>

#define rxPin 4
#define txPin 5

SoftwareSerial gps = SoftwareSerial(rxPin, txPin);
String str = "";
float f_lat = 180, f_lon = 180, dist = 0, max_dist = 0, num_measures = 0;
float EARTH_RADIUS = 6371e3;

unsigned long previousMillis = 0;

void setup() {
  
  pinMode(rxPin, INPUT); pinMode(txPin, OUTPUT);
  gps.begin(9600); gps.flush();
  Serial.begin(9600);
  
  if (gps.isListening()) {
    Serial.print("Reading serial on pin "); Serial.println(rxPin);
  }
}

void loop() {

  // Take into account that invalid coordinates are taken while the device connects with the satellites
 
  char inc;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 5000) {
    
    if (gps.available()) {
      inc = gps.read();
      if (inc == '$') {
        if (str.indexOf("$GNGLL") >= 0) {
          // Needed variables to extract the coordinates
          char arr[str.length()];
          str.toCharArray(arr, str.length() + 1);
          float aux_lat, aux_lon;
  
          // Read coordinates
          char *token; token = strtok(arr, ",");
          token = strtok(NULL, ","); aux_lat = parseGNSS(atof(token)); token = strtok(NULL, ",");
          if (token[0] == 'S') aux_lat = -aux_lat; // If latitude is on the South, convert it to negative
          token = strtok(NULL, ","); aux_lon = parseGNSS(atof(token)); token = strtok(NULL, ",");
          if (token[0] == 'W') aux_lon = -aux_lon; // If longitude is on the West, convert it to negative
          
          // Compute distance between stored and new coordinates
          if ((f_lat == 180) && (f_lon == 180)) {
            f_lat = aux_lat; f_lon = aux_lon;
          } else {
            dist += getTravelledDistance(aux_lat, aux_lon);
            f_lat = aux_lat; f_lon = aux_lon;
            Serial.print("Travelled distance: "); Serial.print(dist, 4); Serial.println("m");
          }
          
          // To increase sampling time
          //previousMillis = currentMillis;
          
        }
        str = "$";
      } else {
        str += inc;
      }
    }
    
  }
  
}

// Compute the distance between 2 points using Haversine formula (https://en.wikipedia.org/wiki/Haversine_formula)
float getTravelledDistance(float aux_lat, float aux_lon) {
  
  float fi = toRadians(aux_lat - f_lat);
  float lambda = toRadians(aux_lon - f_lon);
  float a = pow(sin(fi/2),2) + cos(toRadians(f_lat)) * cos(toRadians(aux_lat)) * pow(sin(lambda/2),2);
  float d = 2 * EARTH_RADIUS * asin(sqrt(a));
  Serial.print("\nDistance between (");
  Serial.print(f_lat,8); Serial.print(", "); Serial.print(f_lon,8); Serial.print(") & (");
  Serial.print(aux_lat,8); Serial.print(", "); Serial.print(aux_lon,8); Serial.print("): ");
  Serial.print(d, 8); Serial.print("-"); Serial.print(err); Serial.print(" = ");
  d -= err; Serial.print(d, 8); Serial.println("m");
  
  return d;
}

float toRadians(float degr) {
  return degr * PI/180;
}

float parseGNSS(float coord) {
  float aux;
  aux = trunc(coord/100) + (coord - trunc(coord/100)*100)/60;
  return aux;
}

void printCharArr(char arr[]) {
  int len = strlen(arr);
  for (int i = 0; i < len; i++) {
    Serial.print(arr[i]);
  }
}
