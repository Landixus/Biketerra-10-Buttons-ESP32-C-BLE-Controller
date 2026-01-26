/**
 * DIY Steering Controller - PRO Version (10 Tasten)
 * Board: ESP32-C3
 * Features: 
 * - Bluetooth Tastatur
 * - Software Entprellung (Debouncing)
 * - Deep Sleep nach 5 Min Inaktivität
 * - Wake-Up über Taste "Links" (Pin 3)
 */

#include <BleKeyboard.h>
#include "driver/gpio.h"

BleKeyboard bleKeyboard("Lenkrad Pro", "DIY", 100);

// --- KONFIGURATION ---
const unsigned long SLEEP_TIMEOUT = 5 * 60 * 1000; // 5 Minuten
const unsigned long DEBOUNCE_DELAY = 50;           // 50ms Entprellzeit

// Struktur für einen Taster
struct Button {
  uint8_t pin;          // Welcher Pin?
  uint8_t key;          // Welche Taste senden? (char oder Key-Code)
  bool isPressed;       // Aktueller logischer Status
  unsigned long lastDebounceTime; // Zeitstempel für Entprellung
  int lastPinState;     // Letzter physischer Status (für Flankenerkennung)
};

// --- TASTEN DEFINITION ---
// Hier fügen wir alle Tasten in eine Liste ein.
// Syntax: {PIN, TASTE, false, 0, HIGH}
Button buttons[] = {
  {3,  KEY_LEFT_ARROW,  false, 0, HIGH}, // Index 0 (Wake-Up Pin!)
  {4,  KEY_RIGHT_ARROW, false, 0, HIGH},
  {0,  'w',             false, 0, HIGH},
  {1,  's',             false, 0, HIGH},
  {2,  'c',             false, 0, HIGH},
  {5,  'b',             false, 0, HIGH},
  {6,  'u',             false, 0, HIGH},
  {7,  'f',             false, 0, HIGH},
  {8,  'n',             false, 0, HIGH},
  {10, ' ',             false, 0, HIGH}  // ' ' ist die Leertaste (Space)
};

// Anzahl der Tasten automatisch berechnen
const int buttonCount = sizeof(buttons) / sizeof(Button);

// Zeit-Management
unsigned long lastActivityTime = 0;

void setup() {
  // Alle Pins initialisieren
  for (int i = 0; i < buttonCount; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }

  // Timer Start
  lastActivityTime = millis();
  
  bleKeyboard.begin();
}

void goToDeepSleep() {
  bleKeyboard.end();
  
  // Wake-Up nur auf Pin 3 (Links) konfigurieren
  // (Pin 3 ist Index 0 in unserem Array)
  uint8_t wakeUpPin = buttons[0].pin; 
  
  // Konfiguration für ESP32-C3: Aufwachen wenn Pin 3 LOW ist
  esp_deep_sleep_enable_gpio_wakeup(1ULL << wakeUpPin, ESP_GPIO_WAKEUP_GPIO_LOW);
  
  esp_deep_sleep_start();
}

void loop() {
  // 1. Auto-Sleep prüfen
  if (millis() - lastActivityTime > SLEEP_TIMEOUT) {
    goToDeepSleep();
  }

  // 2. Nur arbeiten, wenn verbunden
  if (bleKeyboard.isConnected()) {
    
    // Durch alle Tasten loopen
    for (int i = 0; i < buttonCount; i++) {
      // Physischen Pin lesen
      int reading = digitalRead(buttons[i].pin);

      // Hat sich der Status geändert? (Entprell-Logik Start)
      if (reading != buttons[i].lastPinState) {
        buttons[i].lastDebounceTime = millis();
      }

      // Wenn genug Zeit vergangen ist seit der letzten Änderung...
      if ((millis() - buttons[i].lastDebounceTime) > DEBOUNCE_DELAY) {
        
        // ...und der Status tatsächlich anders ist als das, was wir gespeichert haben:
        // (Hier negieren wir "isPressed", da LOW = Gedrückt bedeutet)
        bool physStateIsPressed = (reading == LOW);

        if (physStateIsPressed != buttons[i].isPressed) {
          // Status übernehmen
          buttons[i].isPressed = physStateIsPressed;

          // Aktion ausführen
          if (buttons[i].isPressed) {
            bleKeyboard.press(buttons[i].key);
            lastActivityTime = millis(); // Aktivität erkannt -> Timer reset
          } else {
            bleKeyboard.release(buttons[i].key);
            lastActivityTime = millis();
          }
        }
      }

      // Den "letzten" Pin-Status für den nächsten Durchlauf speichern
      buttons[i].lastPinState = reading;
    }
  } else {
    // Falls Verbindung weg ist: Trotzdem Pins überwachen, um Timer zurückzusetzen
    // (Damit er nicht einschläft, während man versucht zu verbinden)
    for (int i = 0; i < buttonCount; i++) {
       if (digitalRead(buttons[i].pin) == LOW) {
         lastActivityTime = millis();
       }
    }
  }
  
  delay(5); // Kleines Delay für CPU Entlastung
}