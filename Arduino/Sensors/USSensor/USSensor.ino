int USSensor_PIN = A0;
float lectura;
int cm;

float a = 17478.4, b = -1.2093;

void setup() {
  Serial.begin(9600);
}

void loop() {
  lectura = analogRead(USSensor_PIN);
  // Leer a 2 distancias concretas y calcular a y b de la ecuación L = a*x^b (con L distancia real en cm y x distancia medida en "lectura")
  cm = a * pow(lectura, b);
  Serial.print("Obstaculo a "); Serial.print(cm); Serial.print("cm - "); Serial.println(getLevelByDistance(cm));
  delay(1000);
}

int getLevelByDistance(int cm) {
  int level;
  if ((0 <= cm) && (cm <= 15)) 
    level = 1;
  else if ((15 < cm) && (cm <= 40))
    level = 2;
  else if ((40 < cm) && (cm <= 65))
    level = 3;
  else if ((65 < cm) && (cm <= 80))
    level = 4;
  else
    level = -1;
  return level;
}
