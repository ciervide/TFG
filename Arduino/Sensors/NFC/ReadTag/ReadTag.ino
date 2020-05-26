#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
 
#define PN532_IRQ   (2)
#define PN532_RESET (3)
 
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

// Some constants refering to the Mifare Classic NFC standard
const int FIRST_UTIL_BLOCK = 4;
const int TOTAL_UTIL_SECTORS = 15;
const int UTIL_BLOCKS_PER_SECTOR = 3;
const int BLOCK_SIZE = 16;
 
void setup(void) 
{
  Serial.begin(115200);
  setupNFC();  
  Serial.println("Approach the card\n");

  String output = readCard();
  Serial.print("\""); Serial.print(output); Serial.println("\" was read");
}
 
void loop(void) {
  
}

// Configure the device to read RFID tags
void setupNFC() {  
  nfc.begin();
  nfc.setPassiveActivationRetries(0xFF);
  nfc.SAMConfig();
}

// Read the content of a NFC tag
String readCard() {
  
  // Required variables
  uint8_t success = true;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 }; uint8_t uidLength; // UID is 7 bytes sized by default
  String output = "";
  
  // Validate tag
  success = success & nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if (success) {

    Serial.println("Reading card...");

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

  if (success) return output;
  else return "An error occured. Please, try it again"; 
    
}
