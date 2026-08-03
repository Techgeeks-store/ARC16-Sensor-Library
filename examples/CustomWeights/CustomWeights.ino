/*
  CustomWeights - ARC16

  Every sensor has a weight: a number saying where it sits on the array.
  readLine() mixes them together to say where the line is.

  Sensor 0 is the one on the RIGHT and sensor 15 is on the LEFT. The
  right-hand sensors get negative numbers and the left-hand ones positive,
  so a negative position means the line has gone right.

  The weights the library starts with are spread out towards the ends:

      sensor:   0   1   2  3  4  5  6  7   8  9 10 11 12 13 14 15
      weight: -16 -14 -12 -8 -6 -4 -2 -1   1  2  4  6  8 12 14 16
              <-------- right       left -------->

  They are not evenly spaced, because the array is curved. The outer
  sensors sit further out to the side, so they move the position number
  further when the line reaches them.

  This sketch tries evenly spaced weights instead, running -8 to +8. If your
  robot reacts too sharply as the line moves out towards the edges, even
  spacing is worth a try. If it reacts too late, go the other way and spread
  the outer numbers wider still.

  setWeights() also works out the new middle for you, so getError() still
  gives 0 when the line is dead centre.

  IMPORTANT: keep every weight between -32 and +32. The position is worked
  out as weight x reading, readings reach 1000, and the answer is stored in
  an int - which on a Nano stops at 32767.

  Open Tools > Serial Monitor and set the speed to 9600.
*/

#include <ARC16.h>

ARC16 sensors;

int select[4] = {A0, A1, A2, A3};  // S0, S1, S2, S3
const int ENABLE_PIN = A4;     // E pin, active LOW - the library holds it low
const int SIGNAL_PIN = A5;     // SIG - every sensor comes through this one pin

int values[16];

// Evenly spaced, right-hand side negative.
int myWeights[16] = {-8, -7, -6, -5, -4, -3, -2, -1,
                      1,  2,  3,  4,  5,  6,  7,  8};

void setup() {
  Serial.begin(9600);
  sensors.begin(select, ENABLE_PIN, SIGNAL_PIN);

  sensors.setLine(BLACK);
  sensors.setWeights(myWeights);

  Serial.println(F("Sweep the sensors across the line now!"));
  unsigned long start = millis();
  while (millis() - start < 3000) {
    sensors.read(values);
    sensors.calibrate();
  }
  Serial.println(F("Done. Slide the array across the line and watch."));
}

void loop() {
  sensors.read(values);

  // readLine() is what works the position out. printLine() only shows the
  // last one that was worked out, so call readLine() first.
  sensors.readLine();
  sensors.printLine();

  delay(200);
}
