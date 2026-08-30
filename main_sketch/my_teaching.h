#ifndef MY_TEACHING_H
#define MY_TEACHING_H

#include <SCServo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSans9pt7b.h>

#define SERVO_COUNT 6
const uint8_t SERVO_IDS[SERVO_COUNT] = {0, 1, 2, 3, 4, 5};

// For SO101-style leader-arm teaching, the servos are put into a
// back-drivable (torque-off) state so the arm can be posed by hand.
// Torque is re-enabled automatically when power is cycled into run mode.

// Shared controller and display objects are defined in main_sketch.ino.
extern SMS_STS st;
extern Adafruit_SSD1306 display;

// Helper to draw one OLED line with two servo IDs/values.
// The small "01=" style labels use the default font, while the values use
// FreeSans9pt7b for a larger, cleaner look. -1 positions render as "ERR".
inline void drawBigServoLine(int lineY, uint8_t id1, int pos1, uint8_t id2, int pos2) {
  // lineY is the baseline for the big FreeSans9pt7b value.
  // The small default-font label is placed just above that baseline.

  // First pair: small label
  display.setFont();
  display.setTextSize(1);
  display.setCursor(0, lineY - 7);
  if (id1 < 10) display.print("0");
  display.print(id1);
  display.print("=");

  // First pair: big value
  display.setFont(&FreeSans9pt7b);
  display.setTextSize(1);
  display.setCursor(18, lineY);
  if (pos1 == -1) {
    display.print("ERR");
  } else {
    display.print(pos1);
  }

  // Second pair: small label
  display.setFont();
  display.setTextSize(1);
  display.setCursor(64, lineY - 7);
  if (id2 < 10) display.print("0");
  display.print(id2);
  display.print("=");

  // Second pair: big value
  display.setFont(&FreeSans9pt7b);
  display.setTextSize(1);
  display.setCursor(82, lineY);
  if (pos2 == -1) {
    display.print("ERR");
  } else {
    display.print(pos2);
  }
}

// Teaching mode routine.
// This function never returns; it holds the CPU in the teaching loop until power is cycled.
void runTeachingMode() {
  // ------------------------------------------------------------------
  // Teaching-mode initialization
  // ------------------------------------------------------------------
  // Use limited torque so each joint yields by hand and holds when released.
  // Each servo follows an external displacement beyond the movement threshold.
  // Register 40 is the live Torque Enable bit (0 = off, 1 = on).
  const uint16_t teachingTorque[SERVO_COUNT] = {75, 500, 370, 75, 0, 0};
  const int movementThreshold = 12;
  int holdPos[SERVO_COUNT];
  int pos[SERVO_COUNT];

  for (int i = 0; i < SERVO_COUNT; i++) {
    st.writeByte(SERVO_IDS[i], 40, 0);
    delay(2);  // small gap to avoid bus collisions between servos
    holdPos[i] = st.ReadPos(SERVO_IDS[i]);
    if (holdPos[i] == -1) {
      delay(5);
      holdPos[i] = st.ReadPos(SERVO_IDS[i]);
    }
    pos[i] = holdPos[i];
    if (holdPos[i] != -1 && teachingTorque[i] > 0) {
      st.WritePosEx(SERVO_IDS[i], holdPos[i], 0, 0);
      st.writeWord(SERVO_IDS[i], 48, teachingTorque[i]);
      st.writeByte(SERVO_IDS[i], 40, 1);
    }
    delay(2);
  }

  // ------------------------------------------------------------------
  // Teaching-mode display loop
  // ------------------------------------------------------------------
  while (true) {
    // Read all joint encoder values and move the hold target while handled.
    // Add a short gap between consecutive reads to avoid collisions on the
    // shared half-duplex bus.
    for (int i = 0; i < SERVO_COUNT; i++) {
      int rawPos = st.ReadPos(SERVO_IDS[i]);
      if (rawPos == -1) {
        delay(5);
        rawPos = st.ReadPos(SERVO_IDS[i]);
      }
      if (rawPos != -1) {
        pos[i] = rawPos;
      }
      if (teachingTorque[i] > 0 && rawPos != -1 && holdPos[i] != -1 && abs(rawPos - holdPos[i]) >= movementThreshold) {
        holdPos[i] = rawPos;
        st.WritePosEx(SERVO_IDS[i], holdPos[i], 0, 0);
      }
      delay(2);
    }

    // Render a 4-line display layout with large values for readability.
    // FreeSans9pt7b is used for the servo pairs; the mode line stays small.
    // Baseline positions are spaced 18 pixels apart to fit the 9pt font.
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    drawBigServoLine(12, 1, pos[0], 2, pos[1]);
    drawBigServoLine(30, 3, pos[2], 4, pos[3]);
    drawBigServoLine(48, 5, pos[4], 6, pos[5]);

    display.setFont();  // revert to default font for the mode indicator
    display.setTextSize(1);
    display.setCursor(0, 54);
    display.print("[TEACHING MODE]");

    display.display();

    delay(50);  // ~20 Hz refresh rate
  }
}

#endif // MY_TEACHING_H
