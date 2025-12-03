const char* ssid = "limpiador";
const char* password = "123456789";

#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "app_httpd.cpp" // Incluir el archivo para la declaración de startWebServer()



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
  // put your main code here, to run repeatedly:
  delay(1000);
  Serial.printf("RSSi: %ld dBm\n", WiFi.RSSI());
}
