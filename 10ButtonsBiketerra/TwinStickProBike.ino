/**
 * DIY Twin-Stick Controller - PRO Version
 * Board: Seeed Studio XIAO ESP32C3
 * Features: 
 * - Hysterese (gegen Flackern)
 * - Cooldown (gegen ungewolltes Doppelschalten)
 * - Umschaltbar zwischen "Halten" (Lenken) und "Tippen" (Schalten)
 */

#include <BleKeyboard.h>

BleKeyboard bleKeyboard("Twin-Stick Pro", "DIY", 100);

// --- PINS (Korrigiert auf D-Pins) ---
const int j1_X = D0;
const int j1_Y = D1;
const int j1_SW = D4;

const int j2_X = D2;
const int j2_Y = D3;
const int j2_SW = D5;

// --- EINSTELLUNGEN ---
const int CENTER = 2048;       // Mitte des Joysticks
const int DEADZONE = 600;      // Wie weit muss man bewegen? (2048 +/- 600)
const int HYSTERESIS = 200;    // Puffer zum Loslassen (gegen Flackern)

// Cooldown Zeiten (in Millisekunden)
const int COOLDOWN_GEAR = 400; // Zeit zwischen Schaltvorgängen beim Halten
const int COOLDOWN_KEY = 50;   // Kurzer Klick beim Tippen

// Modus-Definitionen
enum Mode { MODE_HOLD, MODE_PULSE };

// Struktur, um den Status jeder Achse zu speichern
struct AxisState {
  unsigned long lastActionTime; // Wann wurde zuletzt gedrückt?
  bool isActiveLow;             // Ist die "Low"-Richtung (Links/Oben) gerade aktiv?
  bool isActiveHigh;            // Ist die "High"-Richtung (Rechts/Unten) gerade aktiv?
};

AxisState stateJ1X, stateJ1Y, stateJ2X, stateJ2Y;
bool btn1State = false;
bool btn2State = false;

void setup() {
  pinMode(j1_SW, INPUT_PULLUP);
  pinMode(j2_SW, INPUT_PULLUP);
  pinMode(j1_X, INPUT); pinMode(j1_Y, INPUT);
  pinMode(j2_X, INPUT); pinMode(j2_Y, INPUT);
  
  bleKeyboard.begin();
}

/**
 * Die Magie passiert hier:
 * @param val: Der aktuelle Analogwert
 * @param keyLow: Taste für "Kleiner Wert" (Links/Oben)
 * @param keyHigh: Taste für "Großer Wert" (Rechts/Unten)
 * @param state: Speicher für diese Achse
 * @param mode: MODE_HOLD (für Lenken/Gas) oder MODE_PULSE (für Gänge)
 */
void handleAxisSmart(int val, uint8_t keyLow, uint8_t keyHigh, AxisState &state, Mode mode) {
  unsigned long now = millis();

  // --- RICHTUNG 1: LOW (z.B. Links oder Oben) ---
  // Einschalten: Wenn Wert unter (Mitte - Deadzone) sinkt
  if (val < (CENTER - DEADZONE)) {
    
    // Fall A: MODE_HOLD (Dauerhaft drücken, z.B. Lenken)
    if (mode == MODE_HOLD) {
      if (!state.isActiveLow) {
        bleKeyboard.press(keyLow);
        state.isActiveLow = true;
      }
    }
    // Fall B: MODE_PULSE (Für Gänge/Tippen)
    else if (mode == MODE_PULSE) {
      // Nur drücken, wenn Cooldown abgelaufen ist
      if (now - state.lastActionTime > COOLDOWN_GEAR) {
        bleKeyboard.press(keyLow);
        delay(COOLDOWN_KEY); // Kurz halten
        bleKeyboard.release(keyLow);
        state.lastActionTime = now;
        state.isActiveLow = true; // Merken, dass wir im "Bereich" sind
      }
    }
  } 
  // Ausschalten mit HYSTERESE: Erst wenn Wert wieder deutlich Richtung Mitte wandert
  else if (val > (CENTER - DEADZONE + HYSTERESIS)) {
    if (state.isActiveLow) {
      if (mode == MODE_HOLD) bleKeyboard.release(keyLow);
      state.isActiveLow = false;
      // Bei Pulse Mode resetten wir den Timer nicht zwingend, 
      // damit man nicht durch schnelles Wackeln spammen kann.
    }
  }


  // --- RICHTUNG 2: HIGH (z.B. Rechts oder Unten) ---
  if (val > (CENTER + DEADZONE)) {
    
    if (mode == MODE_HOLD) {
      if (!state.isActiveHigh) {
        bleKeyboard.press(keyHigh);
        state.isActiveHigh = true;
      }
    }
    else if (mode == MODE_PULSE) {
      if (now - state.lastActionTime > COOLDOWN_GEAR) {
        bleKeyboard.press(keyHigh);
        delay(COOLDOWN_KEY);
        bleKeyboard.release(keyHigh);
        state.lastActionTime = now;
        state.isActiveHigh = true;
      }
    }
  } 
  // Ausschalten mit HYSTERESE
  else if (val < (CENTER + DEADZONE - HYSTERESIS)) {
    if (state.isActiveHigh) {
      if (mode == MODE_HOLD) bleKeyboard.release(keyHigh);
      state.isActiveHigh = false;
    }
  }
}

void loop() {
  if (bleKeyboard.isConnected()) {
    
    // Werte lesen
    int x1 = analogRead(j1_X);
    int y1 = analogRead(j1_Y);
    int x2 = analogRead(j2_X);
    int y2 = analogRead(j2_Y);

    // --- KONFIGURATION DER ACHSEN ---
    
    // Joystick 1 (Links) -> WASD zum Bewegen -> MODE_HOLD
    handleAxisSmart(x1, 'a', 'd', stateJ1X, MODE_HOLD);
    handleAxisSmart(y1, 'w', 's', stateJ1Y, MODE_HOLD);
    
    // Joystick 2 (Rechts) 
    // X-Achse -> Pfeile Links/Rechts (Umschauen) -> MODE_HOLD
    handleAxisSmart(x2, KEY_LEFT_ARROW, KEY_RIGHT_ARROW, stateJ2X, MODE_HOLD);
    
    // Y-Achse -> Gänge schalten (z.B. Pfeil Oben/Unten) -> MODE_PULSE (Hier ist der Cooldown!)
    // Das bedeutet: Stick nach vorne halten = Einmal schalten, warten, nochmal schalten.
    handleAxisSmart(y2, KEY_UP_ARROW, KEY_DOWN_ARROW, stateJ2Y, MODE_PULSE);


    // Buttons (Einfache Entprellung)
    if (digitalRead(j1_SW) == LOW) {
      if (!btn1State) { bleKeyboard.press(' '); btn1State = true; }
    } else btn1State = false; // Hier könnte man auch Hysterese/Debounce einbauen wenn nötig
    
    if (digitalRead(j2_SW) == LOW) {
      if (!btn2State) { bleKeyboard.press(KEY_RETURN); btn2State = true; }
    } else {
        if(btn2State) { bleKeyboard.release(KEY_RETURN); btn2State = false; }
    }
  }
  
  delay(10);
}s