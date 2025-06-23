#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Servo.h>

// ——— Button pins ———
#define RESET_PIN D1    // toggle/reset
#define WORK_PIN  D2    // only active in WORKING

// ——— TFT pins ———
#define TFT_CS   D8
#define TFT_DC   D3
#define TFT_RST  D4

// ——— Servo pin ———
#define SERVO_PIN D6    // GPIO12

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
Servo latchServo;

// debounce
const unsigned long DEBOUNCE_DELAY = 50;
unsigned long lastTimeReset = 0, lastTimeWork = 0;
bool lastRawReset   = HIGH, debouncedReset = HIGH;
bool lastRawWork    = HIGH, debouncedWork  = HIGH;

// state machine
enum State { HOME, WORKING };
State state = HOME;
uint8_t progressCount = 0;

// servo positions
const uint8_t SERVO_OPEN_POS =   0;  // adjust if needed
const uint8_t SERVO_LOCK_POS =  90;  // adjust if needed

//— Debounce helper ——
bool pressedDebounced(uint8_t pin, bool &lastRaw, bool &debounced, unsigned long &lastTime) {
  bool reading = digitalRead(pin);
  unsigned long now = millis();
  if (reading != lastRaw) lastTime = now;
  if (now - lastTime > DEBOUNCE_DELAY && reading != debounced) {
    debounced = reading;
    if (debounced == LOW) {
      lastRaw = reading;
      return true;
    }
  }
  lastRaw = reading;
  return false;
}

//— Screen drawers ——
void drawHomeScreen() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.setCursor(60, 100);
  tft.print("HOME");
  latchServo.write(SERVO_OPEN_POS);
}

void drawWorkingScreen(uint8_t prog = 0) {
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(40, 20);
  tft.print("WORKING");
  // outline
  tft.drawRect(20, 180, 280, 20, ILI9341_WHITE);
  // fill bar
  if (prog > 0) {
    tft.fillRect(20, 180, 56 * min(prog, (uint8_t)5), 20, ILI9341_GREEN);
  }
  latchServo.write(SERVO_LOCK_POS);
}

void setup() {
  Serial.begin(115200);

  // buttons
  pinMode(RESET_PIN, INPUT_PULLUP);
  pinMode(WORK_PIN,  INPUT_PULLUP);

  // display
  tft.begin();
  tft.setRotation(1);

  // servo
  latchServo.attach(SERVO_PIN);
  latchServo.write(SERVO_OPEN_POS);

  // initial screen
  drawHomeScreen();
}

void loop() {
  // RESET button
  if (pressedDebounced(RESET_PIN, lastRawReset, debouncedReset, lastTimeReset)) {
    if (state == HOME) {
      state = WORKING;
      progressCount = 0;
      drawWorkingScreen(0);
      Serial.println("→ ENTERED WORKING");
    } else {
      state = HOME;
      progressCount = 0;
      drawHomeScreen();
      Serial.println("→ RETURNED HOME");
    }
  }

  // WORK button only in WORKING
  if (state == WORKING && pressedDebounced(WORK_PIN, lastRawWork, debouncedWork, lastTimeWork)) {
    progressCount++;
    drawWorkingScreen(progressCount);
    Serial.print("Progress: "); Serial.println(progressCount);
    if (progressCount >= 5) {
      state = HOME;
      drawHomeScreen();
      Serial.println("★ WORK COMPLETE ★");
      progressCount = 0;
    }
  }

  delay(10);
}
