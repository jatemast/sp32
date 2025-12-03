const char* ssid = "limpiador";
const char* password = "123456789";

#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "app_httpd.cpp" // Incluir el archivo para la declaración de startWebServer()
// Definición de los pines para el sensor ultrasónico HC-SR04
const int trigPin = 25; // Pin de disparo (Trigger)
const int echoPin = 26; // Pin de eco (Echo)

// Declaración de funciones para el sensor ultrasónico
void initUltrasonic();
long readUltrasonicDistance();

// Enumeración para los modos de operación
enum RobotMode {
  MODE_REMOTE,
  MODE_AUTO
};

// Variable global para el modo actual del robot
RobotMode currentMode = MODE_REMOTE;

// Variables de estado del movimiento (antes en app_httpd.cpp)
int speed = 255;
int noStop = 0;
enum state {fwd,rev,stp};
state actstate = stp;

const int MotPin0 = 12;
const int MotPin1 = 13;
const int MotPin2 = 14;
const int MotPin3 = 15;

void initMotors()
{
  ledcSetup(3, 2000, 8); // 2000 hz PWM, 8-bit resolution
  ledcSetup(4, 2000, 8); // 2000 hz PWM, 8-bit resolution
  ledcSetup(5, 2000, 8); // 2000 hz PWM, 8-bit resolution
  ledcSetup(6, 2000, 8); // 2000 hz PWM, 8-bit resolution
  ledcAttachPin(MotPin0, 3);
  ledcAttachPin(MotPin1, 4);
  ledcAttachPin(MotPin2, 5);
  ledcAttachPin(MotPin3, 6);
}

const int ServoPin = 2;
void initServo(){
  ledcSetup(8, 50, 16); // 50 hz PWM, 16-bit resolution, range from 3250 to 6500.
  ledcAttachPin(ServoPin, 8);
}

void setup(){
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // prevent brownouts by silencing them

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();


  // Remote Control Car
  initMotors();
  initServo();
  initUltrasonic(); // Inicializar el sensor ultrasónico

  ledcSetup(7, 5000, 8);
  ledcAttachPin(4, 7);  //pin4 is LED

  WiFi.softAP(ssid, password);
  IPAddress miIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(miIP); //probar 192.168.4.1

  startWebServer();

  for (int i = 0; i < 5; i++) {
    ledcWrite(7, 10); // flash led
    delay(50);
    ledcWrite(7, 0);
    delay(50);
  }
}

void loop() {
  if (currentMode == MODE_REMOTE) {
    // La lógica de control remoto se maneja principalmente por HTTP en app_httpd.cpp
    // Aquí podríamos añadir un pequeño delay para evitar un loop muy rápido si no hay actividad
    delay(100);
    Serial.printf("Modo Control Remoto - RSSi: %ld dBm\n", WiFi.RSSI());
  } else { // MODE_AUTO
    // Lógica del modo automático (FinderBot)
    Serial.println("Modo Automatico - Ejecutando FinderBot...");
    long distance = readUltrasonicDistance();
    Serial.printf("Distancia: %ld cm\n", distance);

    // Lógica básica: si detecta un obstáculo cerca, gira; de lo contrario, avanza
    if (distance > 0 && distance < 20) { // Obstáculo a menos de 20 cm
      Serial.println("Obstaculo detectado. Girando...");
      stopMotors();
      delay(200);
      turnRight(); // Opciones: turnLeft, turnRight, moveBackward
      delay(500); // Gira por 0.5 segundos
      stopMotors();
      delay(200);
      moveForward(); // Intenta avanzar un poco después de girar
      delay(500);
    } else {
      moveForward(); // Avanza si no hay obstáculos
    }
    delay(100); // Pequeña pausa para evitar un loop muy rápido
  }
}

// Implementación de funciones de movimiento (extraídas de app_httpd.cpp)
void moveForward() {
  Serial.println("Forward");
  actstate = fwd;
  ledcWrite(4,speed);  // pin 12
  ledcWrite(3,0);      // pin 13
  ledcWrite(5,speed);  // pin 14
  ledcWrite(6,0);      // pin 15
  delay(200);
}

void turnLeft() {
  Serial.println("TurnLeft");
  // Implementación original de turnLeft
  ledcWrite(3,0);
  ledcWrite(5,0);
  ledcWrite(4,speed);
  ledcWrite(6,speed);
  delay(100);
  ledcWrite(4,0);
  ledcWrite(6,0);
}

void stopMotors() {
  Serial.println("Stop");
  actstate = stp;
  ledcWrite(4,0);
  ledcWrite(3,0);
  ledcWrite(5,0);
  ledcWrite(6,0);
}

void turnRight() {
  Serial.println("TurnRight");
  // Implementación original de turnRight
  ledcWrite(4,0);
  ledcWrite(6,0);
  ledcWrite(3,speed);
  ledcWrite(5,speed);
  delay(100);
  ledcWrite(3, 0);
  ledcWrite(5, 0);
}

void moveBackward() {
  Serial.println("Backward");
  actstate = rev;
  ledcWrite(4,0);
  ledcWrite(3,speed);
  ledcWrite(5,0);
  ledcWrite(6,speed);
  delay(200);
}

// Implementación de funciones para el sensor ultrasónico
void initUltrasonic() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

long readUltrasonicDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH);
  long distanceCm = duration * 0.034 / 2; // Velocidad del sonido en cm/microsegundo
  return distanceCm;
}
