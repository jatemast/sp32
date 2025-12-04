#include <WiFi.h>
#include "app_httpd.h"

// ===========================
// DEFINICIÓN DE VARIABLES GLOBALES
// ===========================

// Modo del robot
RobotMode currentMode = MODE_REMOTE;

// Variables de control
int speed = 255;
int noStop = 0;

// Estado del movimiento
state actstate = stp;

// ===========================
// CONFIGURACIÓN DE PINES DEL MOTOR
// ===========================
#define MOTOR_L_FWD  12
#define MOTOR_L_REV  13
#define MOTOR_R_FWD  14
#define MOTOR_R_REV  15

// ============================
// FUNCIONES DE MOVIMIENTO
// ============================

void stopMotors() {
    digitalWrite(MOTOR_L_FWD, LOW);
    digitalWrite(MOTOR_L_REV, LOW);
    digitalWrite(MOTOR_R_FWD, LOW);
    digitalWrite(MOTOR_R_REV, LOW);
    actstate = stp;
}

void moveForward() {
    digitalWrite(MOTOR_L_FWD, HIGH);
    digitalWrite(MOTOR_L_REV, LOW);
    digitalWrite(MOTOR_R_FWD, HIGH);
    digitalWrite(MOTOR_R_REV, LOW);
    actstate = fwd;
}

void moveBackward() {
    digitalWrite(MOTOR_L_FWD, LOW);
    digitalWrite(MOTOR_L_REV, HIGH);
    digitalWrite(MOTOR_R_FWD, LOW);
    digitalWrite(MOTOR_R_REV, HIGH);
    actstate = rev;
}

void turnLeft() {
    digitalWrite(MOTOR_L_FWD, LOW);
    digitalWrite(MOTOR_L_REV, HIGH);
    digitalWrite(MOTOR_R_FWD, HIGH);
    digitalWrite(MOTOR_R_REV, LOW);
}

void turnRight() {
    digitalWrite(MOTOR_L_FWD, HIGH);
    digitalWrite(MOTOR_L_REV, LOW);
    digitalWrite(MOTOR_R_FWD, LOW);
    digitalWrite(MOTOR_R_REV, HIGH);
}

// ============================
// SETUP
// ============================

void setup() {
    Serial.begin(115200);
    delay(500);

    pinMode(MOTOR_L_FWD, OUTPUT);
    pinMode(MOTOR_L_REV, OUTPUT);
    pinMode(MOTOR_R_FWD, OUTPUT);
    pinMode(MOTOR_R_REV, OUTPUT);

    stopMotors();

    // Conexión WiFi
    WiFi.begin("TU_WIFI", "TU_PASSWORD");
    Serial.print("Conectando a WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.print("IP del robot: ");
    Serial.println(WiFi.localIP());

    // Iniciar servidor web
    startWebServer();
}

// ============================
// LOOP PRINCIPAL
// ============================
void loop() {

    // Si está en modo automático
    if (currentMode == MODE_AUTO) {

        // Aquí puedes poner tu lógica del sensor ultrasónico,
        // detector de línea, etc.

        // EJEMPLO (no real):
        // if (obstaculo_detectado) moveForward();
        // if (linea_detectada) stopMotors();
    }

    delay(20);
}
