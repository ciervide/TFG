#include <SD.h>

#define WIDTH 320
#define HEIGHT 240
#define CROP_WIDTH 18 
#define CROP_HEIGHT 27

int SDPIN = 10;

void setup() {
  
  Serial.begin(9600);
  pinMode(SDPIN, OUTPUT);

  if (!SD.begin(SDPIN)) {
    Serial.println("SD initialization failed"); return;
  }
  Serial.println("SD initialization completed");

  deleteFromSD("xHist.txt"); deleteFromSD("yHist.txt");
  for (int i=0; i<10; i++) {
    String s = "N" + String(i) + ".raw";
    Serial.println("\n------------------------------------------------------------------------------------------------------------");
    Serial.print("                                               Reading "); Serial.println(s);
    Serial.println("------------------------------------------------------------------------------------------------------------");
    createHistograms(s);
  }
  Serial.println("\nTraining samples processed");
  
}

void loop() { }



// --------------------------- ARTIFICIAL VISION FUNCTIONS --------------------------- //

void createHistograms(String fileName) {

  Serial.println("Starting recognition process");
  
  File mySDFile = SD.open(fileName);
  int xHist[HEIGHT], yHist[WIDTH];
  memset(xHist, 0, sizeof(xHist));
  memset(yHist, 0, sizeof(yHist));
  if (mySDFile) {
    for (int i=0; i<HEIGHT; i++) {
      for (int j=0; j<WIDTH; j++) {
        byte r = mySDFile.read();
        byte g = mySDFile.read();
        byte b = mySDFile.read();
        if (rgbToGray(r, g, b) > 80) {
          //row += '1';
          xHist[i]++;
          yHist[j]++;
        }
      }
      Serial.print((i*100)/HEIGHT); Serial.println("% completed");
    }
    mySDFile.close();
  }

  int x, y;
  getCorners(xHist, yHist, x, y);
  for (int i=x; i<x+CROP_HEIGHT; i++)
    writeOnSD("xHist.txt", String(xHist[i])+" ", false);
  writeOnSD("xHist.txt", "\n", false);
  for (int i=y-CROP_WIDTH+1; i<=y; i++)
    writeOnSD("yHist.txt", String(yHist[i])+" ", false);
  writeOnSD("yHist.txt", "\n", false);
  
  Serial.println("Recognition process completed");
  
}

byte rgbToGray(byte r, byte g, byte b) {
  return 0.2989*r + 0.5870*g + 0.1140*b;
}

void getCorners(int xHist[HEIGHT], int yHist[WIDTH], int &x, int &y) {
  bool b = false;
  x = -1;
  while (!b) {
    x++;
    b = b || xHist[x] != 0 || x == HEIGHT;
  }
  b = false; y = WIDTH;
  while (!b) {
    y--;
    b = b || yHist[y] != 0 || y == 0;
  }
}

// --------------------------- ARTIFICIAL VISION FUNCTIONS --------------------------- //



// -------------------------------- SD CARD FUNCTIONS -------------------------------- //

void writeOnSD(String fileName, String content, boolean overwrite) {
  if (overwrite)
    deleteFromSD(fileName);
  File mySDFile = SD.open(fileName, FILE_WRITE);
  if (mySDFile) {
    mySDFile.print(content); mySDFile.close();
  }
}

String readFromSD(String fileName) {
  String content = "";
  File mySDFile = SD.open(fileName);
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

// -------------------------------- SD CARD FUNCTIONS -------------------------------- //
