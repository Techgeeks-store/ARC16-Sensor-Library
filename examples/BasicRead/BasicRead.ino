/*
  BasicRead - ARC16

  Reads all 16 sensors and prints their numbers to the Serial Monitor.

  On the ARC16 a BIG number means the sensor is looking at black, and a
  SMALL number means it is looking at white. Slide a piece of black tape
  under the bar and watch which number jumps up.

  All 16 sensors share one analog pin. The four select lines tell the
  multiplexer which sensor to send through, and the library does that
  counting for you.

  Open Tools > Serial Monitor and set the speed to 9600.
*/

#include <ARC16.h>

ARC16 sensors;

int select[4] = {2, 3, 4, 5};  // S0, S1, S2, S3
const int ENABLE_PIN = 6;      // active LOW - the library holds it low
const int SIGNAL_PIN = A0;     // every sensor comes through this one pin

int values[16];

void setup() {
  Serial.begin(9600);
  sensors.begin(select, ENABLE_PIN, SIGNAL_PIN);
}

void loop() {
  // read() fills values[] and also remembers the numbers inside the library,
  // which is what lets printRaw() below know what to print.
  sensors.read(values);
  sensors.printRaw();

  delay(200);  // slow the printing down so you can actually read it
}
