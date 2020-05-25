#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
 
#define PN532_IRQ   (2)
#define PN532_RESET (3)
 
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);
 
void setup(void) {
  Serial.begin(115200);
  setupNFC();  
  Serial.println("Approach the card\n");
  
  //writeCard("Example of text parsing --> WELL DONE!");
  cleanCard();
}
 
void loop(void) {
  
}

// Configure the device to read RFID tags
void setupNFC() {  
  nfc.begin();
  nfc.setPassiveActivationRetries(0xFF);
  nfc.SAMConfig();
}

// Write an input text inot the NFC tag
void writeCard(String input) {

  // Required variables
  uint8_t success = true;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 }; uint8_t uidLength;

  // Validate tag
  success = success & nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if (success) {

    Serial.println("Writing card...");

    // Parsing input text
    uint8_t data[input.length()]; input.toCharArray(data, input.length());
    uint8_t block[16]; int actualBlock = 4;
    for (int i=0; i<sizeof(data); i+=16) {
  
      if (sizeof(data)-i >= 16) {
        for (int j=i; j<i+16; j++){
          block[j-i] = data[j];
        }
      } else {
        for (int j=i; j < sizeof(data); j++){
          block[j-i] = data[j];
        }
        for (int j = sizeof(data)-i; j<16; j++){
          block[j] = 0;
        }
      }
  
      // Validate block
      uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }; 
      success = success & nfc.mifareclassic_AuthenticateBlock(uid, uidLength, actualBlock, 0, keya);
      if (success) {
        // Write block
        success = success & nfc.mifareclassic_WriteDataBlock (actualBlock, block);
      }

      // Move to a valid block (not overwrite the last block of each section)
      actualBlock++;
      if (actualBlock % 4 == 3) {
        actualBlock++;
      }
         
    }
    
  }
    
  if (success) Serial.println("Card is written now!");
  else Serial.println("An error occured. Please, try it again");
  
}

// Clean the NFC tag
void cleanCard() {

  // Required variables
  uint8_t success = true;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 }; uint8_t uidLength;
  
  // Validate tag
  success = success && nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if (success) {

    Serial.println("Cleaning card...");

    // Creating a "clean" block
    uint8_t block[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int actualBlock = 4;

    // Mifare Classic has 15 writtable sectors (the first one has data) with 3 writtable blocks (the last one has data) of 16 bytes --> 15*48 writtable bytes
    for (int i=0; i<15*48; i+=16) {

      // Validate block
      uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }; 
      success = success && nfc.mifareclassic_AuthenticateBlock(uid, uidLength, actualBlock, 0, keya);

      // Clean block
      if (success) success = success && nfc.mifareclassic_WriteDataBlock (actualBlock, block);
      
      // Move to a valid block (not overwrite the last block of each section)
      actualBlock++;
      if (actualBlock % 4 == 3) {
        actualBlock++;
      }

    }
    
  }
    
  if (success) Serial.println("Card is clean now!");
  else Serial.println("An error occured. Please, try it again");
  
}
