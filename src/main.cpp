#define BLYNK_TEMPLATE_ID "Your_Template_ID"
#define BLYNK_TEMPLATE_NAME "Your_Project_Name"
#define BLYNK_AUTH_TOKEN "Your_Auth_Token"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "EggRoll";
char pass[] = "talian64";

// PIN DEFINITIONS
#define PIR_PIN D1
#define BUZZER_PIN D2
#define BLUE_LED D6
#define GREEN_LED D7
#define SERVO_PIN D4

Servo myServo;
BlynkTimer timer;

bool motionDetected = false;
int servoAngle = 90; // Initial servo position
void checkMotion();
void setup() {
  Serial.begin(9600);
  
  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BLUE_LED, LOW);

  myServo.attach(SERVO_PIN);
  myServo.write(servoAngle);

  Blynk.begin(auth, ssid, pass);

  timer.setInterval(1000L, checkMotion); // Check motion every 1 sec
}

// Blink green LED and beep buzzer when motion is detected
void checkMotion() {
  int motion = digitalRead(PIR_PIN);
  if (motion == HIGH) {
    Serial.println("Motion detected!");
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(GREEN_LED, LOW);

    Blynk.virtualWrite(V0, "Motion Detected");
  } else {
    Blynk.virtualWrite(V0, "No Motion");
  }
}

// Servo control from Blynk app button or slider on V1
BLYNK_WRITE(V1) {
  servoAngle = param.asInt();  // Read angle from app
  myServo.write(servoAngle);
  Serial.print("Servo moved to: ");
  Serial.println(servoAngle);
}

void loop() {
  Blynk.run();
  timer.run();

  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(BLUE_LED, HIGH); // Blue LED on when connected
  } else {
    digitalWrite(BLUE_LED, LOW);
  }
}
