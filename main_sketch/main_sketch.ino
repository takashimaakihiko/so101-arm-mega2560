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

void jointpos(int p1, int p2, int p3, int p4, int p5, int p6) {
  uint8_t ids[SERVO_COUNT];
  int16_t positions[SERVO_COUNT] = {p1, p2, p3, p4, p5, p6};
  uint16_t speeds[SERVO_COUNT];
  uint8_t accelerations[SERVO_COUNT];

  for (int i = 0; i < SERVO_COUNT; i++) {
    ids[i] = SERVO_IDS[i];
    speeds[i] = 300;
    accelerations[i] = 0;
  }

  st.SyncWritePosEx(ids, SERVO_COUNT, positions, speeds, accelerations);
  delay(3000);
}

void jointpos2(int p1, int p2, int p3, int p4, int p5, int p6) {
  uint8_t ids[SERVO_COUNT];
  int16_t positions[SERVO_COUNT] = {p1, p2, p3, p4, p5, p6};
  uint16_t speeds[SERVO_COUNT];
  uint8_t accelerations[SERVO_COUNT];

  for (int i = 0; i < SERVO_COUNT; i++) {
    ids[i] = SERVO_IDS[i];
    speeds[i] = 600;
    accelerations[i] = 0;
  }

  st.SyncWritePosEx(ids, SERVO_COUNT, positions, speeds, accelerations);
  delay(3000);
}

void runSequence1() { 
  //グリッパの開閉は最小が2100 これより小さい値では動かない
  //グリッパの最大は3125でこれより大きい値をあたえると動かない
  //グリッパの開閉はいったん時間をおいてアーム動作が終わってから開閉する必要がある
  //グリッパがちゃんと開ききるまで2秒まつ、閉じるまで2秒まつ
  jointpos(2031,  771, 3126,  799, 2024, 3125);
  jointpos(2031, 1491, 1939, 2788, 2024, 3125);
  jointpos(2031, 1491, 1939, 2788, 2024, 3125);
  delay(2000);//1000=1sec
  jointpos(2031, 1491, 1939, 2788, 2024, 2100);
  delay(2000);//1000=1sec
  jointpos(2031,  771, 3126,  799, 2024, 2100);
  jointpos(2031, 1140, 1063,  975, 2024, 2100);
  jointpos(2031, 1140, 1063,  975, 2024, 2100);
  delay(2000);
  jointpos(2031, 1140, 1063,  975, 2024, 3125);
  delay(2000);
  jointpos(2031, 1491, 1939, 2788, 2024, 3125);
  jointpos(2031,  771, 3126,  799, 2024, 3125);
  delay(2000);//これがないとアームが動いてる最中に移動が始まる
}

void runSequence2() {
  jointpos(2031,  771, 3126,  799, 2024, 3125);
  jointpos(2031, 1140, 1063,  975, 2024, 3125);
  jointpos(2031, 1140, 1063,  975, 2024, 3125);
  delay(2000);
  jointpos(2031, 1140, 1063,  975, 2024, 2100);
  delay(2000);
  jointpos(2031,  771, 3126,  799, 2024, 2100);
  jointpos(2031, 3089, 2285,  854, 1925, 2100);
  delay(2000);
  jointpos(2031, 3089, 2285,  854, 1925, 3125);
  delay(2000);
  jointpos(2031,  771, 3126,  799, 2024, 3125);
  delay(4000);//これがないとアームが動いてる最中に移動が始まる 床から初期姿勢までは4秒くらいかかる？

}

void waitForH8Trigger() {
  while (digitalRead(H8_TRIGGER_PIN) == LOW) {}
  delay(20);
  while (digitalRead(H8_TRIGGER_PIN) == LOW) {}
}

void sendH8Done() {
  digitalWrite(H8_DONE_PIN, HIGH);
  delay(500);
  digitalWrite(H8_DONE_PIN, LOW);
}

void showRunStatus(const char *line1, const char *line2) {
  display.clearDisplay();
  display.setFont();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(line1);
  display.setCursor(0, 8);
  display.println(line2);
  display.display();
}

// Run Mode routine.
// This function never returns; it holds the CPU in the H8 handshake loop.
void runRunMode() {
  // Restore absolute full torque for automatic motion.
  // Register 48 is the live SRAM Torque Limit (register 16 is EEPROM only).
  for (int i = 0; i < SERVO_COUNT; i++) {
    display.clearDisplay();
    display.setFont();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("Servo init: ID");
    display.println(i + 1);
    display.display();

    st.writeByte(SERVO_IDS[i], 40, 0);
    delay(5);
    int currentPos = -1;
    while (currentPos == -1) {
      currentPos = st.ReadPos(SERVO_IDS[i]);
      if (currentPos == -1) {
        delay(10);
      }
    }
    st.WritePosEx(SERVO_IDS[i], currentPos, 0, 0);
    delay(5);
    st.writeWord(SERVO_IDS[i], 48, 1023);
    delay(5);
    st.writeByte(SERVO_IDS[i], 40, 1);
    delay(5);
  }

  // Standby screen waiting for H8 trigger.
  showRunStatus("Waiting H8 Trigger", "Sequence 1");
  waitForH8Trigger();

  // Trigger received: show execution status.
  showRunStatus("Executing", "Sequence 1");
  runSequence1();
  sendH8Done();

  // Return to standby screen for the next cycle.
  while (digitalRead(H8_TRIGGER_PIN) == HIGH) {}
  showRunStatus("Waiting H8 Trigger", "Sequence 2");
  waitForH8Trigger();

  showRunStatus("Executing", "Sequence 2");
  runSequence2();
  sendH8Done();

  showRunStatus("Sequences complete", "Power cycle to reset");
  while (true) {}
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
