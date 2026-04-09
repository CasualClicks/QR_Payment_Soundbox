#include <DFRobotDFPlayerMini.h>
#include <HardwareSerial.h>

HardwareSerial mp3Serial(2);
DFRobotDFPlayerMini df;

void setup() {
  Serial.begin(115200);

  // Start DFPlayer serial (RX = 26, TX = 27)
  mp3Serial.begin(9600, SERIAL_8N1, 27, 26);

  Serial.println("\n[INIT] Starting DFPlayer...");

  if (!df.begin(mp3Serial)) {
    Serial.println("[ERROR] DFPlayer NOT detected!");
    Serial.println("Check:");
    Serial.println("- Wiring (TX/RX swapped?)");
    Serial.println("- Power (5V + GND)");
    Serial.println("- SD card inserted");

    while (true); // stop here
  }

  Serial.println("[OK] DFPlayer ready!");

  // Set volume (0–30)
  df.volume(25);

  delay(1000);

  // ================= TEST SEQUENCE =================
  
  Serial.println("[PLAY] System: powering_on");
  df.playFolder(2, 1);   // /02/001.mp3
  delay(3000);

  Serial.println("[PLAY] Word: rupees");
  df.playFolder(3, 2);   // /03/002.mp3
  delay(3000);

  Serial.println("[PLAY] Number: 1");
  df.playFolder(1, 1);   // /01/001.mp3
}

void loop() {
  // Nothing here
}