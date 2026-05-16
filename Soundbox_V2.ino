#include <Arduino.h>
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <HardwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// ================= CONFIG =================
const char apn[]      = "www";   // VI APN
const char gprsUser[] = "";
const char gprsPass[] = "";

const char* broker    = "io.adafruit.com";
const char* topic     = "OUR_ADAFRUIT_TOPIC";
const char* mqtt_user = "YOUR_ADAFRUIT_USER";
const char* mqtt_pass = "YOUR_ADAFRUIT_KEY";

bool isFirstConnect = true; 
// ================= PINS =================
#define ESP_RX_SIM_TX 4 
#define ESP_TX_SIM_RX 5 
#define ESP_RX_DF_TX  27
#define ESP_TX_DF_RX  25

// ================= AUDIO MAP =================
// Folders
#define F_NUM    1
#define F_SYS    2
#define F_WORDS  3
#define F_ERR    4
#define F_PAUSE  99 // MAGIC FOLDER: Used to create silent gaps

// System Sounds (Folder 02)
#define S_BOOTING  1
#define S_NETWORK  2
#define S_DING     3
#define S_READY    4

// Word Sounds (Folder 03)
#define W_PAISE    1
#define W_RUPEES   2
#define W_AND      3
#define W_AMOUNT   4
#define W_RECEIVED 5

// Error Sounds (Folder 04)
#define E_NO_SRV   1
#define E_NO_PAY   2
#define E_ERROR    3

// ================= OBJECTS =================
HardwareSerial SerialAT(1);
HardwareSerial mp3Serial(2);
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient mqtt(client);
DFRobotDFPlayerMini df;

// ================= AUDIO QUEUE =================
int queue[40]; 
int queueSize = 0, currentIndex = 0;

bool isPlaying = false;
unsigned long lastCommandTime = 0;

// Pause Variables
bool isPausing = false;
unsigned long pauseEndTime = 0;

void addToQueue(int folder, int file) {
  if (queueSize < 40) queue[queueSize++] = (folder * 10000) + file;
}

void playNext() {
  if (currentIndex >= queueSize) {
    isPlaying = false;
    isPausing = false;
    queueSize = 0;
    currentIndex = 0;
    Serial.println("[INFO] Audio queue finished");
    return;
  }

  int val = queue[currentIndex++];
  int folder = val / 10000;
  int file = val % 10000;

  if (folder == F_PAUSE) {
    Serial.printf("[INFO] Silent Pause for %d ms...\n", file);
    pauseEndTime = millis() + file;
    isPausing = true;
    isPlaying = false;
  } 
  else {
    Serial.printf("[INFO] Playing -> Folder: %02d | File: %03d\n", folder, file);
    df.playFolder(folder, file);
    lastCommandTime = millis();
    isPlaying = true;
    isPausing = false;
  }
}

void queueNumber(long num) {
  if (num == 0) return;

  if (num <= 20) {
    addToQueue(F_NUM, num);
  } 
  else if (num < 100) {
    addToQueue(F_NUM, (num / 10) + 18); 
    if (num % 10) queueNumber(num % 10);
  } 
  else if (num < 1000) {
    queueNumber(num / 100);
    addToQueue(F_NUM, 28); 
    if (num % 100) queueNumber(num % 100);
  }
  else if (num < 100000) {
    queueNumber(num / 1000);
    addToQueue(F_NUM, 29); 
    if (num % 1000) queueNumber(num % 1000);
  }
  else {
    queueNumber(num / 100000);
    addToQueue(F_NUM, 30); 
    if (num % 100000) queueNumber(num % 100000);
  }
}

// ==== UPGRADED PARSER FOR PAISE ====
void speakAmount(String amount) {
  amount.trim();
  if (amount.length() == 0) return;

  long rupeesVal = 0;
  long paiseVal = 0;

  // Check if there is a decimal point
  int dotIndex = amount.indexOf('.');
  
  if (dotIndex != -1) {
    // Extract the Rupees part before the dot
    rupeesVal = amount.substring(0, dotIndex).toInt();
    
    // Extract the Paise part after the dot
    String pStr = amount.substring(dotIndex + 1);
    
    // Safety check: if they send "150.5" it means 50 paise.
    if (pStr.length() == 1) {
      pStr += "0"; 
    } else if (pStr.length() > 2) {
      pStr = pStr.substring(0, 2); // Ignore anything past 2 decimals
    }
    paiseVal = pStr.toInt();
  } else {
    // No decimal found, it's just pure rupees
    rupeesVal = amount.toInt();
  }

  // If amount is literally 0.00, ignore it
  if (rupeesVal <= 0 && paiseVal <= 0) return;

  Serial.println("\n[EVENT] Processing Payment: ₹" + amount);
  Serial.printf("[INFO] Parsed -> Rupees: %ld | Paise: %ld\n", rupeesVal, paiseVal);

  queueSize = 0;
  currentIndex = 0;

  // ==== THE PACING SEQUENCE ====
  addToQueue(F_SYS, S_DING);       // 1. Play the Ding
  addToQueue(F_PAUSE, 500);        // 2. Pause
  
  addToQueue(F_WORDS, W_AMOUNT);   // 3. "Amount"
  addToQueue(F_WORDS, W_RECEIVED); // 4. "Received"
  
  // 5. Speak Rupees (if any)
  if (rupeesVal > 0) {
    queueNumber(rupeesVal);        
    addToQueue(F_WORDS, W_RUPEES); // "Rupees"
  }

  // 6. Speak Paise (if any)
  if (paiseVal > 0) {
    // If we already said rupees, insert the word "And"
    if (rupeesVal > 0) {
      addToQueue(F_WORDS, W_AND);  
    }
    queueNumber(paiseVal);         // Speak the decimal numbers
    addToQueue(F_WORDS, W_PAISE);  // "Paise"
  }

  playNext(); // Kick off the queue
}

