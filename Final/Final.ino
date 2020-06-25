#include <Adafruit_PN532.h>
#include <ArduinoJson.h>
#include <Gaussian.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Wire.h>
#define PN532_IRQ_PIN   2 // NFC Shield
#define PN532_RESET_PIN 3 // NFC Shield
#define NMEA_RX_PIN     4 // NMEA
#define NMEA_TX_PIN     5 // NMEA
#define SOUND_PIN       8 // Buzzer
#define MODE_CHANGE_PIN 9 // Button
#define SD_PIN         10 // SD module

// Variables of general purpose
int actualMode = 0;

// Variables and constants for NFC (Mifare Classic NFC Standard)
Adafruit_PN532 nfc(PN532_IRQ_PIN, PN532_RESET_PIN);
const int FIRST_UTIL_BLOCK = 4;
const int TOTAL_UTIL_SECTORS = 15;
const int UTIL_BLOCKS_PER_SECTOR = 3;
const int BLOCK_SIZE = 16;

// Variables por SD files read/write
const String TRAVEL_FILE = "travel.txt";
const String MEASURES_FILE = "measures.txt";
const String ODOMETER_FILE = "odometer.txt";

// Variables and constants for NMEA
const float EARTH_RADIUS = 6371e3;
const float MEAN = 0.04;
const float VAR = 0.03;
SoftwareSerial gps = SoftwareSerial(NMEA_RX_PIN, NMEA_TX_PIN);
Gaussian gen;
String str = "";
String GLL = "", VTG = "";

// Variables to store the measures
int measure_num;
String id_trip;
String id_driv;
float dist_act;
float coord_lat = 180;
float coord_lon = 180;
float spd;
float t;

void setup() {
  Serial.begin(9600);
  // Setting variables and pins
  pinMode(NMEA_RX_PIN, INPUT);
  pinMode(NMEA_TX_PIN, OUTPUT);
  pinMode(MODE_CHANGE_PIN, INPUT);
  pinMode(SD_PIN, OUTPUT);
  SD.begin(SD_PIN);
  gps.begin(9600);
  gps.flush();
  gen = Gaussian(MEAN, VAR);
  measure_num = 1;
  if (SD.exists(ODOMETER_FILE))
    dist_act = readFromSD(ODOMETER_FILE).toFloat();
  else
    dist_act = readFromSD(ODOMETER_FILE + ".bac").toFloat();

  // Setting NFC and reading driver code (if time pass without reading, sound an alarm)
  setupNFC();
  id_driv = readCard(10*1000); // 10 seconds
  while(id_driv == "-1") {
    sound(4);
    id_driv = readCard(250);
  }
  sound(3);
  
  // Reading travel number from SD, updating it and storing on a local variable
  int trip = readFromSD(TRAVEL_FILE).toInt()+1;
  writeOnSD(TRAVEL_FILE, String(trip), true);
  id_trip = parseNumber(trip, 4);
  
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
      str = "$";
    } else {
      str += inc;
    }
    if ((GLL != "") && (VTG != "")) {
      extractFromNMEA();
    }
  }
  
}

/**
 * Function that makes the buzzer sound
 * The "status" parameter indicates the sound to play according to the following list:
 *  - 0: First mode of the device
 *  - 1: Second mode of the device
 *  - 2: Third mode of the device
 *  - 3: Operation is correct
 *  - 4: Error: No user
 *  - 5: Parking sound: long range
 *  - 6: Parking sound: medium range
 *  - 7: Parking sound: short range
 */
void sound(int status) {
  int notes[] = {262, 294, 330, 349, 392, 440, 494, 523};
  switch (status) {
    case 0: // First mode of the device
      tone(SOUND_PIN, notes[0]);
      delay(500);
      tone(SOUND_PIN, notes[0]);
      delay(500);
      tone(SOUND_PIN, notes[0]);
      delay(500);
      noTone(SOUND_PIN);
      break;
    case 1: // Second mode of the device
      tone(SOUND_PIN, notes[0]);
      delay(500);
      tone(SOUND_PIN, notes[2]);
      delay(500);
      tone(SOUND_PIN, notes[2]);
      delay(500);
      noTone(SOUND_PIN);
      break;
    case 2: // Third mode of the device
      tone(SOUND_PIN, notes[0]);
      delay(500);
      tone(SOUND_PIN, notes[2]);
      delay(500);
      tone(SOUND_PIN, notes[4]);
      delay(500);
      noTone(SOUND_PIN);
      break;
    case 3: // Operation is correct
      tone(SOUND_PIN, notes[0]);
      delay(100);
      tone(SOUND_PIN, notes[7]);
      delay(100);
      noTone(SOUND_PIN);
      break;
    case 4: // Error: No user
      tone(SOUND_PIN, notes[5]);
      delay(250);
      noTone(SOUND_PIN);
      break;
    /*case 5: // Parking sound: long range
      tone(SOUND_PIN, notes[5]);
      delay(150);
      noTone(SOUND_PIN);
      delay(150);
      break;
    case 6: // Parking sound: medium range
      tone(SOUND_PIN, notes[5]);
      delay(100);
      noTone(SOUND_PIN);
      delay(100);
      break;
    case 7: //Parking sound: short range
      tone(SOUND_PIN, notes[5]);
      delay(50);
      noTone(SOUND_PIN);
      delay(50);
      break;*/
  }
}

