#include <esp_now.h>
#include <WiFi.h>
#include <DFRobotDFPlayerMini.h>
#include <HardwareSerial.h>

#define VIBRATION_PIN 14
#define TRIG_PIN_1 12
#define ECHO_PIN_1 13
#define TRIG_PIN_2 25
#define ECHO_PIN_2 26
#define BUZZER_PIN 27

HardwareSerial mySerial(1); // Use UART1 for DFPlayer (RX=32, TX=33)
DFRobotDFPlayerMini myDFPlayer;

typedef struct struct_message {
  byte uid[4];
} struct_message;

struct_message incomingData;

// Known UIDs
byte UID1[] = {0x39, 0x15, 0xB4, 0x98};
byte UID2[] = {0xD9, 0x12, 0x9F, 0x98};
byte UID3[] = {0xAD, 0x50, 0x83, 0x21};
byte UID4[] = {0x96, 0x2D, 0x05, 0x02};

// ultrasonic function
long getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH, 30000); // timeout in microseconds
  long distance = duration * 0.034 / 2;  // in cm
  return distance;
}
  //buzzer pattern feedback
  void buzzPattern(long distance1, long distance2) {
  long minDist = min(distance1, distance2);
   if (minDist<15){
    digitalWrite(VIBRATION_PIN, HIGH);
    delay(100);               // short ON pulse
    digitalWrite(VIBRATION_PIN, LOW);
    delay(200);  
    for (int i = 0; i < 5; i++) {         // Repeat beep 5 times (optional)
        tone(BUZZER_PIN, 800);            // Start sound at 800 Hz
        delay(200);                       // Beep ON for 200 ms
        noTone(BUZZER_PIN);               // Turn off sound
        delay(200);                       // Pause for 200 ms
    }
   }
   else if (minDist < 30) {  // else if (minDist>=15 && minDist<30)
    digitalWrite(VIBRATION_PIN, LOW);  // Turn on vibration
    for (int i = 0; i < 2; i++) {           // Two short beeps
        tone(BUZZER_PIN, 600);              // Lower frequency for less urgency
        delay(100);                         // Beep ON 100ms
        noTone(BUZZER_PIN);
        delay(100);                         // Beep OFF 100ms
    } 
  } else if (minDist < 60) {
    // Medium close: short pulses
    digitalWrite(VIBRATION_PIN, LOW);   // No vibration
    tone(BUZZER_PIN, 1000);
    delay(300);
    noTone(BUZZER_PIN);
    delay(200);
  }else {
    // No obstacle
    noTone(BUZZER_PIN);
    digitalWrite(VIBRATION_PIN, LOW);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN_1, OUTPUT);
  pinMode(ECHO_PIN_1, INPUT);
  pinMode(TRIG_PIN_2, OUTPUT);
  pinMode(ECHO_PIN_2, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  // Start DFPlayer serial (RX=32, TX=33)
  mySerial.begin(9600, SERIAL_8N1, 32, 33);

  pinMode(VIBRATION_PIN, OUTPUT);

  WiFi.mode(WIFI_STA);
  Serial.print("Receiver MAC: ");
  Serial.println(WiFi.macAddress());

  if (!myDFPlayer.begin(mySerial)) {
    Serial.println("❌ DFPlayer not responding!");
    while (true); // halt
  }

  myDFPlayer.volume(15); // Set volume (0~30)
  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ ESP-NOW init failed!");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
}

bool compareUID(byte *a, byte *b) {
  for (int i = 0; i < 4; i++) {
    if (a[i] != b[i]) return false;
  }
  return true;
}

void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingDataBytes, int len) {
  memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));
  byte *uid = incomingData.uid;

  Serial.print("Received UID: ");
  for (int i = 0; i < 4; i++) {
    Serial.print(uid[i], HEX); Serial.print(" ");
  }
  Serial.println();

  int track = 0;

  if (compareUID(uid, UID1)) track = 1;
  else if (compareUID(uid, UID2)) track = 2;
  else if (compareUID(uid, UID3)) track = 3;
  else if (compareUID(uid, UID4)) track = 4;
  else {
    Serial.println("⚠️ Unknown UID received");
    return;
  }

  // 🟢 Play the corresponding track and vibrate
  Serial.print("🎵 Playing track: "); Serial.println(track);
  myDFPlayer.play(track);
  
  digitalWrite(VIBRATION_PIN, HIGH);
  delay(500);  // Vibration duration
  digitalWrite(VIBRATION_PIN, LOW);
  
}


void loop(){
  long dist1 = getDistance(TRIG_PIN_1, ECHO_PIN_1);
  long dist2 = getDistance(TRIG_PIN_2, ECHO_PIN_2);

  Serial.print("Sensor 1 Distance: "); Serial.println(dist1);
  Serial.print("Sensor 2 Distance: "); Serial.println(dist2);

  // Choose sensor with the closest reading
  long minDist = min(dist1, dist2);
  buzzPattern(dist1, dist2);
  delay(500);
  
}

