// ArduCAM Mini demo (C)2017 Lee
// Web: http://www.ArduCAM.com
// This program is a demo of how to use the enhanced functions
// This demo was made for ArduCAM_Mini_5MP_Plus.
// It can continue shooting and store it into the SD card  in RAW format
// The demo sketch will do the following tasks
// 1. Set the camera to RAW output mode.
// 2. Capture a RAW photo and buffer the image to FIFO
// 3. Write the picture data to the SD card
// 4. Close the file

#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include <SD.h>
#include "memorysaver.h"

#if !(defined (OV5642_MINI_5MP_PLUS))
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif
#define   FRAMES_NUM    0x06

const int CS = 7; // Slave select
#define SD_CS 9   // SD Pin

bool is_header = false;
int total_time = 0;
ArduCAM myCAM(OV5642, CS);
uint8_t read_fifo_burst(ArduCAM myCAM);

void setup() {
  
  uint8_t vid, pid;
  uint8_t temp;
  #if defined(__SAM3X8E__)
    Wire1.begin();
  #else
    Wire.begin();
  #endif
  Serial.begin(115200);
  Serial.println(F("ArduCAM Start!"));
  SPI.begin();
  
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);
  
  //Reset the CPLD
  myCAM.write_reg(0x07, 0x80);
  delay(100);
  myCAM.write_reg(0x07, 0x00);
  delay(100); 
    
  while(1){
    //Check if the ArduCAM SPI bus is OK
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM.read_reg(ARDUCHIP_TEST1);
    if(temp != 0x55) {
      Serial.println(F("SPI interface Error!"));
      delay(1000); continue;
    } else {
      Serial.println(F("SPI interface OK."));break;
    }
  }
  
  while(1){
    myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
    if ((vid != 0x56) || (pid != 0x42)) {
      Serial.println(F("Can't find OV5642 module!"));
      delay(1000);continue;
    } else {
      Serial.println(F("OV5642 detected."));break;      
    }
  }

  //Initialize SD Card
  while(!SD.begin(SD_CS)) {
    Serial.println(F("SD Card Error!"));
    delay(1000);
  }
  Serial.println(F("SD Card detected."));
  
  //Change to RAW capture mode and initialize the OV5640 module
  myCAM.set_format(RAW);
  myCAM.InitCAM();
  myCAM.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
  myCAM.clear_fifo_flag();
  myCAM.write_reg(ARDUCHIP_FRAMES, FRAMES_NUM);
  
}

void loop() {
  
  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();
  myCAM.OV5642_set_RAW_size(OV5642_320x240);
  delay(1000);
  
  //Start capture
  myCAM.start_capture();
  Serial.println(F("start capture."));
  total_time = millis();
  while ( !myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)); 
  Serial.println(F("CAM Capture Done."));
  total_time = millis() - total_time;
  Serial.print(F("capture total_time used (in miliseconds):"));
  Serial.println(total_time, DEC);
  total_time = millis();
  read_fifo_burst(myCAM);
  total_time = millis() - total_time;
  Serial.print(F("save capture total_time used (in miliseconds):"));
  Serial.println(total_time, DEC);
  
  //Clear the capture done flag
  myCAM.clear_fifo_flag();
  delay(5000);
  
}

uint8_t read_fifo_burst(ArduCAM myCAM) {
  
  uint8_t temp = 0, temp_last = 0;
  uint32_t length = 0;
  static int i = 0;
  static int k = 0;
  char str[8];
  File outFile;
  byte buf[256]; 
  length = myCAM.read_fifo_length();
  Serial.print(F("The fifo length is :"));
  Serial.println(length, DEC);
  if (length >= MAX_FIFO_SIZE) {  // 8M
    Serial.println("Over size.");
    return 0;
  }
  if (length == 0 ) {             // 0 kb
    Serial.println(F("Size is 0."));
    return 0;
  } 
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();//Set fifo burst mode
  i = 0;
  while (length--) {
    temp_last = temp;
    temp =  SPI.transfer(0x00);
    
    //Read RAW data from FIFO
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) {  //If find the end ,break while,
      buf[i++] = temp;  //save the last  0XD9
         
      //Write the remain bytes in the buffer
      myCAM.CS_HIGH();
      outFile.write(buf, i);  
        
      //Close the file
      outFile.close();
      Serial.println(F("OK"));
      is_header = false;
      myCAM.CS_LOW();
      myCAM.set_fifo_burst();
      i = 0;
    }
    if (is_header == true) { 
      //Write image data to buffer if not full
      if (i < 256)
       buf[i++] = temp;
      else {
        //Write 256 bytes image data to file
        myCAM.CS_HIGH();
        outFile.write(buf, 256);
        i = 0;
        buf[i++] = temp;
        myCAM.CS_LOW();
        myCAM.set_fifo_burst();
      }        
    } else if ((temp == 0xD8) & (temp_last == 0xFF)) {
      is_header = true;
      myCAM.CS_HIGH();
      
      //Create a avi file
      k = k + 1;
      itoa(k, str, 10);
      strcat(str, ".jpg");
      
      //Open the new file
      outFile = SD.open(str, O_WRITE | O_CREAT | O_TRUNC);
      if (!outFile) {
        Serial.println(F("File open failed"));
        while (1);
      }
      myCAM.CS_LOW();
      myCAM.set_fifo_burst();   
      buf[i++] = temp_last;
      buf[i++] = temp;   
    }
  }
  myCAM.CS_HIGH();
  return 1;
}