/** 
 * Function that configures the device to read RFID tags
 */
void setupNFC() {
  nfc.begin();
  nfc.setPassiveActivationRetries(0xFF);
  nfc.SAMConfig();
}

/**
 * Function that reads the content of a NFC tag.
 * As it is thought to read the code of the driver, it returns the
 * code of the driver (String) if it's valid or -1 if an error ocurred
 * The "timeToSound" parameter indicates how much to wait until a sound alarm
 */
String readCard(int timeToSound) {
  
  // Required variables
  uint8_t success = true;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 }; uint8_t uidLength; // UID is 7 bytes sized by default
  String output = "";
  
  // Validate tag
  success = success & nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, timeToSound); // If nothing is read in specified time, throw error
  if (success) {

    // Creating a "clean" block and setting the working block
    uint8_t block[BLOCK_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int actualBlock = FIRST_UTIL_BLOCK;

    for (int i=0; i<(TOTAL_UTIL_SECTORS*UTIL_BLOCKS_PER_SECTOR*BLOCK_SIZE); i+=BLOCK_SIZE) {
      
      // Validate block
      uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }; // The validation key is 6 bytes sized by default
      success = success && nfc.mifareclassic_AuthenticateBlock(uid, uidLength, actualBlock, 0, keya);

      if (success) {

        // Validate block
        uint8_t data[BLOCK_SIZE];
        success = success & nfc.mifareclassic_ReadDataBlock(actualBlock, data);

        // Read block
        if (success)
          if (strcmp(block, data) != 0)
            for (int j=0; j<BLOCK_SIZE; j++)
              output += (char) data[j];
              
      }
      
      // Move to a valid block (not overwrite the last block of each section)
      actualBlock++;
      if (actualBlock % (UTIL_BLOCKS_PER_SECTOR+1) == UTIL_BLOCKS_PER_SECTOR) {
        actualBlock++;
      }
      
    }
  
  }

  if (success) return parseNumber(output.toInt(), 4);
  else return "-1"; 
    
}

/**
 * Function that writes a text in a file (existing or not) of a SD
 * The parameters of the function are the following:
 *  - String filename: Name of the file to write in
 *  - String content: Text to write on the SD file
 *  - Boolean overwrite: "true" if the content should overwrite the file and "false" if not
 */
void writeOnSD(String fileName, String content, boolean overwrite) {
  File mySDFile;
  if (overwrite) {
    writeOnSD(fileName + ".bac", content, false);
    deleteFromSD(fileName);
  }
  mySDFile = SD.open(fileName, FILE_WRITE);
  if (mySDFile) {
    mySDFile.print(content); mySDFile.close();
    deleteFromSD(fileName + ".bac");
  }
}

/**
 * Function that read a file from a SD
 * The parameters of the function are the following:
 *  - String filename: Name of the file to read from
 */
String readFromSD(String fileName) {
  File mySDFile;
  String content = "";
  mySDFile = SD.open(fileName);
  if (mySDFile) {
    while (mySDFile.available())
      content += (char) mySDFile.read();
    mySDFile.close();
  }
  return content;
}

/**
 * Function that deletes a file from a SD
 * The parameters of the function are the following:
 *  - String filename: Name of the file to delete
 */
void deleteFromSD(String fileName) {
  SD.remove(fileName);
}

/** 
 * Function that extract the desired data from the NMEA strings
 */
