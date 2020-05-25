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
 
const uint8_t validUID[4] = { 0xF2, 0x51, 0xD2, 0xB2 };  // Ejemplo de UID valido
 
//Función para comparar dos vectores
bool isEqualArray(uint8_t* arrayA, uint8_t* arrayB, uint8_t length)
{
  for (uint8_t index = 0; index < length; index++)
  {
    if (arrayA[index] != arrayB[index]) return false;
  }
  return true;
}
 
void loop(void)
{
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;
 
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if (success)
  {
    uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
 
    if (isEqualArray(uid, validUID, uidLength))
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
