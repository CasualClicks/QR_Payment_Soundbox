#include <Arduino.h>
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <HardwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// ================= CONFIGURATION =================
const char apn[]      = "www"; // VI APN (Alternative: "smartnet")
const char gprsUser[] = "";
const char gprsPass[] = "";

const char* broker    = "io.adafruit.com";
const char* topic     = "OUR_ADAFRUIT_TOPIC";
const char* mqtt_user = "YOUR_ADAFRUIT_USER";
const char* mqtt_pass = "YOUR_ADAFRUIT_KEY";

// ================= PINS =================
#define SIM_TX 17
#define SIM_RX 16
#define DF_TX  25
#define DF_RX  26
#define LED_RED 33   // RGB Red
#define LED_GRN 32   // RGB Green
#define LED_BLU 35   // RGB Blue

// ================= OBJECTS =================
HardwareSerial SerialAT(1);
HardwareSerial mp3Serial(2);
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient mqtt(client);
DFRobotDFPlayerMini df;

// ================= AUDIO QUEUE =================
int queue[30];
int queueSize = 0, currentIndex = 0;
bool isPlaying = false;
unsigned long lastCommandTime = 0;

void setLED(int r, int g, int b) {
  analogWrite(LED_RED, r);
  analogWrite(LED_GRN, g);
  analogWrite(LED_BLU, b);
}

void addToQueue(int folder, int file) {
  if (queueSize < 30) queue[queueSize++] = (folder * 1000) + file;
}

void playNext() {
  if (currentIndex >= queueSize) {
    isPlaying = false;
    queueSize = 0; currentIndex = 0;
    return;
  }
  int combined = queue[currentIndex++];
  df.playFolder(combined / 1000, combined % 1000);
  lastCommandTime = millis();
  isPlaying = true;
}

void queueNumber(int num) {
  if (num == 0) return;
  if (num <= 20) { addToQueue(1, num); } 
  else if (num < 100) {
    addToQueue(1, (num / 10) + 18); 
    if (num % 10) queueNumber(num % 10);
  } 
  else if (num < 1000) {
    addToQueue(1, num / 100); addToQueue(1, 28);
    if (num % 100) queueNumber(num % 100);
  }
  else if (num < 100000) {
    queueNumber(num / 1000); addToQueue(1, 29);
    if (num % 1000) queueNumber(num % 1000);
  }
}

void speakAmount(String amount) {
  int val = amount.toInt();
  if (val <= 0) return;
  queueSize = 0; currentIndex = 0;
  setLED(0, 255, 255); // Cyan for processing
  addToQueue(3, 4); addToQueue(3, 5);
  queueNumber(val);
  addToQueue(3, 2);
  playNext();
}

// ================= MQTT CALLBACK =================
void callback(char* topic, byte* payload, unsigned int len) {
  char message[len + 1];
  memcpy(message, payload, len);
  message[len] = '\0';
  speakAmount(String(message));
}

void connectMQTT() {
  while (!mqtt.connected()) {
    setLED(0, 0, 255); // Blue: Attempting MQTT
    String clientId = "ESP32-Box-" + String(random(0, 999));
    if (mqtt.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      mqtt.subscribe(topic);
      setLED(0, 255, 0); // Green: Ready
    } else {
      setLED(255, 0, 0); // Red: Failed
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("System Booting Up");
  pinMode(LED_RED, OUTPUT); pinMode(LED_GRN, OUTPUT); pinMode(LED_BLU, OUTPUT);
  
  setLED(255, 255, 0); // Yellow: Booting

  mp3Serial.begin(9600, SERIAL_8N1, DF_RX, DF_TX);
  if (!df.begin(mp3Serial)) while(true);
  df.volume(28);

  // SIM800L Setup
  SerialAT.begin(115200, SERIAL_8N1, SIM_RX, SIM_TX);
  delay(3000);
  Serial.println("Initializing modem...");
  modem.restart();

modem.sendAT("+CNETLIGHT=0"); 
Serial.println("Netlight disabled to save power.");

  Serial.println("Waiting for network...");
  if (!modem.waitForNetwork()) {
    setLED(255, 0, 0);
    while(true);
  }

  Serial.println("Connecting GPRS...");
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    setLED(255, 0, 0);
    while(true);
  }

  mqtt.setServer(broker, 1883);
  mqtt.setCallback(callback);
}

void loop() {
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  if (isPlaying && (millis() - lastCommandTime > 600)) { 
    if (df.available() && df.readType() == DFPlayerPlayFinished) {
      playNext();
      if (!isPlaying) setLED(0, 255, 0); // Back to Green after speaking
    }
  }
}