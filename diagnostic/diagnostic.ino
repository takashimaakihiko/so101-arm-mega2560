// Simple servo bus diagnostic for Feetech STS3215 on Arduino Mega 2560.
// Connects to Serial1 at 1,000,000 bps and scans IDs 1-6.
// Open the Serial Monitor (115200 bps) to see the results.

#include <SCServo.h>

SMS_STS st;

void setup() {
  Serial.begin(115200);
  while (!Serial);  // wait for Serial Monitor to open (optional, remove if not needed)

  Serial1.begin(1000000, SERIAL_8N1);
  st.pSerial = &Serial1;

  delay(1000);

  Serial.println("===== Servo bus diagnostic =====");

  // One-shot scan of IDs 1-6
  for (int id = 1; id <= 6; id++) {
    int ping = st.Ping(id);

    Serial.print("ID ");
    Serial.print(id);

    if (ping == -1) {
      Serial.println(" : NO RESPONSE (check ID, wiring, baud rate, power)");
    } else {
      int goalPos    = st.readWord(id, 42);   // Goal Position
      int presentPos = st.readWord(id, 56);   // Present Position
      int torqueLimit = st.readWord(id, 48);  // Current Torque Limit

      Serial.print(" : OK  goal=");
      Serial.print(goalPos);
      Serial.print(" present=");
      Serial.print(presentPos);
      Serial.print(" torqueLimit=");
      Serial.println(torqueLimit);
    }
  }

  Serial.println("===== End of scan =====");
  Serial.println("Rotate a servo by hand to see if 'present' changes.");
}

void loop() {
  // Continuously read present and goal positions so you can see whether
  // the encoder value actually changes when you move a joint.
  for (int id = 1; id <= 6; id++) {
    int goalPos    = st.readWord(id, 42);
    int presentPos = st.readWord(id, 56);

    Serial.print("ID");
    Serial.print(id);
    Serial.print(": goal=");
    Serial.print(goalPos);
    Serial.print(" present=");
    Serial.print(presentPos);
    Serial.print("  ");
  }
  Serial.println();
  delay(500);
}
