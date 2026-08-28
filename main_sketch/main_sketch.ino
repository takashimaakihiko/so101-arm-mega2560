#include <SCServo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "my_teaching.h"

// --- Pin assignments (Arduino Mega 2560) ---
#define MODE_SWITCH_PIN 5   // INPUT_PULLUP: LOW = Teaching Mode, HIGH = Run Mode
#define H8_TRIGGER_PIN  4   // INPUT: 5V start trigger from H8 (P85)
#define H8_DONE_PIN     3   // OUTPUT: completion pulse to H8 (P87), starts LOW

// --- OLED configuration ---
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDRESS  0x3C

// --- Shared servo-bus controller and OLED display ---
// SERVO_IDS and SERVO_COUNT are defined in my_teaching.h.
SMS_STS st;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Run-mode sequence data (user-defined) ---

// First pose: safe starting coordinates for the automatic sequence.
// Replace these placeholder values with the real calibrated coordinates.
const int FIRST_POSE[SERVO_COUNT] = {2048, 2048, 2048, 2048, 2048, 2048};

// Recorded teaching trajectory. Each row is one playback step for IDs 1..6.
// Replace these placeholder rows with the actual recorded positions.
const int RECORDED_TRAJECTORY[][SERVO_COUNT] = {
  {2048, 2048, 2048, 2048, 2048, 2048},
  {2048, 2048, 2048, 2048, 2048, 2048}
};
const int TRAJECTORY_STEPS = sizeof(RECORDED_TRAJECTORY) / sizeof(RECORDED_TRAJECTORY[0]);

// Run Mode routine.
// This function never returns; it holds the CPU in the H8 handshake loop.
void runRunMode() {
  // Restore absolute full torque for automatic motion.
  // Register 48 is the live SRAM Torque Limit (register 16 is EEPROM only).
  for (int i = 0; i < SERVO_COUNT; i++) {
    st.writeWord(SERVO_IDS[i], 48, 1023);
  }

  // Lock the arm at its current boot position so it cannot drop suddenly.
  for (int i = 0; i < SERVO_COUNT; i++) {
    int currentPos = st.ReadPos(SERVO_IDS[i]);
    if (currentPos >= 0) {
      st.WritePosEx(SERVO_IDS[i], currentPos, 0, 0);
    }
  }

  // Standby screen waiting for H8 trigger.
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Waiting H8 Trigger");
  display.setCursor(0, 8);
  display.println("(Pin 4)...");
  display.display();

  while (true) {
    // Polling lock: wait until H8 raises Pin 4 HIGH.
    while (digitalRead(H8_TRIGGER_PIN) == LOW) {
      // Busy wait for trigger.
    }

    // Trigger received: show execution status.
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Executing");
    display.setCursor(0, 8);
    display.println("sequence...");
    display.display();

    // Gently move from the physical boot pose to the sequence start pose.
    for (int i = 0; i < SERVO_COUNT; i++) {
      st.WritePosEx(SERVO_IDS[i], FIRST_POSE[i], 300, 0);
    }
    delay(3000);  // Hold to guarantee arrival.

    // Stream the recorded trajectory.
    for (int step = 0; step < TRAJECTORY_STEPS; step++) {
      for (int i = 0; i < SERVO_COUNT; i++) {
        st.WritePosEx(SERVO_IDS[i], RECORDED_TRAJECTORY[step][i], 300, 0);
      }
      delay(20);  // Tune this interval to match the recorded playback rate.
    }

    // Completion handshake: 500 ms HIGH pulse on Pin 3 to H8 (P87).
    digitalWrite(H8_DONE_PIN, HIGH);
    delay(500);
    digitalWrite(H8_DONE_PIN, LOW);

    // Return to standby screen for the next cycle.
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Waiting H8 Trigger");
    display.setCursor(0, 8);
    display.println("(Pin 4)...");
    display.display();
  }
}

void setup() {
  Serial.begin(115200);

  // Servo bus on Mega's Serial1 at 1,000,000 bps.
  Serial1.begin(1000000, SERIAL_8N1);
  st.pSerial = &Serial1;

  // Mode switch, H8 trigger input, and H8 completion output.
  pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);
  pinMode(H8_TRIGGER_PIN, INPUT);
  pinMode(H8_DONE_PIN, OUTPUT);
  digitalWrite(H8_DONE_PIN, LOW);

  // Initialize the SSD1306 I2C OLED.
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Boot-time mode selection.
  // The mode is evaluated once and then the processor is locked into the
  // selected routine until power is cycled.
  while (true) {
    if (digitalRead(MODE_SWITCH_PIN) == LOW) {
      runTeachingMode();  // Never returns.
    } else {
      runRunMode();       // Never returns.
    }
  }
}

void loop() {
  // Intentionally empty per specification.
}
