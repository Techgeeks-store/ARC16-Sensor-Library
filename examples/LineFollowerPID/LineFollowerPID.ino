/*
  LineFollowerPID - ARC16

  A complete line follower.

  The library works out how far off the line the robot is, and
  getCorrection() turns that into one steering number. A positive number
  means the line has drifted to the right, so speed the left wheel up and
  slow the right wheel down. A negative number means the opposite.

  How to tune it:
    1. Start with kp small and ki and kd both 0.
    2. Make kp bigger until the robot follows the line but wobbles.
    3. Add a little kd to calm the wobble down.
    4. Leave ki at 0 unless the robot always sits slightly off to one side.

  With 16 sensors the position runs from about -8000 to +8000, so kp wants
  to be about half of what you would use on an 8-sensor bar.

  The motor pins below are for an L298N driver. Change them to match yours.
*/

#include <ARC16.h>

ARC16 sensors;

int select[4] = {2, 3, 4, 5};
const int ENABLE_PIN = 6;
const int SIGNAL_PIN = A0;

int values[16];

// Left motor
const int ENA = 9;   // must be a PWM pin
const int IN1 = 7;
const int IN2 = 8;

// Right motor
const int ENB = 10;  // must be a PWM pin
const int IN3 = 11;
const int IN4 = 12;

const int BASE_SPEED = 120;  // how fast the robot goes on a straight line

void setup() {
  Serial.begin(9600);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  sensors.begin(select, ENABLE_PIN, SIGNAL_PIN);
  sensors.setLine(BLACK);
  sensors.setPID(0.015, 0.0, 0.01);

  // Sweep the robot across the line by hand while these 3 seconds run.
  Serial.println(F("Sweep the robot across the line now!"));
  unsigned long start = millis();
  while (millis() - start < 3000) {
    sensors.read(values);
    sensors.calibrate();
  }
  Serial.println(F("Go!"));
}

void loop() {
  // Read the sensors as fast as we can. No delay() here - a line follower
  // needs to notice it is drifting straight away. Reading all 16 sensors
  // takes about 1.7 milliseconds, so this loop runs around 500 times a
  // second, which is plenty.
  sensors.read(values);

  // If every sensor has lost the line, the library keeps giving the last
  // position it was sure about, so the robot carries on turning the way it
  // was already turning and usually finds the line again.
  int correction = sensors.getCorrection();

  drive(BASE_SPEED + correction, BASE_SPEED - correction);
}

// Sends a speed to each motor. Both wheels always go forwards here, so any
// speed below 0 is treated as "stopped" and anything above 255 as "flat out".
void drive(int left, int right) {
  left  = constrain(left,  0, 255);
  right = constrain(right, 0, 255);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, left);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, right);
}
