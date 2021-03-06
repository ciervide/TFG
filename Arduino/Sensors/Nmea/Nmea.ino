#include <SoftwareSerial.h>
#include <Gaussian.h>

#define rxPin 4
#define txPin 5

SoftwareSerial gps = SoftwareSerial(rxPin, txPin);
String str = "";
String GLL = "", VTG = "";
float f_lat = 180, f_lon = 180, dist = 0, max_dist = 0, num_measures = 0;
float EARTH_RADIUS = 6371e3;

Gaussian gen;
float MEAN = 0.04, VAR = 0.03;

unsigned long previousMillis = 0;

void setup() {
  // Setup the neccesary items to use the GPS antenna
  pinMode(rxPin, INPUT); pinMode(txPin, OUTPUT);
  gps.begin(9600); gps.flush();
  Serial.begin(9600);
  if (gps.isListening()) {
    Serial.print("Reading serial on pin "); Serial.println(rxPin);
  }

  // Initialize the normal distribution number generator
  gen = Gaussian(MEAN, VAR);
}

void loop() {

  char inc;
  if (gps.available()) {
    inc = gps.read();
    if (inc == '$') {
      if (str.indexOf("$GNGLL") >= 0)
        GLL = str;
      else if (str.indexOf("$GNVTG") >= 0)
        VTG = str;
      if ((GLL != "") && (VTG != ""))
        extractFromNMEA();
      str = "$";
    } else {
      str += inc;
    }
    
  }
  
}

// Compute the distance between 2 points using Haversine formula (https://en.wikipedia.org/wiki/Haversine_formula)
float getTravelledDistance(float aux_lat, float aux_lon) {
  // Computation of the error based on a normal distribution
  float err = gen.random(); // Error
  if (err < 0) err = -err;

  // Computation of the distance using the Haversine formula
  float fi = toRadians(aux_lat - f_lat);
  float lambda = toRadians(aux_lon - f_lon);
  float a = pow(sin(fi/2),2) + cos(toRadians(f_lat)) * cos(toRadians(aux_lat)) * pow(sin(lambda/2),2);
  float d = 2 * EARTH_RADIUS * asin(sqrt(a));

  // Printings to debug the code
  Serial.print("\nDistance between (");
  Serial.print(f_lat,8); Serial.print(", "); Serial.print(f_lon,8); Serial.print(") & (");
  Serial.print(aux_lat,8); Serial.print(", "); Serial.print(aux_lon,8); Serial.print("): ");
  Serial.print(d, 8); Serial.print("-"); Serial.print(err); Serial.print(" = ");
  d -= err; Serial.print(d, 8); Serial.println("m");

  // Uncomment when deleting the printings
  // d -= err; 
  return d;
}

// Computes the given degrees angle in radians
float toRadians(float degr) {
  return degr * PI/180;
}

// Parses the GNSS coordinates given by the antenna (hhmm.mmmm) into hours
float parseGNSS(float coord) {
  float aux;
  aux = trunc(coord/100) + (coord - trunc(coord/100)*100)/60;
  return aux;
}

// Prints the content of a char array
void printCharArr(char arr[]) {
  int len = strlen(arr);
  for (int i = 0; i < len; i++) {
    Serial.print(arr[i]);
  }
}

// Function that extract the desired data from the NMEA strings
void extractFromNMEA() {
  // Needed variables to extract the coordinates
  char arrGLL[GLL.length()];
  GLL.toCharArray(arrGLL, GLL.length() + 1);
  float aux_lat, aux_lon;

  // Read coordinates
  char *token; token = strtok(arrGLL, ",");
  token = strtok(NULL, ","); aux_lat = parseGNSS(atof(token)); token = strtok(NULL, ",");
  if (token[0] == 'S') aux_lat = -aux_lat; // If latitude is on the South, convert it to negative
  if ((aux_lat < -90) || (aux_lat > 90)) aux_lat = 0;
  token = strtok(NULL, ","); aux_lon = parseGNSS(atof(token)); token = strtok(NULL, ",");
  if (token[0] == 'W') aux_lon = -aux_lon; // If longitude is on the West, convert it to negative
  if ((aux_lon < -180) || (aux_lon > 180)) aux_lon = 0;
  
  // Compute distance between stored and new coordinates
  if ((f_lat == 180) && (f_lon == 180)) {
    if ((aux_lat != 0) && (aux_lon != 0)) {
      f_lat = aux_lat; f_lon = aux_lon;
      // Notify error
    }
  } else {
    dist += getTravelledDistance(aux_lat, aux_lon);
    f_lat = aux_lat; f_lon = aux_lon;
    Serial.print("Travelled distance: "); Serial.print(dist, 4); Serial.println("m");
  }

  // Needed variables to extract the speed
  char arrVTG[VTG.length()];
  VTG.toCharArray(arrVTG, VTG.length() + 1);

  // Read speed
  token = strtok(arrVTG, ","); token = strtok(NULL, ",");
  token = strtok(NULL, ","); token = strtok(NULL, ",");
  token = strtok(NULL, ","); token = strtok(NULL, ",");
  Serial.print("Actual speed: "); Serial.println(atof(token));

  // Clean GLL and VTG
  GLL = ""; VTG = "";
}
