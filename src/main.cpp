#include <SPI.h>
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// Pin assignments
const uint8_t PIN_D1 = D1;      // Touch/button 1: toggle/reset state
const uint8_t PIN_D2 = D2;      // Touch/button 2: only in WORKING state

// Debounce settings
const unsigned long DEBOUNCE_DELAY = 50; // ms

// State machine
enum State {
  HOME,
  WORKING
};

State currentState = HOME;
uint8_t progressCount = 0;

// For debouncing
unsigned long lastDebounceTimeD1 = 0;
unsigned long lastDebounceTimeD2 = 0;
bool lastButtonStateD1 = HIGH;
bool lastButtonStateD2 = HIGH;

void setup() {
  Serial.begin(115200);

  // Configure buttons as inputs with pullups
  pinMode(PIN_D1, INPUT_PULLUP);
  pinMode(PIN_D2, INPUT_PULLUP);

  // Initial state
  Serial.println("Starting in HOME state");
}

void loop() {
  // Read pins (LOW when pressed if wired to GND + INPUT_PULLUP)
  bool readingD1 = digitalRead(PIN_D1);
  bool readingD2 = digitalRead(PIN_D2);
  unsigned long now = millis();

  // --- Handle D1 press (toggle/reset) ---
  if (readingD1 != lastButtonStateD1) {
    lastDebounceTimeD1 = now;
  }
  if (now - lastDebounceTimeD1 > DEBOUNCE_DELAY) {
    // If the button state has stabilized…
    static bool stableStateD1 = HIGH;
    if (readingD1 != stableStateD1) {
      stableStateD1 = readingD1;
      if (stableStateD1 == LOW) { // button went HIGH→LOW: pressed
        if (currentState == HOME) {
          currentState = WORKING;
          progressCount = 0;
          Serial.println("→ ENTERED WORKING state");
        } else {
          currentState = HOME;
          progressCount = 0;
          Serial.println("→ RESET TO HOME state");
        }
      }
    }
  }
  lastButtonStateD1 = readingD1;

  // --- Handle D2 press (only in WORKING) ---
  if (currentState == WORKING) {
    if (readingD2 != lastButtonStateD2) {
      lastDebounceTimeD2 = now;
    }
    if (now - lastDebounceTimeD2 > DEBOUNCE_DELAY) {
      static bool stableStateD2 = HIGH;
      if (readingD2 != stableStateD2) {
        stableStateD2 = readingD2;
        if (stableStateD2 == LOW) {
          // Increment progress
          progressCount++;
          Serial.print("Working progress: ");
          Serial.println(progressCount);
          // Check for completion
          if (progressCount >= 5) {
            currentState = HOME;
            progressCount = 0;
            Serial.println("★ WORK COMPLETE — Returning to HOME ★");
          }
        }
      }
    }
    lastButtonStateD2 = readingD2;
  } else {
    // If not in WORKING, keep lastButtonStateD2 in sync
    lastButtonStateD2 = readingD2;
  }

  // small delay to avoid busy‐looping
  delay(10);
}