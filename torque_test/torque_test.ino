// Torque toggle test for Feetech STS3215 on Arduino Mega 2560.
// Connects to Serial1 at 1,000,000 bps and toggles torque on servo ID 1.
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

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial1.begin(1000000, SERIAL_8N1);
  st.pSerial = &Serial1;

  delay(1000);

  Serial.println("===== Torque toggle test =====");

  int ping = st.Ping(TEST_ID);
  Serial.print("Ping ID ");
  Serial.print(TEST_ID);
  if (ping == -1) {
    Serial.println(" : NO RESPONSE");
  } else {
    Serial.println(" : OK");
  }

  Serial.println("Torque will toggle every 3 seconds.");
  Serial.println("0 = torque OFF (should be loose), 1 = torque ON (should hold)");
}

void loop() {
  static bool torqueOn = false;

  // Disable/enable torque
  int enable = torqueOn ? 1 : 0;
  int result = st.writeWord(TEST_ID, 40, enable);  // Torque Enable register

  Serial.print("Torque=");
  Serial.print(enable);
  Serial.print(" writeWord result=");
  Serial.print(result);

  delay(500);

  int presentPos = st.readWord(TEST_ID, 56);   // Present Position
  int goalPos    = st.readWord(TEST_ID, 42);   // Goal Position
  int torqueLimit = st.readWord(TEST_ID, 48);  // Torque Limit

  Serial.print("  goal=");
  Serial.print(goalPos);
  Serial.print(" present=");
  Serial.print(presentPos);
  Serial.print(" torqueLimit=");
  Serial.println(torqueLimit);

  Serial.print(torqueOn ? "*** FEEL SERVO: should be LOOSE NOW ***" : "*** FEEL SERVO: should HOLD NOW ***");
  Serial.println();

  torqueOn = !torqueOn;
  delay(3000);
}
