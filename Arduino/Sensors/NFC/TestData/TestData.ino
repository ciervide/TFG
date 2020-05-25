#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
 
#define PN532_IRQ   (2)
#define PN532_RESET (3)
 
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);
 
void setup(void) {
  Serial.begin(115200);
 
  // Configurar para leer etiquetas RFID
  nfc.begin();
  nfc.setPassiveActivationRetries(0xFF);
  nfc.SAMConfig();
 
  Serial.println("Esperando tarjeta");
}
 
const uint8_t validData[16] = { 'F', 'i', 'r', 's', 't', ' ', 't', 'e', 's', 't', ' ', '-', ' ', 'N', 'F', 'C' };  // Ejemplo de clave valida
 
//Función para comparar dos vectores
bool isEqualArray(uint8_t* arrayA, uint8_t* arrayB, uint8_t length)
{
  for (uint8_t index = 0; index < length; index++)
  {
    if (arrayA[index] != arrayB[index]) return false;
  }
  return true;
}
 
void loop(void) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;
 
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if (success)
  {
    uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
 
    success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keya);
    if (success)
    {
      uint8_t data[16];
 
      success = nfc.mifareclassic_ReadDataBlock(4, data);
      if (success)
      {
        if (isEqualArray(data, validData, sizeof validData))
        {
          Serial.println("Tarjeta valida");
          delay(5000);
        }
        else
        {
          Serial.println("Tarjeta invalida");
          delay(5000);
        }
      }
    }
    else
    {
      Serial.println("Fallo autentificar tarjeta");
      delay(5000);
    }
  }
}