// ================= MQTT =================
void callback(char* topic, byte* payload, unsigned int len) {
  char msg[len + 1];
  memcpy(msg, payload, len);
  msg[len] = '\0';

  Serial.println("[INFO] MQTT Message Received: " + String(msg));
  speakAmount(String(msg));
}

void connectMQTT() {
  String clientId = "SoundBox-" + String(micros());

  // Add a local counter to prevent the "No Service" sound from looping
  int retryCount = 0;

  while (!mqtt.connected()) {
    modem.sendAT("+CNETLIGHT=1"); 
    modem.waitResponse();

    Serial.println("[INFO] Connecting to MQTT Broker...");

    if (mqtt.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("[INFO] MQTT Connected Successfully!");
      mqtt.subscribe(topic);
      
      modem.sendAT("+CNETLIGHT=0"); 
      modem.waitResponse();
      
      // Only play "Ready" on the very first successful boot
      if (isFirstConnect) {
        df.playFolder(F_SYS, S_READY); 
        isFirstConnect = false; 
      }
      else{
      Serial.println("[INFO] MQTT Reconnected");      
      }
    } 
    else {
      Serial.print("[WARN] MQTT Failed, rc=");
      Serial.println(mqtt.state());
      
      // ONLY play "No Service" on the first failure. 
      // This prevents the box from shouting every 5 seconds.
      if (retryCount == 0 && isFirstConnect) {
         df.playFolder(F_ERR, E_NO_SRV); 
      }
      
      retryCount++;
      delay(5000); // Wait 5 seconds before trying again
    }
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  delay(3000);

  Serial.println("\n=================================");
  Serial.println("    SOUNDBOX BOOT SEQUENCE       ");
  Serial.println("=================================");
  randomSeed(micros());

  // --- DFPlayer Init ---
  Serial.println("[INIT] Starting DFPlayer...");
  mp3Serial.begin(9600, SERIAL_8N1, ESP_RX_DF_TX, ESP_TX_DF_RX);
  if (!df.begin(mp3Serial)) {
    Serial.println("[ERROR] DFPlayer missing or wiring faulty!");
  } else {
    Serial.println("[INFO] DFPlayer Ready. Volume set to 25.");
    df.volume(15);
    df.playFolder(F_SYS, S_BOOTING); 
  }

  // --- SIM800 Init ---
  Serial.println("[INIT] Starting SIM800L...");
  SerialAT.begin(9600, SERIAL_8N1, ESP_RX_SIM_TX, ESP_TX_SIM_RX);

  // FORCE NETLIGHT ON IMMEDIATELY
  // We send a raw AT command because 'modem.init()' hasn't passed yet
  Serial.println("[DEBUG] Forcing Netlight ON for troubleshooting...");
  SerialAT.println("AT+CNETLIGHT=0"); 
  delay(100);
  
  Serial.println("[INFO] Syncing Modem...");
  if (!modem.init()) {
    Serial.println("[ERROR] Modem init failed! Check power/wiring.");
    df.playFolder(F_ERR, E_ERROR); 
    while (true);
  }
  
  modem.sendAT("+CNETLIGHT=0"); 
  modem.waitResponse(); 
  Serial.println("[INFO] Netlight disabled.");
  
  Serial.println("[INFO] Waiting for cellular network...");
  if (!modem.waitForNetwork(60000L)) {
    Serial.println("[ERROR] No Network Signal.");
    df.playFolder(F_ERR, E_ERROR);
    while (true); 
  }

  Serial.println("[INFO] Cellular Network Found!");

  Serial.println("[INFO] Establishing GPRS (Internet)...");
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    Serial.println("[ERROR] GPRS Connection Failed.");
    df.playFolder(F_ERR, E_NO_SRV);
    while (true); 
  }

  Serial.println("[INFO] Internet Connection Established!");
  df.playFolder(F_SYS, S_NETWORK); 

  mqtt.setServer(broker, 1883);
  mqtt.setCallback(callback);

  mqtt.setBufferSize(512);   // Gives more room for cellular data packets
  mqtt.setKeepAlive(120);    // Tells Adafruit to wait 2 minutes before disconnecting


  Serial.println("[INFO] Setup Complete. Entering Loop.");
}

// ================= LOOP =================
void loop() {
  if (!mqtt.connected()) {
    Serial.print("[DEBUG] Connection lost. Internal State: ");
    Serial.println(mqtt.state()); // This will tell us WHY it dropped
    connectMQTT();
  }

  mqtt.loop();

  if (isPausing && (millis() >= pauseEndTime)) {
    playNext();
  }

  if (df.available()) {
    uint8_t type = df.readType();
    
    if (isPlaying && type == DFPlayerPlayFinished) {
      if (millis() - lastCommandTime > 150) {
        playNext();
      }
    }
  }
}