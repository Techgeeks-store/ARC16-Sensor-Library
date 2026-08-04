#include "ARC16.h"

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

ARC16::ARC16() {
  for (int i = 0; i < 4; i++) _select[i] = 0;
  _enable = 0;
  _signal = 0;

  for (int i = 0; i < 16; i++) {
    _raw[i] = 0;

    // Start the smallest at the highest possible reading and the biggest at
    // the lowest, so the very first real reading replaces both of them.
    _calMin[i] = 1023;
    _calMax[i] = 0;
  }

  // How much each sensor counts towards the line position.
  //
  // Sensor 0 is the one on the RIGHT of the array and sensor 15 is on the
  // LEFT. The right-hand half counts negative and the left-hand half
  // positive, so a NEGATIVE position means the line has gone right.
  //
  // That is the same way round as the Spirit robot firmware, which is why
  // the steering line reads speedLeft = base - correction.
  //
  // There is no zero, because with an even number of sensors the true
  // middle falls between sensors 7 and 8. The numbers spread out towards
  // the ends rather than climbing evenly, which matches the curve of the
  // array: the outer sensors sit further out to the side, so they should
  // move the position further. It all balances to a dead centre of 0.
  _weights[0]  = -16;  _weights[1]  = -14;  _weights[2]  = -12;  _weights[3]  =  -8;
  _weights[4]  =  -6;  _weights[5]  =  -4;  _weights[6]  =  -2;  _weights[7]  =  -1;
  _weights[8]  =   1;  _weights[9]  =   2;  _weights[10] =   4;  _weights[11] =   6;
  _weights[12] =   8;  _weights[13] =  12;  _weights[14] =  14;  _weights[15] =  16;

  _hasRead       = false;
  _calibrated    = false;
  _threshold     = 500;
  _setpoint      = 0;
  _lineIsBlack   = true;
  _lastLineValue = 0;

  _warnedPinTwice       = false;
  _warnedBadIndex       = false;
  _warnedThreshold      = false;
  _warnedNoRead         = false;
  _warnedNotCalibrated  = false;
  _warnedBadCalibration = false;
  _warnedLineMode       = false;
}

void ARC16::begin(int select[4], int enable, int signal, bool useExternalRef) {
  // This has to happen before anything reads the analog pin. If AREF is
  // wired to 3.3V and the board is still set to its normal 5V reference,
  // the first reading shorts one against the other and can wreck the chip.
  if (useExternalRef) analogReference(EXTERNAL);

  for (int i = 0; i < 4; i++) {
    _select[i] = select[i];
    pinMode(_select[i], OUTPUT);
    digitalWrite(_select[i], LOW);  // start on a known sensor
  }

  // The enable pin is active LOW, so LOW switches the multiplexer on. It is
  // set here once and never touched again.
  _enable = enable;
  pinMode(_enable, OUTPUT);
  digitalWrite(_enable, LOW);

  _signal = signal;
  pinMode(_signal, INPUT);

  // Using the same pin twice is a common typo, so say something about it.
  int used[6] = {_select[0], _select[1], _select[2], _select[3], _enable, _signal};
  for (int i = 0; i < 6; i++) {
    for (int j = i + 1; j < 6; j++) {
      if (used[i] == used[j] && !_warnedPinTwice) {
        Serial.println(F("ARC16: pin used twice"));
        _warnedPinTwice = true;
      }
    }
  }
}

// ---------------------------------------------------------------------------
// Reading one channel of the multiplexer
// ---------------------------------------------------------------------------

// Picks one sensor and reads it.
//
// The four select lines spell the sensor number out in binary: S0 is the
// ones, S1 the twos, S2 the fours, S3 the eights. So sensor 5 (0101) means
// S0 high, S1 low, S2 high, S3 low. bitRead pulls out one bit at a time.
//
// The tiny wait afterwards matters. The select lines change instantly, but
// the switch inside the multiplexer, and the voltage on the shared signal
// wire, take a moment to settle. Read too soon and the sensor you looked at
// a moment ago bleeds into this one. Five millionths of a second is enough,
// and it stalls nothing.
int ARC16::_readChannel(int i) {
  digitalWrite(_select[0], bitRead(i, 0));
  digitalWrite(_select[1], bitRead(i, 1));
  digitalWrite(_select[2], bitRead(i, 2));
  digitalWrite(_select[3], bitRead(i, 3));
  delayMicroseconds(5);
  return analogRead(_signal);
}

