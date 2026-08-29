// Register scan for Feetech STS3215 on Arduino Mega 2560.
// Reads registers 30..70 from servo ID 1 and prints values that changed.
// Torque is disabled so you can rotate the servo by hand.
// Open Serial Monitor (115200 bps) to see the results.
//
// Wiring (direct half-duplex):
//   Mega TX1 (Pin 18) --[1kΩ]--+-- Servo DATA
//   Mega RX1 (Pin 19) ----------+-- Servo DATA
//   Mega GND ------------------- Servo GND
//   7.4V (NOT Mega 5V!) ------- Servo VCC

#include <SCServo.h>

SMS_STS st;

const int TEST_ID = 1;
const int FIRST_REG = 30;
const int LAST_REG  = 70;
const int REG_COUNT = LAST_REG - FIRST_REG + 1;

int previous[REG_COUNT];

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial1.begin(1000000, SERIAL_8N1);
  st.pSerial = &Serial1;

  delay(1000);

  Serial.println("===== Register scan =====");

  int ping = st.Ping(TEST_ID);
  Serial.print("Ping ID ");
  Serial.print(TEST_ID);
  if (ping == -1) {
    Serial.println(" : NO RESPONSE");
  } else {
    Serial.println(" : OK");
  }

  // Disable torque so the servo can be moved by hand.
  st.writeWord(TEST_ID, 40, 0);
  Serial.println("Torque disabled. Rotate the servo to find Present Position register.");

  // Initialize previous values.
  for (int i = 0; i < REG_COUNT; i++) {
    previous[i] = st.readWord(TEST_ID, FIRST_REG + i);
  }
}

void loop() {
  Serial.println("--- scan ---");

  for (int i = 0; i < REG_COUNT; i++) {
    int reg = FIRST_REG + i;
    int value = st.readWord(TEST_ID, reg);

    if (value != previous[i]) {
      Serial.print("Reg ");
      Serial.print(reg);
      Serial.print(" changed: ");
      Serial.print(previous[i]);
      Serial.print(" -> ");
      Serial.println(value);
      previous[i] = value;
    }
  }

  delay(200);
}
