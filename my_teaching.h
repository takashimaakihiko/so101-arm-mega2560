#ifndef MY_TEACHING_H
#define MY_TEACHING_H

#include <SCServo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SERVO_COUNT 6
const uint8_t SERVO_IDS[SERVO_COUNT] = {1, 2, 3, 4, 5, 6};

// Soft torque limit used during gravity-compensated direct teaching.
// This value allows a user to move the arm by hand while still preventing sudden drops.
#define GRAVITY_TORQUE 250

// Shared controller and display objects are defined in main_sketch.ino.
extern SMS_STS st;
extern Adafruit_SSD1306 display;

// Helper to draw one OLED line with two servo labels/values.
// pos values of -1 are rendered as "ERR".
inline void drawServoLine(int lineY, const char* label1, int pos1, const char* label2, int pos2) {
  display.setCursor(0, lineY);
  display.print(label1);
  if (pos1 == -1) {
    display.print("ERR");
  } else {
    display.print(pos1);
  }
  display.print(" ");
  display.print(label2);
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
      st.WritePosEx(SERVO_IDS[i], currentPos, 0, 0, 0);
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

    // Render the 4-line display layout at 128x64 with text size 1.
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    drawServoLine(0,  "ID1=", pos[0], "ID2=", pos[1]);
    drawServoLine(8,  "ID3=", pos[2], "ID4=", pos[3]);
    drawServoLine(16, "ID5=", pos[4], "ID6=", pos[5]);

    display.setCursor(0, 24);
    display.print("[TEACHING MODE]");

    display.display();

    // Instantly overwrite the servo's goal with its freshly read current position.
    // This prevents gravitational spring-back after the user releases the arm.
    for (int i = 0; i < SERVO_COUNT; i++) {
      int currentPos = st.ReadPos(SERVO_IDS[i]);
      if (currentPos >= 0) {
        st.WritePosEx(SERVO_IDS[i], currentPos, 0, 0, 0);
      }
    }

    delay(50);  // ~20 Hz refresh rate
  }
}

#endif // MY_TEACHING_H
