#include <Adafruit_PN532.h>
#include <Gaussian.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Wire.h>

#define PN532_IRQ_PIN   2 // NFC Shield
#define PN532_RESET_PIN 3 // NFC Shield
#define NMEA_RX_PIN 4     // NMEA
#define NMEA_TX_PIN 5     // NMEA
#define SOUND_PIN      8  // Buzzer
#define MODE_CHANGE_PIN 9 // Button
#define SD_PIN         10 // SD module

// Variables for menus
int actualMode = 0;
unsigned long previousMillis = 0;

// Variables and constants for NFC (Mifare Classic NFC Standard)
Adafruit_PN532 nfc(PN532_IRQ_PIN, PN532_RESET_PIN);
const int FIRST_UTIL_BLOCK = 4;
const int TOTAL_UTIL_SECTORS = 15;
const int UTIL_BLOCKS_PER_SECTOR = 3;
const int BLOCK_SIZE = 16;

// Variables por SD files read/write
File mySDFile;
const String TRAVEL_FILE = "travel.txt";
const String MEASURES_FILE = "measures.txt";

// Variables and constants for NMEA
const float EARTH_RADIUS = 6371e3;
const float MEAN = 0.2:
const float VAR = 0.4;
SoftwareSerial gps = SoftwareSerial(NMEA_RX_PIN, NMEA_TX_PIN);
Gaussian gen;
String str = "";
float f_lat = 180, f_lon = 180, dist = 0, max_dist = 0, num_measures = 0;

// Variables to store the measures
int measure_num;
int id_trip;
int id_driv;
int dist_act;
int coord_lat;
int coord_lon;
int spd;
int t;

void setup() {

                                                                                              //Setting the serial
                                                                                              Serial.begin(115200);

  // Setting variables and pins
  pinMode(NMEA_RX_PIN, INPUT);
  pinMode(NMEA_TX_PIN, OUTPUT);
  pinMode(MODE_CHANGE_PIN, INPUT);
  pinMode(SD_PIN, OUTPUT);
  gps.begin(9600);
  gps.flush();
  SD.begin(SD_PIN);
  gen = Gaussian(MEAN, VAR);
  measure_num = 0;
  dist_act = 0;

  // Setting NFC and reading driver code (if time pass without reading, sound an alarm)
  setupNFC();
  id_driv = readCard(10*1000); // 10 seconds
  while(id_driv == -1) {
    sound(4);
    id_driv = readCard(250);
  }
  sound(3);

                                                                                              Serial.print("Driver code: ");
                                                                                              Serial.println(id_driv);

  // Reading travel number from SD, updating it and storing on a local variable
  id_trip = readFromSD(TRAVEL_FILE).toInt()+1;
  writeOnSD(TRAVEL_FILE, String(id_trip), true);

                                                                                              Serial.print("Travel number: ");
                                                                                              Serial.println(id_trip);
  
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
    case 5: // Parking sound: long range
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
      break;
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
 * code of the driver (int) if it's valid or -1 if an error ocurred
 * The "timeToSound" parameter indicates how much to wait until a sound alarm
 */
int readCard(int timeToSound) {
  
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

  if (success) return output.toInt();
  else return -1; 
    
}

/**
 * Function that writes a text in a file (existing or not) of a SD
 * The parameters of the function are the following:
 *  - String filename: Name of the file to write in
 *  - String content: Text to write on the SD file
 *  - Boolean overwrite: "true" if the content should overwrite the file and "false" if not
 */
void writeOnSD(String fileName, String content, boolean overwrite) {
  if (overwrite)
    deleteFromSD(fileName);
  mySDFile = SD.open(fileName, FILE_WRITE);
  if (mySDFile) {
    mySDFile.println(content); mySDFile.close();
  }
}

/**
 * Function that read a file from a SD
 * The parameters of the function are the following:
 *  - String filename: Name of the file to read from
 */
String readFromSD(String fileName) {
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
  float fi = toRadians(aux_lat - f_lat);
  float lambda = toRadians(aux_lon - f_lon);
  float a = pow(sin(fi/2),2) + cos(toRadians(f_lat)) * cos(toRadians(aux_lat)) * pow(sin(lambda/2),2);
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
 * Function that prints the content of a char array
 */
void printCharArr(char arr[]) {
  int len = strlen(arr);
  for (int i = 0; i < len; i++) {
    Serial.print(arr[i]);
  }
}
