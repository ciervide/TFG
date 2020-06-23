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
 
void setup(void) {
  Serial.begin(115200);
  setupNFC();  
  Serial.println("Approach the card\n");

  if (writeCard("1998\n")) Serial.println("Card is written now!");
  else Serial.println("An error occured. Please, try it again");

  /*if (cleanCard()) Serial.println("Card is clean now!");
  else Serial.println("An error occured. Please, try it again");*/
  
}
 
void loop(void) {
  
}

// Configure the device to read RFID tags
void setupNFC() {  
  nfc.begin();
  nfc.setPassiveActivationRetries(0xFF);
  nfc.SAMConfig();
}

// Write an input text into the NFC tag
uint8_t writeCard(String input) {

  // Required variables
  uint8_t success = true;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 }; uint8_t uidLength; // UID is 7 bytes sized by default

  // Validate tag
  success = success & nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if (success) {

    Serial.println("Writing card...");

    // Parsing input text
    uint8_t data[input.length()]; input.toCharArray(data, input.length());
    uint8_t block[BLOCK_SIZE]; int actualBlock = FIRST_UTIL_BLOCK;
    for (int i=0; i<sizeof(data); i+=BLOCK_SIZE) {
  
      if (sizeof(data)-i >= BLOCK_SIZE) {
        for (int j=i; j<i+BLOCK_SIZE; j++){
          block[j-i] = data[j];
        }
      } else {
        for (int j=i; j < sizeof(data); j++){
          block[j-i] = data[j];
        }
        for (int j = sizeof(data)-i; j<BLOCK_SIZE; j++){
          block[j] = 0;
        }
      }
  
      // Validate block
      uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }; // The validation key is 6 bytes sized by default
      success = success & nfc.mifareclassic_AuthenticateBlock(uid, uidLength, actualBlock, 0, keya);

      // Write block
      if (success) success = success & nfc.mifareclassic_WriteDataBlock (actualBlock, block);

      // Move to a valid block (not overwrite the last block of each section)
      actualBlock++;
      if (actualBlock % (UTIL_BLOCKS_PER_SECTOR+1) == UTIL_BLOCKS_PER_SECTOR) {
        actualBlock++;
      }
         
    }
    
  }
    
  return success;
  
}

// Clean the NFC tag
uint8_t cleanCard() {

  // Required variables
  uint8_t success = true;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 }; uint8_t uidLength; // UID is 7 bytes sized by default
  
  // Validate tag
  success = success && nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if (success) {

    Serial.println("Cleaning card...");

    // Creating a "clean" block and setting the working block
    uint8_t block[BLOCK_SIZE];
    for (int i=0; i<BLOCK_SIZE; i++) 
      block[i] = 0;
    int actualBlock = FIRST_UTIL_BLOCK;

    for (int i=0; i<(TOTAL_UTIL_SECTORS*UTIL_BLOCKS_PER_SECTOR*BLOCK_SIZE); i+=BLOCK_SIZE) {

      // Validate block
      uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };  // The validation key is 6 bytes sized by default
      success = success && nfc.mifareclassic_AuthenticateBlock(uid, uidLength, actualBlock, 0, keya);

      // Clean block
      if (success) success = success && nfc.mifareclassic_WriteDataBlock (actualBlock, block);
      
      // Move to a valid block (not overwrite the last block of each section)
      actualBlock++;
      if (actualBlock % (UTIL_BLOCKS_PER_SECTOR+1) == UTIL_BLOCKS_PER_SECTOR) {
        actualBlock++;
      }

    }
    
  }
    
  return success;
  
}
