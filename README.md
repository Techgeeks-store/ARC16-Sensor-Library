# ARC16 Sensor Library

The official Arduino library for the **ARC16**, a 16-channel analog IR reflectance sensor array by TECHGEEKS.

Built for line following, maze solving, and edge or sumo-ring detection. Twice the sensors of the ARC8, but it uses **fewer pins** — because all 16 channels share a single analog pin.

---

## Contents

- [What the sensor does](#what-the-sensor-does)
- [How 16 sensors fit on one pin](#how-16-sensors-fit-on-one-pin)
- [Which boards work](#which-boards-work)
- [Power and mounting](#power-and-mounting)
- [Installing the library](#installing-the-library)
- [Wiring](#wiring)
- [Your first sketch](#your-first-sketch)
- [The 3.3V reference option](#the-33v-reference-option)
- [How it all fits together](#how-it-all-fits-together)
- [Full function reference](#full-function-reference)
- [Example sketches](#example-sketches)
- [Warning messages and what to do about them](#warning-messages-and-what-to-do-about-them)
- [Tuning the PID](#tuning-the-pid)
- [Things to know](#things-to-know)
- [License](#license)

---

## What the sensor does

The ARC16 has 16 infrared sensors set in a curve rather than a straight line. Each one shines invisible light at the floor and measures how much bounces back.

The curve is deliberate. It gives the array a wider field of view and lets it pick up sharp turns, acute angles, intersections, line gaps, circular loops and track inversions that a straight array would run straight past.

Each sensor gives you a number from **0 to 1023**:

| Reading | What it means |
|---|---|
| **High** (e.g. 900) | Dark surface. Black tape soaks up the light, so little comes back. |
| **Low** (e.g. 100) | Light surface. White floor bounces the light straight back. |

That is the one rule everything else is built on: **bigger number means blacker**.

---

## How 16 sensors fit on one pin

An Arduino does not have 16 analog pins. So the ARC16 uses a **multiplexer** — think of it as a rotary switch with 16 positions. You tell it which sensor you want, and it connects that one sensor to a single shared wire.

You choose the sensor by counting in binary on four **select lines**:

```
sensor  0  ->  S3 S2 S1 S0  =  0 0 0 0
sensor  1  ->                  0 0 0 1
sensor  5  ->                  0 1 0 1
sensor 15  ->                  1 1 1 1
```

**The library does all of that for you.** You never touch the select lines yourself — `read()` walks through all 16 and hands you the numbers in order.

Two things worth knowing:

- **It takes about 1.7 milliseconds** to read all 16 sensors, because each one needs its own analog reading. That still lets your loop run around 500 times a second, which is plenty for a line follower.
- **There is a 5-microsecond pause** built into each channel change, to let the multiplexer settle before reading. Without it, the previous sensor's value bleeds into the next one. It is 5 millionths of a second and stalls nothing.

---

## Which boards work

| Board | Works? | Why |
|---|---|---|
| **Arduino Nano** | Yes — recommended | Only needs 6 pins |
| **Arduino Uno** | **Yes** | Also only needs 6 pins — unlike ARC8, the Uno handles ARC16 fine |
| **Arduino Pro Mini** | Yes | Same 6 pins |
| **Mega, ESP32, STM32, etc.** | Should work, not yet tested | See below |

> If you have used the ARC8 before: that library needed 8 analog pins, which an Uno does not have. ARC16 needs only **one** analog pin, so an Uno is perfectly happy with it despite there being twice as many sensors.

**About other boards:** the library is plain, portable C++ — no direct register poking, no AVR-only headers. Nothing stops it running on an ESP32. That said, it has only been built and verified for AVR so far, so treat other boards as untested. Check that your board's `analogRead()` also returns 0–1023; some return 0–4095, which would throw the calibration off.

---

## Power and mounting

| | |
|---|---|
| Input voltage | 5V DC only |
| Input current | about 310 mA |
| Weight | 8 g |
| Working height | 3 to 10 mm above the track |
| Mounting | 2 × M2 screw holes |

**The ARC16 wants 5V, not 3.3V.** If you have used an ARC8 before, note the difference: the ARC8 will run anywhere from 3.3V to 5V, the ARC16 will not.

310 mA is a lot for a sensor, roughly double what an ARC8 draws. Do not try to run it from an Arduino pin. Give it a proper 5V supply from an MP1584, an LM7805 or similar. If you are using the Blueprint 01 Robot Controller Board, its onboard LM7805 is rated for this and the sensor rail gives you a 5V pin to plug into.

Mount the array with the two M2 screw holes and set the height anywhere between 3 and 10 mm. Which height works best depends on your track, so try a few and watch the readings.

There is an indicator LED on the board. It lights up steadily when the sensor has power.

---

## Installing the library

### Option 1 — Add .ZIP Library (recommended)

1. Download **`ARC16.zip`** from the [Releases page](https://github.com/Techgeeks-store/ARC16-Sensor-Library/releases).
2. Open the Arduino IDE.
3. Go to **Sketch → Include Library → Add .ZIP Library…**
4. Select the `ARC16.zip` file you downloaded.

Restart the IDE, and the examples appear under **File → Examples → ARC16**.

### Option 2 — Download the whole repository

1. Click the green **Code** button at the top of this page, then **Download ZIP**.
2. Unzip it. You will get a folder called **`ARC16-Sensor-Library-main`**.
3. **Rename that folder to just `ARC16`.** This step matters — the Arduino IDE wants the folder name to match the library name, and it does not like the dashes.
4. Move the `ARC16` folder into your sketchbook's `libraries` folder:
   - **Windows:** `Documents\Arduino\libraries\`
   - **macOS:** `~/Documents/Arduino/libraries/`
   - **Linux:** `~/Arduino/libraries/`
5. Restart the Arduino IDE.

### Option 3 — Git clone

```bash
git clone https://github.com/Techgeeks-store/ARC16-Sensor-Library.git ARC16
```

Run that inside your `Arduino/libraries` folder. Note the `ARC16` on the end — it clones into a correctly named folder for you.

### Checking it worked

Open **File → Examples → ARC16 → BasicRead**. If that sketch appears and compiles, you are ready.

---

## Wiring

Six pins, plus power:

| ARC16 pin | Arduino | In your code |
|---|---|---|
| S0 | A0 | `select[0]` |
| S1 | A1 | `select[1]` |
| S2 | A2 | `select[2]` |
| S3 | A3 | `select[3]` |
| E (enable) | A4 | `enable` |
| SIG (signal) | A5 | `signal` |
| VCC | 5V | — |
| GND | GND | — |

```cpp
int select[4] = {A0, A1, A2, A3};   // S0, S1, S2, S3 - order matters
sensors.begin(select, A4, A5);      // ...then enable, then signal
```

A0 to A3 and A4 are being used as ordinary digital outputs here, which the Nano is happy to do. Only SIG needs to be a real analog input.

Everything sits on A0 to A5, so an ARC16 plugs straight into the sensor rail of the Blueprint 01 Robot Controller Board with nothing left over.

The **order inside `select[]` is what matters**: S0 first, S3 last. Get that wrong and your sensors come back shuffled.

**The enable pin is active LOW.** The library drives it LOW in `begin()` and leaves it there for the whole run, so the array is always switched on. You never have to think about it.

> Watch the numbering: your board may label the sensors **1 to 16**, but in code they are always **0 to 15**. Asking for sensor `16` gets you `ARC16: bad sensor index`.
>
> **Sensor 0 is the one on the right, sensor 15 is on the left.** The multiplexer channel decides that, so it is the same on every board and you cannot change it by rewiring.

### If you also need I2C

A4 and A5 are the Nano's I2C pins (SDA and SCL). While the ARC16 is using them, you cannot run an I2C device such as an MPU6050 at the same time. If you need both, move two pins:

| ARC16 pin | Instead of | Use |
|---|---|---|
| E (enable) | A4 | any spare digital pin, e.g. D13 |
| SIG (signal) | A5 | A6 or A7 |

```cpp
int select[4] = {A0, A1, A2, A3};
sensors.begin(select, 13, A6);      // enable on D13, signal on A6
```

That frees A4 and A5 for I2C. A6 and A7 are analog-only on the Nano, which is exactly what SIG needs anyway, so nothing is wasted. On the Blueprint 01 board, D13 is the digital pin left free by the motors, buttons and LEDs.

---

## Your first sketch

```cpp
#include <ARC16.h>

ARC16 sensors;

int select[4] = {A0, A1, A2, A3};
int values[16];

void setup() {
  Serial.begin(9600);              // you must do this yourself
  sensors.begin(select, A4, A5);   // select pins, enable pin, signal pin
}

void loop() {
  sensors.read(values);          // read all 16 sensors
  sensors.printRaw();            // print them
  delay(200);
}
```

Open **Tools → Serial Monitor**, set it to **9600**, and slide a piece of black tape under the bar.

### The one rule to remember

**`read(values)` comes first, every time round the loop.**

Almost every other function works from the batch of numbers that `read()` collected. Call them without reading first and you get `ARC16: call read() first` in the Serial Monitor and a safe `0` back, rather than nonsense.

---

## The 3.3V reference option

> ### ⚠️ Read this whole section before wiring anything. Getting it wrong can permanently destroy your Arduino.

On a cheap, faded, or shiny track, the readings can bunch up near the top of the range and become hard to tell apart. Lowering the Arduino's measuring range from 5V to 3.3V spreads them back out.

To use it you must do **two** things, and they must match:

1. **Wire the AREF pin to the 3.3V pin** on your Nano with a short jumper.
2. **Pass `true`** as the fourth thing in `begin()`.

```cpp
sensors.begin(select, 6, A0, true);   // <- the true is what switches it
```

| AREF wired to 3.3V? | `true` passed? | Result |
|---|---|---|
| No | No | Normal 5V range. Fine — this is the default. |
| Yes | Yes | 3.3V range. Fine, and much better on poor tracks. |
| **Yes** | **No** | **CAN PERMANENTLY DESTROY THE CHIP.** |
| No | Yes | No damage, but AREF floats and every reading is rubbish. |

**Why the third row is dangerous:** if AREF is wired to 3.3V while the Arduino still thinks it should use its own internal 5V reference, the first `analogRead()` shorts one against the other, straight through the chip.

The library protects you as far as it can — when you pass `true`, it switches the reference **before it reads anything at all**, which is the only safe order. But it cannot see your wiring. Matching the jumper to the flag is your job.

**Never connect anything other than 3.3V to AREF.** Not 5V, not a battery, not a potentiometer.

See the **ExternalReference** example for a working sketch.

---

## How it all fits together

### 1. Calibrating

Every sensor is slightly different. One might read 120 on your white floor and 800 on the tape, while the sensor beside it reads 200 and 950. Compare those directly and the brighter sensor always looks closer to the line — so the robot drifts towards it forever.

Calibrating fixes that. You sweep the bar across the line for a few seconds while `calibrate()` runs, and it remembers the lightest and darkest each sensor saw. After that, every sensor is stretched onto the **same 0 to 1000 scale**.

```cpp
unsigned long start = millis();
while (millis() - start < 3000) {   // 3 seconds
  sensors.read(values);
  sensors.calibrate();              // call it over and over while you sweep
}
```

Sweep so that **every** sensor sees both floor and tape. With 16 sensors the bar is wide, so sweep wider and slower than you would with an 8-sensor array.

### 2. On the line, or not

```cpp
sensors.setLine(BLACK);       // black tape on a white floor (the default)
sensors.setThreshold(500);

bool onTheLine[16];
sensors.readDigital(onTheLine);      // true / false for each sensor
int howMany = sensors.countOnLine();
```

White line on a dark floor? `sensors.setLine(WHITE);` flips the comparison, at any time, even mid-loop. That also covers track inversion: if your track flips from black-on-white to white-on-black partway round, call `setLine(WHITE)` the moment you detect it and everything downstream follows on the very next read.

> **`readDigital()` is not `digitalRead()`.** The ARC16 can be read with `digitalRead()` on the SIG pin, but the manual warns that anywhere between about 1.5V and 3V the output is unpredictable, so on a poor track the value flickers. This library never does that. `readDigital()` takes the full analog reading, stretches it with your calibration, then compares it to the threshold. You get a steady yes or no even where `digitalRead()` would jump around.

### 3. Where is the line?

Each sensor has a **weight** saying where it sits on the array:

```
sensor:  0  1  2  3  4  5  6  7   8  9 10 11 12 13 14 15
weight: 16 14 12  8  6  4  2  1  -1 -2 -4 -6 -8 -12 -14 -16
        <-------- right        left -------->
```

**Sensor 0 is the one on the right and sensor 15 is on the left.** That is fixed by the hardware, not by how you wire it, so the right-hand sensors carry the positive numbers.

There is no zero, because with an even number of sensors the true middle falls between sensors 7 and 8.

The numbers are not evenly spaced. They widen towards the ends to match the curve of the array: the outer sensors sit further out to the side, so they move the position further when the line reaches them.

The position is a weighted average of the sensors that can see the line. Because a sensor half-over the tape reads lower than one sitting right on it, sensors that barely touch the line only tug the answer a little, which makes the number slide smoothly as you drift.

| `readLine()` returns | Meaning |
|---|---|
| about `-16000` | line is hard over to the **left** |
| `0` | line is **dead centre** |
| about `+16000` | line is hard over to the **right** |

`getError()` is the same thing measured from the centre point. With the default weights the centre is 0, so both give the same answer — but if you set your own weights, `getError()` is the one that stays honest.

**If every sensor loses the line,** the position stays at the last value it was sure about, so the robot keeps turning the way it was already turning. Use `lineFound()` to check.

### 4. Steering

```cpp
sensors.setPID(0.008, 0.0, 0.005);   // kp, ki, kd

void loop() {
  sensors.read(values);
  int correction = sensors.getCorrection();

  int leftSpeed  = BASE_SPEED + correction;
  int rightSpeed = BASE_SPEED - correction;
  // send those to your motors
}
```

Positive means the line drifted right, so speed the left wheel up.

---

## Full function reference

### Setting up

| Function | What it does |
|---|---|
| `ARC16()` | Creates the object with sensible defaults. |
| `void begin(int select[4], int enable, int signal, bool useExternalRef = false)` | Tells the library your pins. Drives the enable pin LOW. Leave the last argument out unless you have wired AREF — see the warning above. |

### Reading

| Function | What it does |
|---|---|
| `void read(int values[16])` | Reads all 16 sensors into your array, and remembers them for every other function. **Call this first.** Takes about 1.7 ms. |
| `int read(int i)` | Reads sensor `i` right now and returns 0–1023. Returns `-1` for a bad index. |

### Calibration

| Function | What it does |
|---|---|
| `void calibrate()` | Updates the lightest/darkest seen so far. Call repeatedly while sweeping. |
| `void readCalibrated(int values[16])` | Fills your array with the 0–1000 stretched values. |
| `int readCalibrated(int i)` | Reads sensor `i` right now, stretched to 0–1000. `-1` for a bad index. |
| `int getMin(int i)` | The lightest reading sensor `i` has seen. `-1` for a bad index. |
| `int getMax(int i)` | The darkest reading sensor `i` has seen. `-1` for a bad index. |

### On the line or not

| Function | What it does |
|---|---|
| `void setThreshold(int value)` | The dividing number, 0–1000. Default 500. Out-of-range values get pulled into range. |
| `void readDigital(bool values[16])` | Fills your array with `true`/`false` per sensor. |
| `bool onLine(int i)` | Checks sensor `i` right now. `false` for a bad index. |
| `int countOnLine()` | How many sensors can see the line, 0–16. |

### Line position

| Function | What it does |
|---|---|
| `void setLine(int mode)` | `BLACK` or `WHITE`. Anything else is ignored and warns. Takes effect immediately. |
| `void setWeights(int w[16])` | Your own weights. Also recalculates the centre point. **Keep every weight between −32 and +32** — see below. |
| `int readLine()` | Where the line is. Roughly −16000 to +16000 with default weights. Positive means the line went right. |
| `int getError()` | How far off centre, which is what you steer on. |
| `bool lineFound()` | `true` if at least one sensor can see the line. |

### Steering

| Function | What it does |
|---|---|
| `void setPID(float kp, float ki, float kd)` | Sets the three steering numbers. |
| `int getCorrection()` | The steering number. Positive = line drifted right. |

### Printing (for debugging)

| Function | What it does |
|---|---|
| `void printRaw()` | Prints the 16 raw readings, space separated. |
| `void printCalibrated()` | Prints the 16 stretched 0–1000 readings. |
| `void printLine()` | Prints `line: <position> error: <error>`. |

> Because sensor `0` is the right-hand end, `printRaw()`, `printCalibrated()` and `readDigital()` all run **right to left**. Slide tape under the right-hand end of the array and the first number on screen is the one that moves.
>
> The print functions and every warning use `Serial`, but the library **never** calls `Serial.begin()` for you. Do that yourself in `setup()`, or you will see nothing at all.
>
> `printLine()` shows the last position that was worked out — it does not recalculate. Call `readLine()` or `getError()` first if you want it fresh.

### Constants

| Name | Value | Used with |
|---|---|---|
| `BLACK` | 1 | `setLine(BLACK)` — dark line on a light floor |
| `WHITE` | 0 | `setLine(WHITE)` — light line on a dark floor |

---

## Example sketches

All under **File → Examples → ARC16**.

| Example | What it teaches |
|---|---|
| **BasicRead** | Reading all 16 sensors and printing them. Start here. |
| **Calibration** | Sweeping to calibrate, then reading on the tidy 0–1000 scale. |
| **DigitalRead** | Turning readings into simple yes/no, and counting sensors on the line. |
| **LineFollowerPID** | A complete line follower with motor driver code. |
| **CustomWeights** | Changing the weights and seeing how the position number changes. |
| **ExternalReference** | The 3.3V AREF option, with the wiring warning in full. |

---

## Warning messages and what to do about them

If something is wrong, the library says so in the Serial Monitor in plain English.

**Each message prints only once per run.** Without that, a single wiring mistake would fill your Serial Monitor thousands of times a second and hide everything useful.

| Message | What went wrong | How to fix it |
|---|---|---|
| `ARC16: call read() first` | You used a function that needs sensor data, but `read(values)` has never run. | Put `sensors.read(values);` at the top of your `loop()`. |
| `ARC16: pin used twice` | Two of your six pins are the same number. | Check `select[]`, the enable pin and the signal pin for a typo. |
| `ARC16: bad sensor index` | You asked for a sensor that does not exist. | Sensors are numbered **0 to 15**, not 1 to 16. |
| `ARC16: not calibrated yet` | You used a calibrated reading before ever calling `calibrate()`. | Run the calibration sweep in `setup()`. |
| `ARC16: sensor not calibrated properly` | One sensor's light and dark readings are almost the same. | That sensor never saw both floor and tape — sweep wider. Or it may be faulty, or too far from the ground. |
| `ARC16: threshold out of range` | `setThreshold()` got a value outside 0–1000. | The scale is 0–1000, not 0–1023. Try 500. |
| `ARC16: invalid line mode` | `setLine()` got something other than `BLACK` or `WHITE`. | Use the words `BLACK` or `WHITE`. |

### Something is wrong but there is no message

Not every mistake can be detected, so here are the usual suspects:

| What you see | Almost certainly |
|---|---|
| **All 16 numbers are identical** | You forgot `sensors.begin(...)` in `setup()`, or the enable pin is not connected. |
| **The sensors seem shuffled** — the wrong one lights up | Your `select[]` order is wrong. It must be S0, S1, S2, S3. |
| **Nothing at all in the Serial Monitor** | No `Serial.begin(9600);` in `setup()`, or the Serial Monitor's speed box does not match. |
| **All numbers stuck at 0, or stuck at 1023** | Power or wiring. Check VCC, GND, and the signal wire. |
| **Wild, jumpy readings after using the AREF option** | AREF is not actually jumpered to 3.3V, but you passed `true`. |
| **Numbers barely change** between tape and floor | The array is at the wrong height. It works between 3 and 10 mm off the track. Move it up or down until the gap between white and black readings is widest. |
| **The indicator LEDs are off, or flickering** | Off means no power at all. Flickering means the supply is not steady: loose jumper wires, a low battery, or too little current. The ARC16 draws about 310 mA, so this is worth checking first. Solder the wires to the terminals and put a capacitor near the 5V supply. |
| **You think one sensor has died** | Point a phone camera at the board and look at the screen. Every sensor should show a faint purple dot. Infrared is invisible to your eye but the camera picks it up, so a sensor with no dot is not getting power. Check the E pin is being held LOW as well. |
| **Position jumps around wildly** | Not calibrated, or the sweep missed some sensors. Sweep again, slower and wider. |
| **The robot steers the wrong way** | Your motors are mirrored. Swap the `+` and `−` in the two speed lines. |
| **It follows a white line badly** | Add `sensors.setLine(WHITE);` in `setup()`. |

---

## Tuning the PID

Do this in order, and change **one number at a time**.

1. **Start with `setPID(0.008, 0.0, 0.0)`** — only `kp`, the other two at zero.
2. **Raise `kp`** until the robot follows the line. Too low and it drifts off corners; too high and it snakes violently.
3. **Back `kp` off slightly**, until it follows with only a gentle wobble.
4. **Add a little `kd`**, maybe `0.002` to `0.01`. This damps the wobble.
5. **Leave `ki` at 0.** You only need it if the robot consistently sits to one side, and even then keep it tiny.

> **Coming from the ARC8?** The ARC16 position range is four times as wide, −16000 to +16000 instead of −4000 to +4000, so start with roughly **a quarter** of the `kp` you used there.

| Symptom | Try |
|---|---|
| Drifts off on corners | Raise `kp` |
| Snakes wildly side to side | Lower `kp`, or raise `kd` |
| Wobbles gently but constantly | Raise `kd` |
| Always rides slightly off to one side | A tiny `ki` |
| Overshoots then swings back hard | Lower `ki` (or set it to 0) |

Faster robots need lower `kp`. If you raise your base speed, expect to retune.

---

## Things to know

- **Always exactly 16 sensors.** There is no sensor-count setting.
- **Keep custom weights between −32 and +32.** The position is `weight × reading`, readings reach 1000, and the answer is stored in an `int` — which on an Uno or Nano tops out at 32767. A weight of 33 or more can wrap round and send the robot the wrong way. The defaults (−8 to +8) and the CustomWeights example (−24 to +24) are inside the limit. What matters is the *ratio* between weights, not their size.
- **The enable pin stays LOW the whole time.** There is no function to switch the array off.
- **`BLACK` and `WHITE` are global names.** Many display libraries (Adafruit GFX, most TFT and OLED libraries) define their own. Using one alongside ARC16 will produce a "redefined" warning.
- **Calibration only narrows.** The remembered light/dark values can only get further apart. Moving to a completely different surface means restarting the sketch before recalibrating.
- **The PID never resets.** The running total behind `ki` and the previous error carry on for the whole run.
- **No EEPROM.** Calibration is forgotten at power-off, so you calibrate each time you switch on. That is worth doing anyway. Room lighting does not bother the ARC16 much, but the track surface, the sensor height and the battery voltage all change between sessions, and those do move the readings.
- **No memory tricks.** No `malloc`, no `new`, no `String`. Everything is fixed size, so it cannot fragment your RAM mid-race.
- **No `delay()` inside the library.** The only blocking wait is the 5-microsecond multiplexer settling pause, which is unavoidable.

### Memory use

Measured on an Arduino Nano, compiled with the real Arduino AVR core:

| Example | Flash | RAM |
|---|---|---|
| BasicRead | 3,940 bytes (12.0%) | 402 bytes (19.6%) |
| ExternalReference | 4,804 bytes (14.7%) | 402 bytes (19.6%) |
| Calibration | 4,996 bytes (15.2%) | 402 bytes (19.6%) |
| DigitalRead | 5,068 bytes (15.5%) | 418 bytes (20.4%) |
| CustomWeights | 5,280 bytes (16.1%) | 434 bytes (21.2%) |
| LineFollowerPID | 5,934 bytes (18.1%) | 402 bytes (19.6%) |

Plenty of room left for the rest of your robot.

---

## License

MIT — see [LICENSE](LICENSE). Use it, change it, build products with it. Just keep the copyright notice.

Copyright © 2026 TECHGEEKS