void extractFromNMEA() {
  // Needed variables to extract the speed
  char arrVTG[VTG.length()];
  VTG.toCharArray(arrVTG, VTG.length() + 1);

  // Read speed
  char *token; token = strtok(arrVTG, ",");
  token = strtok(NULL, ","); token = strtok(NULL, ",");
  token = strtok(NULL, ","); token = strtok(NULL, ",");
  token = strtok(NULL, ","); spd = atof(token);

  // Needed variables to extract the coordinates
  char arrGLL[GLL.length()];
  GLL.toCharArray(arrGLL, GLL.length() + 1);
  float aux_lat, aux_lon;  

  // Read coordinates
  token = strtok(arrGLL, ",");
  token = strtok(NULL, ","); aux_lat = parseGNSS(atof(token)); token = strtok(NULL, ",");
  if (token[0] == 'S') aux_lat = -aux_lat;  // If latitude is on the South, convert it to negative
  if ((aux_lat < -90) || (aux_lat > 90)) aux_lat = 0;
  token = strtok(NULL, ","); aux_lon = parseGNSS(atof(token)); token = strtok(NULL, ",");
  if (token[0] == 'W') aux_lon = -aux_lon;  // If longitude is on the West, convert it to negative
  if ((aux_lon < -180) || (aux_lon > 180)) aux_lon = 0;
  token = strtok(NULL, ", "); t = atof(token);
  
  // Compute distance between stored and new coordinates
  if ((coord_lat == 180) && (coord_lon == 180) && (aux_lat != 0) && (aux_lon != 0)) {   // First measure
      sound(3);
      coord_lat = aux_lat; coord_lon = aux_lon;
  } else {                                  // Rest of the measures
    dist_act += getTravelledDistance(aux_lat, aux_lon);
    coord_lat = aux_lat; coord_lon = aux_lon;
    storeMeasure();
  }

  // Clean GLL and VTG
  GLL = ""; VTG = "";
}

/**
 * Function that computes the distance between 2 points using Haversine formula
 * The parameters of the function are the following:
 *  - Float aux_lat: Latitude of the new measured coordinates
 *  - Float aux_lon: Longitude of the new measured coordinates
 */
float getTravelledDistance(float aux_lat, float aux_lon) {
  
  // Computation of the error based on a normal distribution
  float err = gen.random(); // Error
  if (err < 0) err = -err;

  // Computation of the distance using the Haversine formula
  float fi = toRadians(aux_lat - coord_lat);
  float lambda = toRadians(aux_lon - coord_lon);
  float a = pow(sin(fi/2),2) + cos(toRadians(coord_lat)) * cos(toRadians(aux_lat)) * pow(sin(lambda/2),2);
  float d = 2 * EARTH_RADIUS * asin(sqrt(a));

  d -= err; 
  return d;
}

/**
 * Function that computes the given degrees angle in radians
 */
float toRadians(float degr) {
  return degr * PI/180;
}

/**
 * Function that parses the GNSS coordinates given by the antenna (hhmm.mmmm) into hours
 */
float parseGNSS(float coord) {
  float aux;
  aux = trunc(coord/100) + (coord - trunc(coord/100)*100)/60;
  return aux;
}

/**
 * Function that stores a new measure (JSON format) on the SD card
 */
void storeMeasure() {

  DynamicJsonDocument doc(JSON_OBJECT_SIZE(20));

  String measure = parseNumber(measure_num, 5);

  doc["id"] = "T" + id_trip + "M" + measure;
  doc["id_trip"] = id_trip;
  doc["id_driv"] = id_driv;
  doc["dist_act"] = String(dist_act);
  doc["coord_lat"] = coord_lat;
  doc["coord_lon"] = coord_lon;
  doc["speed"] = spd;
  doc["time"] = t;

  String output;
  serializeJson(doc, output);
  Serial.println("");
  
  // Update some values
  writeOnSD(ODOMETER_FILE, String(dist_act), true);   // Store odometer
  if (SD.exists(MEASURES_FILE))                       // Store measure
    writeOnSD(MEASURES_FILE, "," + output, false);
  else
    writeOnSD(MEASURES_FILE, output, true);
  measure_num++;                                      // Increase number of measures
  
}

/**
 * Function that parses a number into the format required for storing
 * The parameters of the function are the following:
 *  - Int number: Number to parse
 *  - Int positions: Number of positions of the desired format
 */
String parseNumber(int number, int positions) {
  String s = "";
  int zeros = pow(10,(positions-1));
  bool finish = false;

  while(!finish) {
    if (number > zeros) {
      s = s + String(number);
      finish = true;
    } else {
      s = s + "0";
      zeros = zeros / 10;
    }
  }

  return s;
}
