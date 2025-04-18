#define BLYNK_TEMPLATE_ID "TMPL2GqdIuQgD"
#define BLYNK_TEMPLATE_NAME "PhysicalIDS"
#define BLYNK_AUTH_TOKEN "eDUr6N1j893Twp7BYnJ-9foNdWbRcG5O"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>


// WiFi credentials
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Reindeer";
char pass[] = "200120022003";
const char* serverIP = "http://192.168.10.103:5000";

// Pin definitions (NodeMCU)
#define PIR_PIN      5   // D1
#define BUZZER_PIN   4   // D2
#define BLUE_LED    12   // D6 - WiFi status
#define GREEN_LED   13   // D7 - Motion indicator
#define SERVO_PIN    2  // D4

// Globals
Servo myServo;
BlynkTimer timer;
int servoAngle = 90;  // Initial servo angle


// Function declarations
void checkMotion();
void sendChatbotAlert(const String& message);

// ==================== MOVE SERVO ====================
void moveServo(int angle) {
  if (angle >= 0 && angle <= 180) {
    myServo.write(angle);  // Move servo to specified angle
    Serial.print("Servo moved to: ");
    Serial.println(angle);
  } else {
    Serial.println("Invalid angle. Please use an angle between 0 and 180.");
  }
}

void getCommandFromServer() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClient client;
    
    http.begin(client, serverIP + String("/command"));  // Flask endpoint to get pending command
    int httpResponseCode = http.GET();  // Send GET request to fetch command
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Received response: " + response);

      // Parse JSON
      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, response);

      if (!error) {
        String command = doc["command"];
        Serial.println("Parsed command: " + command);

        if (command.startsWith("servo")) {
          int angle = command.substring(command.indexOf(" ") + 1).toInt();
          moveServo(angle);
        }
      } else {
        Serial.println("❌ Failed to parse JSON");
      }
    }

    http.end();  // Close the HTTP connection
  } else {
    Serial.println("WiFi not connected, cannot fetch command");
  }
}
// ==================== SETUP ====================
void setup() {
  Serial.begin(9600);

  // Pin modes
  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  // Initialize outputs
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BLUE_LED, LOW);

  // Attach servo
  myServo.attach(SERVO_PIN);
  myServo.write(servoAngle);

  // Start Blynk and setup periodic motion check
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(1000L, checkMotion);  // Run every 1 second
}

// ==================== LOOP ====================
void loop() {
  Blynk.run();
  timer.run();
  getCommandFromServer();
  delay(1000);

  // Blue LED shows WiFi status
  digitalWrite(BLUE_LED, WiFi.status() == WL_CONNECTED ? HIGH : LOW);
}

// ==================== MOTION CHECK ====================
void checkMotion() {
  int motion = digitalRead(PIR_PIN);

  if (motion == HIGH) {
    Serial.println("🚨 Motion detected!");
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);  // Buzz briefly
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(GREEN_LED, LOW);

    // Update Blynk and notify chatbot
    Blynk.virtualWrite(V0, "Motion Detected");
    sendChatbotAlert("motion detected");
  } else {
    Blynk.virtualWrite(V0, "No Motion");
  }
}

// ==================== SERVO CONTROL ====================
BLYNK_WRITE(V1) {
  servoAngle = param.asInt();  // Read angle from app
  myServo.write(servoAngle);
  Serial.print("🦾 Servo moved to: ");
  Serial.println(servoAngle);
}

// ==================== SEND TO CHATBOT ====================
void sendChatbotAlert(const String& message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClient client;

    http.begin(client, "http://192.168.10.103:5000/chat");  // 🛠️ Replace with your Flask server IP
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"message\": \"" + message + "\"}";
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      Serial.println("✅ Alert sent to chatbot!");
      Serial.println("Response: " + http.getString());
    } else {
      Serial.print("❌ Error sending alert. HTTP code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("⚠️ WiFi not connected, cannot send alert.");
  }
}
