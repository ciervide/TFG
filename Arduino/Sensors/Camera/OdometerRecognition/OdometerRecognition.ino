#include <SD.h>

#define HEIGHT 240
#define WIDTH 320
#define CROP_HEIGHT 27
#define CROP_WIDTH 18 
#define ODOMETER_CIPHERS 6

int SDPIN = 10;
String fileName = "I1.raw";
String xHistsFileName = "XHIST.TXT";
String yHistsFileName = "YHIST.TXT";

void setup() {
  
  Serial.begin(9600);
  pinMode(SDPIN, OUTPUT);

  if (!SD.begin(SDPIN)) {
    Serial.println("SD initialization failed"); return;
  }
  Serial.println("SD initialization completed");

  
  Serial.println("\nStarting recognition process");
  float odometer = recognizeOdometer();
  Serial.println("\nRecognition process completed");
  Serial.print("Odometer: " + String(odometer));
  
}

void loop() { }



// --------------------------- ARTIFICIAL VISION FUNCTIONS --------------------------- //

float recognizeOdometer() {

  Serial.println("\nProcessing image");
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
          xHist[i]++;
          yHist[j]++;
        }
      }
      Serial.println(String((i*100)/HEIGHT) + "% completed");
    }
    mySDFile.close();
  }
  Serial.println("Image processed");

  Serial.println("\nReading samples histograms");
  int xHists[10][CROP_HEIGHT], yHists[10][CROP_WIDTH];
  memset(xHists, 0, 10*CROP_HEIGHT*sizeof(int));
  memset(yHists, 0, 10*CROP_WIDTH*sizeof(int));
  String xHistsSRaw = readFromSD(xHistsFileName), yHistsSRaw = readFromSD(yHistsFileName);
  char xHistsCRaw[xHistsSRaw.length()], yHistsCRaw[yHistsSRaw.length()], *aux;
  xHistsSRaw.toCharArray(xHistsCRaw, xHistsSRaw.length());
  yHistsSRaw.toCharArray(yHistsCRaw, yHistsSRaw.length());

  aux = strtok(xHistsCRaw, " \n");
  for (int i=0; i<10; i++) {
    for (int j=0; j<CROP_HEIGHT; j++) {
      xHists[i][j] = atoi(aux);
      aux = strtok(NULL, " \n");
    }
  }
  
  aux = strtok(yHistsCRaw, " \n");
  for (int i=0; i<10; i++) {
    for (int j=0; j<CROP_WIDTH; j++) {
      yHists[i][j] = atoi(aux);
      aux = strtok(NULL, " \n");
    }
  }
  Serial.println("Samples histograms read");

  Serial.println("\nProcessing image's odometer");
  int x = 0, y = WIDTH;
  getTopMargin(xHist, x);
  getRightObject(yHist, y); getRightGap(yHist, y);
  getRightObject(yHist, y); getRightGap(yHist, y);

  float odometer = 0;
  for (int i=0; i<ODOMETER_CIPHERS; i++) {
    getRightObject(yHist, y);
    float aux = 1;
    for (int j=0; j<i; j++)
      aux = aux*10;
    odometer += identifyNumber(x, y, xHists, yHists)*aux;
    getRightGap(yHist, y);
    Serial.println(String(i+1) + "/" + String(ODOMETER_CIPHERS) + " ciphers processed");
  }
  Serial.println("Image's odometer processed");

  return odometer;
  
}

byte rgbToGray(byte r, byte g, byte b) {
  return 0.2989*r + 0.5870*g + 0.1140*b;
}

void getTopMargin(int xHist[HEIGHT], int &x) {
  while (!(xHist[x] != 0 || x == HEIGHT))
    x++;
}

void getRightObject(int yHist[WIDTH], int &y) {
  while (!(yHist[y-1] != 0 || y == 0)) {
    y--;
  }
}

void getRightGap(int yHist[WIDTH], int &y) {
  while (!(yHist[y-1] == 0 || y == 0)) {
    y--;
  }
}

int identifyNumber(int x, int y, int xHists[10][CROP_HEIGHT], int yHists[10][CROP_WIDTH]) {
  
  File mySDFile = SD.open(fileName);
  int xHist[CROP_HEIGHT], yHist[CROP_WIDTH];
  memset(xHist, 0, CROP_HEIGHT*sizeof(int));
  memset(yHist, 0, CROP_WIDTH*sizeof(int));
  if (mySDFile) {
    for (int i=0; i<HEIGHT; i++) {
      for (int j=0; j<WIDTH; j++) {
        if ((i>=x) && (i<x+CROP_HEIGHT) && (j>=y-CROP_WIDTH) && (j<y)) {
          byte r = mySDFile.read();
          byte g = mySDFile.read();
          byte b = mySDFile.read();
          if (rgbToGray(r, g, b) > 80) {
            xHist[i-x]++;
            yHist[j-(y-CROP_WIDTH)]++;
          }
        } else {
          mySDFile.read(); mySDFile.read(); mySDFile.read();
        }
      }
    }
    mySDFile.close();
  }

  int bestX = -1, bestXIndex = -1;
  for (int i=0; i<10; i++) {
    int dist = 0;
    for (int j=0; j<CROP_HEIGHT; j++) {
      dist += abs(xHist[j]-xHists[i][j]);
    }
    if ((bestX == -1) || (dist < bestX)) {
      bestX = dist;
      bestXIndex = i;
    }
  }

  int bestY = -1, bestYIndex = -1;
  for (int i=0; i<10; i++) {
    int dist = 0;
    for (int j=0; j<CROP_WIDTH; j++) {
      dist += abs(yHist[j]-yHists[i][j]);
    }
    if ((bestY == -1) || (dist < bestY)) {
      bestY = dist;
      bestYIndex = i;
    }
  }

  if (bestX <= bestY)
    return bestXIndex;
  else
    return bestYIndex;
    
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
