#include <DFRobotDFPlayerMini.h>
#include <HardwareSerial.h>

HardwareSerial mp3Serial(2);
DFRobotDFPlayerMini df;

unsigned long lastCheckTime = 0;
const int checkInterval = 2000; // Check connection every 2 seconds

void setup() {
  Serial.begin(115200);
  
  // Start DFPlayer serial (RX = 27, TX = 25)
  // Pin 27 on ESP32 <--- TX on DFPlayer
  // Pin 25 on ESP32 ---> RX on DFPlayer (through 1k resistor)
  mp3Serial.begin(9600, SERIAL_8N1, 27, 25);

  Serial.println("\n[STABILITY TEST] Initializing DFPlayer...");

  if (!df.begin(mp3Serial)) {
    Serial.println("[CRITICAL] DFPlayer not found at boot!");
    while (true); 
  }

  Serial.println("[OK] Initial Connection Successful.");
  df.volume(25);
  delay(500);

  // Start playing a long file to stress the power supply
  Serial.println("[STRESS] Starting playback...");
  df.playFolder(2, 1); 
}

void loop() {
  // We use a non-blocking timer to poll the device
  if (millis() - lastCheckTime >= checkInterval) {
    lastCheckTime = millis();

    Serial.print("[CHECK] Pinging DFPlayer... ");
    
    // Attempt to read current volume as a "Heartbeat"
    int vol = df.readVolume();

    if (vol == -1) {
      Serial.println("FAILED! Connection dropped or Chip Reset.");
      // If it fails, try to re-initialize or play an error sound
    } else {
      Serial.print("STABLE. Volume is: ");
      Serial.println(vol);
    }
  }

  // Check for any hardware-level errors sent by the chip
  if (df.available()) {
    uint8_t type = df.readType();
    int value = df.read();
    
    if (type == DFPlayerError) {
      Serial.print("[HARDWARE ERROR] Code: ");
      Serial.println(value);
      // Code 1: Busy, 2: Sleeping, 3: Serial Wrong, 4: Checksum, 5: Out of range
    }
  }
}