// ---------------------------------------------------------------------------
// Raw read
// ---------------------------------------------------------------------------

void ARC16::read(int values[16]) {
  for (int i = 0; i < 16; i++) {
    values[i] = _readChannel(i);
    _raw[i]   = values[i];
  }
  _hasRead = true;
}

int ARC16::read(int i) {
  if (_badIndex(i)) return -1;
  return _readChannel(i);
}

// ---------------------------------------------------------------------------
// Calibration
// ---------------------------------------------------------------------------

void ARC16::calibrate() {
  if (!_checkRead()) return;

  for (int i = 0; i < 16; i++) {
    if (_raw[i] < _calMin[i]) _calMin[i] = _raw[i];
    if (_raw[i] > _calMax[i]) _calMax[i] = _raw[i];
  }
  _calibrated = true;
}

void ARC16::readCalibrated(int values[16]) {
  if (!_checkRead()) {
    for (int i = 0; i < 16; i++) values[i] = 0;
    return;
  }

  for (int i = 0; i < 16; i++) {
    values[i] = _scale(_raw[i], i);
  }
}

int ARC16::readCalibrated(int i) {
  if (_badIndex(i)) return -1;
  return _scale(_readChannel(i), i);
}

int ARC16::getMin(int i) {
  if (_badIndex(i)) return -1;
  return _calMin[i];
}

int ARC16::getMax(int i) {
  if (_badIndex(i)) return -1;
  return _calMax[i];
}

// Stretches one sensor's raw reading onto a shared 0 to 1000 scale.
//
// Why bother? No two sensors are the same. One might read 120 on the white
// floor and 800 on the black tape, while the sensor right next to it reads
// 200 and 950. If we compared those numbers directly, the brighter sensor
// would always look like it was closer to the line, and the robot would
// steer towards it forever. Giving every sensor its own stretch, from the
// white it actually saw up to the black it actually saw, means all 16 end
// up speaking the same language and can be compared fairly.
int ARC16::_scale(int raw, int i) {
  if (_calMax[i] - _calMin[i] < 10) {
    // The gap between white and black is far too small to stretch. Dividing
    // by a tiny gap would blow small wobbles up into huge numbers, so we hand
    // the raw reading straight back instead of a broken scaled one.
    if (!_calibrated) {
      if (!_warnedNotCalibrated) {
        Serial.println(F("ARC16: not calibrated yet"));
        _warnedNotCalibrated = true;
      }
    } else {
      if (!_warnedBadCalibration) {
        Serial.println(F("ARC16: sensor not calibrated properly"));
        _warnedBadCalibration = true;
      }
    }
    return constrain(raw, 0, 1000);
  }

  // The (long) is not optional. raw - _calMin[i] can be about 1000, and
  // multiplying that by 1000 gives roughly a million, which does not fit in
  // the 16-bit int an Uno or Nano uses. Without the (long) the number wraps
  // around into nonsense and the robot steers the wrong way.
  long value = (long)(raw - _calMin[i]) * 1000 / (_calMax[i] - _calMin[i]);
  return constrain((int)value, 0, 1000);
}

// ---------------------------------------------------------------------------
// Digital / threshold
// ---------------------------------------------------------------------------

void ARC16::setThreshold(int value) {
  if (value < 0 || value > 1000) {
    if (!_warnedThreshold) {
      Serial.println(F("ARC16: threshold out of range"));
      _warnedThreshold = true;
    }
  }
  _threshold = constrain(value, 0, 1000);
}

void ARC16::readDigital(bool values[16]) {
  if (!_checkRead()) {
    for (int i = 0; i < 16; i++) values[i] = false;
    return;
  }

  for (int i = 0; i < 16; i++) {
    values[i] = _isOnLine(_scale(_raw[i], i));
  }
}

bool ARC16::onLine(int i) {
  if (_badIndex(i)) return false;
  return _isOnLine(_scale(_readChannel(i), i));
}

int ARC16::countOnLine() {
  if (!_checkRead()) return 0;

  int count = 0;
  for (int i = 0; i < 16; i++) {
    if (_isOnLine(_scale(_raw[i], i))) count++;
  }
  return count;
}

