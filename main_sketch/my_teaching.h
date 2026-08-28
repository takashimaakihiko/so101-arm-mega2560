#ifndef MY_TEACHING_H
#define MY_TEACHING_H

#include <SCServo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSans9pt7b.h>

#define SERVO_COUNT 6
const uint8_t SERVO_IDS[SERVO_COUNT] = {1, 2, 3, 4, 5, 6};

// Soft torque limit used during gravity-compensated direct teaching.
// This value allows a user to move the arm by hand while still preventing sudden drops.
#define GRAVITY_TORQUE 250

// Shared controller and display objects are defined in main_sketch.ino.
extern SMS_STS st;
extern Adafruit_SSD1306 display;

// Helper to draw one OLED line with two servo IDs/values.
// Uses the FreeSans9pt7b GFX font for a clean, larger look without
// the blocky pixel-doubling of setTextSize(2). -1 positions render as "ERR".
inline void drawBigServoLine(int lineY, uint8_t id1, int pos1, uint8_t id2, int pos2) {
  display.setFont(&FreeSans9pt7b);
  display.setTextSize(1);
  display.setCursor(0, lineY);

  if (id1 < 10) display.print("0");
  display.print(id1);
  display.print("=");
  if (pos1 == -1) {
    display.print("ERR");
  } else {
    display.print(pos1);
  }

  display.print("  ");  // visual separator between the two servo pairs

  if (id2 < 10) display.print("0");
  display.print(id2);
  display.print("=");
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
  // Gravity-compensation initialization
  // ------------------------------------------------------------------
  // Read the arm's current physical pose, command each servo to hold that
  // pose immediately, and then limit the max torque for safe manual positioning.
  for (int i = 0; i < SERVO_COUNT; i++) {
    int currentPos = st.ReadPos(SERVO_IDS[i]);
    if (currentPos >= 0) {
      st.WritePosEx(SERVO_IDS[i], currentPos, 0, 0);
    }
    st.writeWord(SERVO_IDS[i], 16, GRAVITY_TORQUE);
  }

  // ------------------------------------------------------------------
  // Anti-backlash trajectory-following loop
  // ------------------------------------------------------------------
  while (true) {
    // Read all joint encoder values for display.
    int pos[SERVO_COUNT];
    for (int i = 0; i < SERVO_COUNT; i++) {
      pos[i] = st.ReadPos(SERVO_IDS[i]);
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
    display.setCursor(0, 60);
    display.print("[TEACHING MODE]");

    display.display();

    // Instantly overwrite the servo's goal with its freshly read current position.
    // This prevents gravitational spring-back after the user releases the arm.
    for (int i = 0; i < SERVO_COUNT; i++) {
      int currentPos = st.ReadPos(SERVO_IDS[i]);
      if (currentPos >= 0) {
        st.WritePosEx(SERVO_IDS[i], currentPos, 0, 0);
      }
    }

    delay(50);  // ~20 Hz refresh rate
  }
}

#endif // MY_TEACHING_H
