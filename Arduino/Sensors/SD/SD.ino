#include <SD.h>

File mySDFile;
int SDPIN = 10;

void setup() {
  
  Serial.begin(9600);
  pinMode(SDPIN, OUTPUT);

  if (!SD.begin(SDPIN)) {
    Serial.println("SD initialization failed"); return;
  }
  Serial.println("SD initialization completed");

  writeOnSD("test.txt", "Hello world!", true);
  String str = readFromSD("test.txt");
  Serial.println(str);
  
}

void loop() {
  // put your main code here, to run repeatedly:

}

void writeOnSD(String fileName, String content, boolean overwrite) {
  if (overwrite)
    deleteFromSD(fileName);
  mySDFile = SD.open(fileName, FILE_WRITE);
  if (mySDFile) {
    mySDFile.println(content); mySDFile.close();
  }
}

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

void deleteFromSD(String fileName) {
  SD.remove(fileName);
}
