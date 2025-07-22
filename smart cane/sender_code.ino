#include <SPI.h>
#include <MFRC522.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// #define ACCEL_THRESHOLD 1.2
// #define INACTIVITY_TIMEOUT 60000  // 1 minute
// #define GRAVITY 9.81
// #define MOVEMENT_DEADZONE 0.05

Adafruit_MPU6050 mpu;
const long vibrationDuration = 5000;
const long inactivityTimeout = 60000;
const float movementThreshold = 0.5;
unsigned long lastMovementTime = 0;
unsigned long vibrationStartTime = 0;
bool isVibrating = false;
bool inactivityAlertSent = false;
float last_ax, last_ay, last_az;

#define TRIG_PIN 12
#define ECHO_PIN 13

#define VIBRATION_PIN 32
#define BUZZER_PIN 14

#define RST_PIN 22
#define SS_PIN  21
MFRC522 rfid(21, 22);

// Define 4 known UID tags
byte UID1[] = {0x39, 0x15, 0xB4, 0x98}; // Example
byte UID2[] = {0xD9, 0x12, 0x9F, 0x98};
byte UID3[] = {0xAD, 0x50, 0x83, 0x21};
byte UID4[] = {0x96, 0x2D, 0x05, 0x02};

uint8_t receiverAddress[] = {0x68, 0x25, 0xDD, 0x46, 0xFA, 0x44}; // Replace with your receiver MAC

typedef struct struct_message {
  byte uid[4];
} struct_message;

struct_message msg;

long getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  long distance = duration * 0.034 / 2;
  return distance;
}

  void buzzPattern(long distance){
    if (distance<20){
      digitalWrite(VIBRATION_PIN, HIGH);
      tone(BUZZER_PIN, HIGH);
    }
    else if (distance<40){
      digitalWrite(VIBRATION_PIN, LOW);
      tone(BUZZER_PIN, 2000); 
      delay(200);  
      noTone(BUZZER_PIN);
      delay(100);   
    } else if(distance>=40 && distance < 80){
      tone(BUZZER_PIN, 1000);
      delay(300);    
      noTone(BUZZER_PIN);
      delay(200);
    } else{
      noTone(BUZZER_PIN);
      digitalWrite(VIBRATION_PIN, LOW);
    }
  }

// void activateFeedback(int duration_ms) {
//   digitalWrite(VIBRATION_PIN, HIGH);
//   feedbackStartTime = millis();
//   feedbackActive = true;
// }

// void stopFeedback() {
//   digitalWrite(VIBRATION_PIN, LOW);
//   feedbackActive = false;
// }



void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(VIBRATION_PIN, OUTPUT);

  digitalWrite(VIBRATION_PIN, LOW);

  Wire.begin(27, 26); // SDA, SCL
  if (!mpu.begin()) {
   Serial.println("MPU6050 not found!");
     while (1){
       delay(10);
   }
   }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  

  // mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  // mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);

  // lastMovementTime = millis(); // Start activity tracking
  // Serial.println("MPU6050 ready!");

   sensors_event_t a, g, temp;
   mpu.getEvent(&a, &g, &temp);
  

  last_ax = a.acceleration.x;
  last_ay = a.acceleration.y;
  last_az = a.acceleration.z;
  lastMovementTime = millis();
  Serial.println("Smart Cane Inactivity initiated");

  SPI.begin();
  rfid.PCD_Init();
  WiFi.mode(WIFI_STA);
  Serial.print("Sender MAC: "); Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed!");
    return;
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

bool compareUID(byte *a, byte *b) {
  for (int i = 0; i < 4; i++) {
    if (a[i] != b[i]) return false;
  }
  return true;
}

void loop() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    byte *uid = rfid.uid.uidByte;
    Serial.print("UID: ");
    for (byte i = 0; i < rfid.uid.size; i++) {
      Serial.print(uid[i], HEX); Serial.print(" ");
      msg.uid[i] = uid[i];
    }
    Serial.println();

    esp_now_send(receiverAddress, (uint8_t *)&msg, sizeof(msg));
  }

    long dist = getDistance(TRIG_PIN, ECHO_PIN);
    Serial.print("Sensor distance: ");
    Serial.println(dist);
    buzzPattern(dist);

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float delta_ax = a.acceleration.x - last_ax;
  float delta_ay = a.acceleration.y - last_ay;
  float delta_az = a.acceleration.z - last_az;
  float totalChange = sqrt(sq(delta_ax) + sq(delta_ay) + sq(delta_az));

  last_ax = a.acceleration.x;
  last_ay = a.acceleration.y;
  last_az = a.acceleration.z;

  // // If movement is detected, reset the inactivity timer
  if (totalChange > movementThreshold) {
    lastMovementTime = millis();
       // If it was vibrating due to inactivity, stop it immediately upon movement
     if (isVibrating) {
         digitalWrite(VIBRATION_PIN, LOW);
         isVibrating = false;
         Serial.println("Movement detected, stopping inactivity alert.");
     }
  }

  // // Check for inactivity IF NOT ALREADY VIBRATING
   if (!isVibrating && (millis() - lastMovementTime > inactivityTimeout)) {
     Serial.println("Inactivity detected! Starting 5-second vibration.");
     digitalWrite(VIBRATION_PIN, HIGH);
     isVibrating = true;
     vibrationStartTime = millis();
   }

  // // Check if the 5-second vibration duration is over
   if (isVibrating && (millis() - vibrationStartTime > vibrationDuration)) {
     Serial.println("Vibration stopped.");
     digitalWrite(VIBRATION_PIN, LOW);
     isVibrating = false;
    
  //   // FIX: Reset the inactivity timer AFTER the vibration has finished.
  //   // This starts the 30-second countdown for the NEXT check.
     lastMovementTime = millis();
   }

  delay(500); 
}




