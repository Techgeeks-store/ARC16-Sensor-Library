/*
  DigitalRead - ARC16

  Turns each sensor into a simple yes or no answer: am I on the line?

  Anything above the threshold counts as being on the line when the line is
  black. Print a 1 for yes and a 0 for no, and you get a little picture of
  where the tape is sitting under the bar - 16 characters wide.

  Calibrate first (sweep the bar for the first 3 seconds), then slide the
  bar sideways and watch the 1s move.

  Open Tools > Serial Monitor and set the speed to 9600.
*/

#include <ARC16.h>

ARC16 sensors;

int select[4] = {A0, A1, A2, A3};  // S0, S1, S2, S3
const int ENABLE_PIN = A4;     // E pin, active LOW - the library holds it low
const int SIGNAL_PIN = A5;     // SIG - every sensor comes through this one pin

int  values[16];
bool onTheLine[16];

void setup() {
  Serial.begin(9600);
  sensors.begin(select, ENABLE_PIN, SIGNAL_PIN);

  sensors.setLine(BLACK);     // black tape on a white floor
  sensors.setThreshold(500);  // halfway up the 0 to 1000 scale

  Serial.println(F("Sweep the sensors across the line now!"));
  unsigned long start = millis();
  while (millis() - start < 3000) {
    sensors.read(values);
    sensors.calibrate();
  }
  Serial.println(F("Done."));
}

void loop() {
  sensors.read(values);
  sensors.readDigital(onTheLine);

  for (int i = 0; i < 16; i++) {
    Serial.print(onTheLine[i] ? 1 : 0);
  }

  Serial.print(F("  sensors on the line: "));
  Serial.println(sensors.countOnLine());

  delay(200);
}
