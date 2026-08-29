// Torque limit test for Feetech STS3215 on Arduino Mega 2560.
// Cycles the SRAM Torque Limit (register 48) on servo ID 1.
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

// Torque limit values to try (0 = free, 1000 = full power).
const int TORQUE_LIMITS[] = {0, 50, 250, 1000};
const int NUM_LIMITS = sizeof(TORQUE_LIMITS) / sizeof(TORQUE_LIMITS[0]);

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial1.begin(1000000, SERIAL_8N1);
  st.pSerial = &Serial1;

  delay(1000);

  Serial.println("===== Torque limit test =====");

  int ping = st.Ping(TEST_ID);
  Serial.print("Ping ID ");
  Serial.print(TEST_ID);
  if (ping == -1) {
    Serial.println(" : NO RESPONSE");
  } else {
    Serial.println(" : OK");
  }

  // Make sure torque is enabled so the torque limit has an effect.
  st.writeWord(TEST_ID, 40, 1);

  Serial.println("Torque is ON. Torque limit (register 48) will cycle:");
  Serial.println("0=free, 50=very soft, 250=soft, 1000=full");
  Serial.println("Try moving the servo by hand at each setting.");
}

void loop() {
  static int idx = 0;
  int torqueLimit = TORQUE_LIMITS[idx];

  st.writeWord(TEST_ID, 48, torqueLimit);
  delay(500);

  int presentPos = st.readWord(TEST_ID, 56);   // Present Position
  int goalPos    = st.readWord(TEST_ID, 42);   // Goal Position
  int actualLimit = st.readWord(TEST_ID, 48);  // Read back torque limit

  Serial.print("TorqueLimit=");
  Serial.print(torqueLimit);
  Serial.print(" actual=");
  Serial.print(actualLimit);
  Serial.print("  goal=");
  Serial.print(goalPos);
  Serial.print(" present=");
  Serial.print(presentPos);

  if (torqueLimit == 0) {
    Serial.println("  *** Try moving: should be LOOSE ***");
  } else if (torqueLimit <= 250) {
    Serial.println("  *** Try moving: should be MOVEABLE with light force ***");
  } else {
    Serial.println("  *** Try moving: should HOLD firmly ***");
  }

  idx = (idx + 1) % NUM_LIMITS;
  delay(3000);
}
