#include <ClickButton.h>
#include <EEPROM.h>
#include <SPI.h>
#include <MFRC522.h>

// === Pin Setup ===
const int buttonPin = 2;
const int relayPins[4] = {3, 4, 5, 6};
const int touchPin = 7;
const int buzzerPin = 8;

// === RFID Setup ===
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);

bool relayState[4] = {false, false, false, false};
ClickButton button(buttonPin, LOW, CLICKBTN_PULLUP);

// EEPROM address
const int eepromAddr[3] = {0, 1, 2};
const int eepromModeAddr   = 10;
const int eepromRfidAddr   = 11;
const int eepromTouchAddr  = 12;

unsigned long touchStart = 0;
bool touchActive = false;
const unsigned long touchThreshold = 250;

// Beep & Mode
int beepMode = 1; // 1–5 normal, 6 = diam
bool rfidEnabled  = true;
bool touchEnabled = true;

// RFID reset watchdog
unsigned long lastRFIDCheck = 0;
const unsigned long RFID_TIMEOUT = 3000; 
int rfidResetCount = 0;

// === Beep Functions ===
void playBeepOn(int mode) {
  if (mode == 1) {          
    tone(buzzerPin, 1047, 150); delay(180);
    tone(buzzerPin, 1175, 150); delay(180);
    tone(buzzerPin, 1319, 200); delay(250);
  } 
  else if (mode == 2) {     
    tone(buzzerPin, 988, 200); delay(250);
    tone(buzzerPin, 1319, 250); delay(300);
  }
  else if (mode == 3) {     
    tone(buzzerPin, 1319, 150); delay(180);
    tone(buzzerPin, 1175, 150); delay(180);
    tone(buzzerPin, 1568, 250); delay(300);
  }
  else if (mode == 4) {     
    tone(buzzerPin, 1200, 300); delay(350);
  }
  else if (mode == 5) {     
    tone(buzzerPin, 1500, 150); delay(200);
    tone(buzzerPin, 1200, 500); delay(600);
  }
  noTone(buzzerPin);
}

void playBeepOff(int mode) {
  if (mode == 1) {          
    tone(buzzerPin, 1319, 150); delay(180);
    tone(buzzerPin, 1175, 150); delay(180);
    tone(buzzerPin, 1047, 200); delay(250);
  }
  else if (mode == 2) {     
    tone(buzzerPin, 880, 200); delay(250);
  }
  else if (mode == 3) {     
    tone(buzzerPin, 1568, 150); delay(180);
    tone(buzzerPin, 1175, 150); delay(180);
    tone(buzzerPin, 988, 200); delay(250);
  }
  else if (mode == 4) {     
    tone(buzzerPin, 1000, 300); delay(350);
  }
  else if (mode == 5) {     
    tone(buzzerPin, 1000, 600); delay(650);
  }
  noTone(buzzerPin);
}

void playModeChange(int mode) {
  if (mode == 6) {
    Serial.println("Mode beep = Diam");
    return;
  }
  for (int i = 0; i < mode; i++) {
    tone(buzzerPin, 1000 + i*200, 150);
    delay(200);
  }
  noTone(buzzerPin);
}

// === Welcome Tone ===
void playWelcomeTone() {
  if (beepMode == 6) return; // kalau mode diam, jangan bunyi
  // tone(buzzerPin, 880, 150);  delay(200);
  // tone(buzzerPin, 1047, 150); delay(200);
  // tone(buzzerPin, 1319, 250); delay(300);
  tone(buzzerPin, 2100, 100);
  delay(300);
  tone(buzzerPin, 2100, 100);
  delay(300);
  noTone(buzzerPin);
}

// === Relay Toggle ===
void toggleRelay4() {
  relayState[3] = !relayState[3];
  digitalWrite(relayPins[3], relayState[3] ? LOW : HIGH);
  Serial.print("Relay 4 = ");
  Serial.println(relayState[3] ? "ON" : "OFF");
  if (relayState[3]) playBeepOn(beepMode);
  else playBeepOff(beepMode);
}

// === Setup ===
void setup() {
  Serial.begin(9600);
  Serial.println("=== Sistem Start ===");

  // Relay 1–3 status EEPROM
  for (int i = 0; i < 3; i++) {
    relayState[i] = EEPROM.read(eepromAddr[i]) == 1;
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], relayState[i] ? LOW : HIGH); 
    Serial.print("Relay ");
    Serial.print(i + 1);
    Serial.print(" = ");
    Serial.println(relayState[i] ? "ON" : "OFF");
  }

  // Relay 4 selalu ON saat start
  relayState[3] = true;             
  pinMode(relayPins[3], OUTPUT);
  digitalWrite(relayPins[3], LOW);  
  Serial.println("Relay 4 = ON (default saat start)");

  // Tombol
  button.debounceTime   = 20;
  button.multiclickTime = 400;
  button.longClickTime  = 2000;  // 5 detik long press

  pinMode(touchPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  // Mode beep
  beepMode = EEPROM.read(eepromModeAddr);
  if (beepMode == 255 || beepMode < 1 || beepMode > 6) {
    beepMode = 1;
    EEPROM.update(eepromModeAddr, beepMode);
  }

  // Status RFID & Touch
  rfidEnabled  = EEPROM.read(eepromRfidAddr)  != 0;
  touchEnabled = EEPROM.read(eepromTouchAddr) != 0;

  Serial.print("Mode beep = ");
  if (beepMode == 6) Serial.println("Diam");
  else Serial.println(beepMode);
  Serial.print("RFID = ");  Serial.println(rfidEnabled  ? "Aktif" : "Mati");
  Serial.print("Touch = "); Serial.println(touchEnabled ? "Aktif" : "Mati");

  // Beep startup
  if (beepMode != 6) {
    // tone(buzzerPin, 1200, 200);
    // delay(250);
    // noTone(buzzerPin);

    // 🎶 Welcome Tone
    playWelcomeTone();
  }

  // Inisialisasi RFID
  SPI.begin();
  mfrc522.PCD_Init();
  lastRFIDCheck = millis();
  Serial.println("Tempelkan kartu RFID...");
}

