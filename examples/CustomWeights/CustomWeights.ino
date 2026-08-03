/*
  CustomWeights - ARC16

  Every sensor has a weight: a number saying where it sits on the bar. The
  left-hand sensors get negative numbers, the right-hand ones get positive
  numbers, and readLine() mixes them together to say where the line is.

  The weights the library starts with run evenly from -8 to +8, skipping 0,
  because the middle of a 16-sensor bar falls between sensors 7 and 8.

  Here we spread the outer ones further apart instead. When the line reaches
  an outer sensor the position number shoots up harder, so the robot reacts
  before it loses the line completely.

  setWeights() also works out the new middle for you, so getError() still
  gives 0 when the line is dead centre.

  IMPORTANT: keep every weight between -32 and +32. The position is worked
  out as weight x reading, readings reach 1000, and the answer is stored in
  an int - which on a Nano stops at 32767.

  Open Tools > Serial Monitor and set the speed to 9600.
*/

#include <ARC16.h>

ARC16 sensors;

int select[4] = {2, 3, 4, 5};
const int ENABLE_PIN = 6;
const int SIGNAL_PIN = A0;

int values[16];

// Close together in the middle, further apart at the ends.
int myWeights[16] = {-24, -18, -13, -9, -6, -4, -2, -1,
                       1,   2,   4,  6,  9, 13, 18, 24};

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
  Serial.println(F("Done. Slide the bar across the line and watch."));
}

void loop() {
  sensors.read(values);

  // readLine() is what works the position out. printLine() only shows the
  // last one that was worked out, so call readLine() first.
  sensors.readLine();
  sensors.printLine();

  delay(200);
}