bool ARC16::_isOnLine(int calValue) {
  return _lineIsBlack ? (calValue > _threshold) : (calValue < _threshold);
}

// ---------------------------------------------------------------------------
// Line position
// ---------------------------------------------------------------------------

void ARC16::setLine(int mode) {
  if (mode != BLACK && mode != WHITE) {
    if (!_warnedLineMode) {
      Serial.println(F("ARC16: invalid line mode"));
      _warnedLineMode = true;
    }
    return;  // keep whatever was set before
  }
  _lineIsBlack = (mode == BLACK);
}

// Keep your weights inside about -32 to +32. The line position is worked out
// as weight x reading, readings go up to 1000, and the answer is stored in an
// int - so a weight of 33 or more can tip it over and flip the sign.
void ARC16::setWeights(int w[16]) {
  int smallest = w[0];
  int biggest  = w[0];

  for (int i = 0; i < 16; i++) {
    _weights[i] = w[i];
    if (w[i] < smallest) smallest = w[i];
    if (w[i] > biggest)  biggest  = w[i];
  }

  // Halfway between the smallest and biggest weight is what "dead centre"
  // looks like, so that is the number getError() measures against.
  _setpoint = (smallest + biggest) / 2;
}

// Works out where the line is by taking a weighted average of the sensors
// that can see it.
//
// Why weighted? Each sensor gets a number saying where it sits on the bar:
// the left-hand ones are negative, the right-hand ones are positive. A
// sensor that is only half over the tape gives a smaller reading than one
// sitting right on it, so multiplying each sensor's position by how strongly
// it sees black lets a sensor that is barely touching the line pull the
// answer only a little. That is what makes the position slide smoothly as
// the robot drifts, instead of jumping from one sensor to the next.
void ARC16::_updateLine() {
  if (countOnLine() == 0) return;  // nothing to see, keep the last position

  // sum is a long because the running total of weight x reading can easily
  // pass what an int holds. The finished answer still goes into
  // _lastLineValue, which is an int, so keep custom weights inside about
  // -32 to +32: 32 x 1000 is 32000, which just fits.
  long sum   = 0;
  int  count = 0;

  for (int i = 0; i < 16; i++) {
    int cal = _scale(_raw[i], i);
    if (_isOnLine(cal)) {
      sum += (long)_weights[i] * cal;
      count++;
    }
  }
  _lastLineValue = sum / count;
}

int ARC16::readLine() {
  if (!_checkRead()) return 0;
  _updateLine();
  return _lastLineValue;
}

int ARC16::getError() {
  if (!_checkRead()) return 0;
  _updateLine();
  return _lastLineValue - _setpoint;
}

bool ARC16::lineFound() {
  if (!_checkRead()) return false;
  return countOnLine() > 0;
}

// ---------------------------------------------------------------------------
// Debug
// ---------------------------------------------------------------------------

void ARC16::printRaw() {
  if (!_checkRead()) return;

  for (int i = 0; i < 16; i++) {
    Serial.print(_raw[i]);
    if (i < 15) Serial.print(' ');
  }
  Serial.println();
}

void ARC16::printCalibrated() {
  if (!_checkRead()) return;

  for (int i = 0; i < 16; i++) {
    Serial.print(_scale(_raw[i], i));
    if (i < 15) Serial.print(' ');
  }
  Serial.println();
}

void ARC16::printLine() {
  if (!_checkRead()) return;

  Serial.print(F("line: "));
  Serial.print(_lastLineValue);
  Serial.print(F(" error: "));
  Serial.println(_lastLineValue - _setpoint);
}

// ---------------------------------------------------------------------------
// Private checks
// ---------------------------------------------------------------------------

bool ARC16::_checkRead() {
  if (_hasRead) return true;

  if (!_warnedNoRead) {
    Serial.println(F("ARC16: call read() first"));
    _warnedNoRead = true;
  }
  return false;
}

bool ARC16::_badIndex(int i) {
  if (i >= 0 && i <= 15) return false;

  if (!_warnedBadIndex) {
    Serial.println(F("ARC16: bad sensor index"));
    _warnedBadIndex = true;
  }
  return true;
}