// === Main Loop ===
void loop() {
  button.Update();

  // 🔔 Bunyi tombol sekali saat ditekan, semua mode
  static bool lastButtonState = HIGH;
  bool currentButtonState = digitalRead(buttonPin);
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    tone(buzzerPin, 1200, 80);  // beep 1x langsung saat tekan
  }
  lastButtonState = currentButtonState;

  if (button.clicks != 0) {
    if (button.clicks > 0) { 
      int clickNum = button.clicks;
      Serial.print("Tombol ditekan ");
      Serial.print(clickNum);
      Serial.println(" kali");

      // === Aksi Tombol ===
      if (clickNum >= 1 && clickNum <= 3) {
        int relayIndex = clickNum - 1;
        relayState[relayIndex] = !relayState[relayIndex];
        digitalWrite(relayPins[relayIndex], relayState[relayIndex] ? LOW : HIGH);
        EEPROM.update(eepromAddr[relayIndex], relayState[relayIndex] ? 1 : 0);
        Serial.print("Relay ");
        Serial.print(relayIndex + 1);
        Serial.print(" = ");
        Serial.println(relayState[relayIndex] ? "ON" : "OFF");

        // Beep 2x jika relay ON, 1x jika relay OFF
        if (relayState[relayIndex]) {
          tone(buzzerPin, 2200, 80); 
          delay(200);
          tone(buzzerPin, 2200, 80); 
        } else {
          tone(buzzerPin, 3200, 80); 
          delay(80);
          noTone(buzzerPin);
        }
      }
      else if (clickNum == 4) {
        toggleRelay4();
      }
      else if (clickNum == 6) {  
        beepMode++;
        if (beepMode > 6) beepMode = 1;   
        EEPROM.update(eepromModeAddr, beepMode);
        Serial.print("Mode beep diganti ke ");
        if (beepMode == 6) Serial.println("Diam");
        else Serial.println(beepMode);
        playModeChange(beepMode);
      }
      else if (clickNum == 7) {  
        rfidEnabled = !rfidEnabled;
        EEPROM.update(eepromRfidAddr, rfidEnabled ? 1 : 0);
        Serial.print("RFID ");
        Serial.println(rfidEnabled ? "Diaktifkan" : "Dimatikan");
        if (beepMode != 6) tone(buzzerPin, rfidEnabled ? 1500 : 500, 200);
      }
      else if (clickNum == 8) {  
        touchEnabled = !touchEnabled;
        EEPROM.update(eepromTouchAddr, touchEnabled ? 1 : 0);
        Serial.print("Touch ");
        Serial.println(touchEnabled ? "Diaktifkan" : "Dimatikan");
        if (beepMode != 6) tone(buzzerPin, touchEnabled ? 1800 : 600, 200);
      }
    }
    else if (button.clicks < 0) {
      Serial.println("Long press 5 detik: Toggle Relay 1-3");

      bool newState = !(relayState[0] || relayState[1] || relayState[2]); 

      for (int i = 0; i < 3; i++) {
        relayState[i] = newState;
        digitalWrite(relayPins[i], relayState[i] ? LOW : HIGH);
        EEPROM.update(eepromAddr[i], relayState[i] ? 1 : 0);
        Serial.print("Relay ");
        Serial.print(i + 1);
        Serial.print(" = ");
        Serial.println(relayState[i] ? "ON" : "OFF");
      }

      if (newState) playBeepOn(beepMode);
      else playBeepOff(beepMode);
    }
  }

  // === Touch Sensor untuk Relay 4 ===
  if (touchEnabled) {
    int touchState = digitalRead(touchPin);
    if (touchState == HIGH && !touchActive) {
      touchStart = millis();
      touchActive = true;
    }
    if (touchState == HIGH && touchActive && (millis() - touchStart >= touchThreshold)) {
      Serial.println("Sentuh terdeteksi: Toggle relay 4");
      toggleRelay4();
      while (digitalRead(touchPin) == HIGH) delay(10);
      touchActive = false;
    }
    if (touchState == LOW) touchActive = false;
  }

  // === RFID untuk Relay 4 ===
  if (rfidEnabled) {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      String uidString = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        if (mfrc522.uid.uidByte[i] < 0x10) uidString += "0";
        uidString += String(mfrc522.uid.uidByte[i], HEX);
      }
      uidString.toUpperCase();

      Serial.print("Kartu Terdeteksi UID: ");
      Serial.println(uidString);

      if (uidString == "idtag1" || uidString == "idtag2") {   
        Serial.println("Kartu valid → Toggle Relay 4");
        toggleRelay4();
      } else {
        Serial.println("Kartu tidak dikenal!");
        if (beepMode != 6) {
          tone(buzzerPin, 400, 300); 
          delay(300);
        }
      }
      mfrc522.PICC_HaltA();
      lastRFIDCheck = millis();
    }

    // === Watchdog RFID Reset ===
    if (millis() - lastRFIDCheck > RFID_TIMEOUT) {
      Serial.println("⚠️ RFID tidak merespon, reset modul...");
      mfrc522.PCD_Init();
      lastRFIDCheck = millis();
      rfidResetCount++;
      Serial.print("Jumlah reset RFID: ");
      Serial.println(rfidResetCount);
    }
  }
}
