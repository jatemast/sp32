#include <Arduino.h> // Necesario para funciones básicas de Arduino y ESP32 (incluye ledc)
#include <driver/ledc.h> // Incluir para funciones ledc (PWM)
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
#define MOTOR_A_IN1  21 // Izquierdo Adelante
#define MOTOR_A_IN2  22 // Izquierdo Atrás
#define MOTOR_A_PWM  23 // PWM para Motor Izquierdo

#define MOTOR_B_IN1  25 // Derecho Adelante (corresponde a IN3 en TB6612FNG)
#define MOTOR_B_IN2  26 // Derecho Atrás (corresponde a IN4 en TB6612FNG)
#define MOTOR_B_PWM  27 // PWM para Motor Derecho

#define MOTOR_STBY   32 // Standby del TB6612FNG

// Canales PWM
#define MOTOR_A_PWM_CHANNEL 0
#define MOTOR_B_PWM_CHANNEL 1
#define PWM_FREQ            5000
#define PWM_RESOLUTION      8

// ============================
// FUNCIONES DE MOVIMIENTO
// ============================

void setMotorSpeed(int motor_pwm_channel, int motor_speed) {
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)motor_pwm_channel, motor_speed);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)motor_pwm_channel);
}

void stopMotors() {
    digitalWrite(MOTOR_STBY, LOW); // Desactivar STBY para detener ambos motores
    digitalWrite(MOTOR_A_IN1, LOW);
    digitalWrite(MOTOR_A_IN2, LOW);
    digitalWrite(MOTOR_B_IN1, LOW);
    digitalWrite(MOTOR_B_IN2, LOW);
    setMotorSpeed(MOTOR_A_PWM_CHANNEL, 0);
    setMotorSpeed(MOTOR_B_PWM_CHANNEL, 0);
    actstate = stp;
}

void moveForward() {
    digitalWrite(MOTOR_STBY, HIGH); // Activar STBY
    digitalWrite(MOTOR_A_IN1, HIGH);
    digitalWrite(MOTOR_A_IN2, LOW);
    digitalWrite(MOTOR_B_IN1, HIGH);
    digitalWrite(MOTOR_B_IN2, LOW);
    setMotorSpeed(MOTOR_A_PWM_CHANNEL, speed);
    setMotorSpeed(MOTOR_B_PWM_CHANNEL, speed);
    actstate = fwd;
}

void moveBackward() {
    digitalWrite(MOTOR_STBY, HIGH); // Activar STBY
    digitalWrite(MOTOR_A_IN1, LOW);
    digitalWrite(MOTOR_A_IN2, HIGH);
    digitalWrite(MOTOR_B_IN1, LOW);
    digitalWrite(MOTOR_B_IN2, HIGH);
    setMotorSpeed(MOTOR_A_PWM_CHANNEL, speed);
    setMotorSpeed(MOTOR_B_PWM_CHANNEL, speed);
    actstate = rev;
}

void turnLeft() {
    digitalWrite(MOTOR_STBY, HIGH); // Activar STBY
    digitalWrite(MOTOR_A_IN1, LOW);  // Motor izquierdo hacia atrás
    digitalWrite(MOTOR_A_IN2, HIGH);
    digitalWrite(MOTOR_B_IN1, HIGH); // Motor derecho hacia adelante
    digitalWrite(MOTOR_B_IN2, LOW);
    setMotorSpeed(MOTOR_A_PWM_CHANNEL, speed);
    setMotorSpeed(MOTOR_B_PWM_CHANNEL, speed);
}

void turnRight() {
    digitalWrite(MOTOR_STBY, HIGH); // Activar STBY
    digitalWrite(MOTOR_A_IN1, HIGH); // Motor izquierdo hacia adelante
    digitalWrite(MOTOR_A_IN2, LOW);
    digitalWrite(MOTOR_B_IN1, LOW);  // Motor derecho hacia atrás
    digitalWrite(MOTOR_B_IN2, HIGH);
    setMotorSpeed(MOTOR_A_PWM_CHANNEL, speed);
    setMotorSpeed(MOTOR_B_PWM_CHANNEL, speed);
}

// ============================
// SETUP
// ============================

void setup() {
    Serial.begin(115200);
    delay(500);

    pinMode(MOTOR_A_IN1, OUTPUT);
    pinMode(MOTOR_A_IN2, OUTPUT);
    pinMode(MOTOR_B_IN1, OUTPUT);
    pinMode(MOTOR_B_IN2, OUTPUT);
    pinMode(MOTOR_STBY, OUTPUT);

    // Configurar temporizador LEDC
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = (ledc_timer_bit_t)PWM_RESOLUTION,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = PWM_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer);

    // Configurar canales PWM
    ledc_channel_config_t channelA = {
        .gpio_num = MOTOR_A_PWM,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = (ledc_channel_t)MOTOR_A_PWM_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0
    };
    ledc_channel_config(&channelA);

    ledc_channel_config_t channelB = {
        .gpio_num = MOTOR_B_PWM,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = (ledc_channel_t)MOTOR_B_PWM_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0
    };
    ledc_channel_config(&channelB);

    stopMotors();

    // Conexión WiFi
    WiFi.begin("findebot", "contraseña123456789");
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
