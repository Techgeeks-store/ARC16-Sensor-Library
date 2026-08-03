/*
  ExternalReference - ARC16

  Sometimes, on a cheap or faded track, the sensors give readings that are
  all bunched up near the top and hard to tell apart. Lowering the Arduino's
  measuring range from 5V to 3.3V spreads those readings back out and makes
  the difference between white and black much clearer.

  ==========================================================================
   READ THIS BEFORE YOU RUN THE SKETCH - YOU CAN DAMAGE YOUR ARDUINO
  ==========================================================================

  To use this you must do TWO things, and they must match:

    1. Wire the AREF pin to the 3.3V pin on your Nano with a short jumper.
    2. Pass `true` as the last thing in begin(), as shown below.

  If you wire AREF to 3.3V but leave the `true` out, the Arduino's own
  built-in 5V reference gets shorted against the 3.3V you just connected,
  the first time it reads a sensor. That can permanently kill the chip.

  If you pass `true` but do NOT wire AREF, nothing breaks, but the AREF pin
  is left floating and every reading you get will be rubbish.

    AREF wired?   true passed?   Result
    -----------   ------------   ----------------------------------------
    no            no             normal 5V. fine.
    yes           yes            3.3V range. fine, and better on bad tracks.
    yes           no             CAN DESTROY THE CHIP. never do this.
    no            yes            no damage, but all readings are rubbish.

  Never connect anything other than 3.3V to AREF. Not 5V, not a battery.

  ==========================================================================

  Everything else works exactly as in the other examples. Readings still
  come back as 0 to 1023, and after calibrating they still land on the
  0 to 1000 scale - they are simply better spread out.

  Open Tools > Serial Monitor and set the speed to 9600.
*/

#include <ARC16.h>

ARC16 sensors;

int select[4] = {A0, A1, A2, A3};  // S0, S1, S2, S3
const int ENABLE_PIN = A4;     // E pin, active LOW - the library holds it low
const int SIGNAL_PIN = A5;     // SIG - every sensor comes through this one pin

int values[16];

void setup() {
  Serial.begin(9600);

  // The `true` on the end is the whole point of this example. The library
  // switches the reference over before it reads anything at all, which is
  // the only safe order to do it in.
  sensors.begin(select, ENABLE_PIN, SIGNAL_PIN, true);

  Serial.println(F("Running on the 3.3V reference."));
  Serial.println(F("If these numbers look wrong, check your AREF jumper."));

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

  Serial.print(F("raw: "));
  sensors.printRaw();

  Serial.print(F("cal: "));
  sensors.printCalibrated();
  Serial.println();

  delay(400);
}
