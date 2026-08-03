/*
  ARC16 - 16-channel analog IR reflectance sensor array

  Every sensor gives a number from 0 to 1023.
  A BIG number means the sensor is looking at black.
  A SMALL number means the sensor is looking at white.

  All 16 sensors share ONE analog pin. A multiplexer sits in between, and
  four select lines tell it which sensor you want, as a binary number.

  The array always has exactly 16 sensors.
*/

#ifndef ARC16_H
#define ARC16_H

#include <Arduino.h>

// Use these with setLine() to say what colour your line is.
#define BLACK 1
#define WHITE 0

class ARC16 {
  public:
    // Gets the library ready with sensible starting settings.
    ARC16();

    // Tells the library which pins you used. Leave the last one out unless
    // you have wired AREF to 3.3V - see the README before you do.
    void begin(int select[4], int enable, int signal, bool useExternalRef = false);

    // Reads all 16 sensors at once and copies the numbers into your array.
    void read(int values[16]);

    // Reads just one sensor right now and gives you its number.
    int read(int i);

    // Remembers the smallest and biggest number each sensor has seen so far.
    void calibrate();

    // Copies all 16 readings into your array, stretched onto a 0 to 1000 scale.
    void readCalibrated(int values[16]);

    // Reads one sensor right now and stretches it onto the 0 to 1000 scale.
    int readCalibrated(int i);

    // Gives the smallest number sensor i saw while calibrating (its white).
    int getMin(int i);

    // Gives the biggest number sensor i saw while calibrating (its black).
    int getMax(int i);

    // Sets the dividing number between "on the line" and "off the line".
    void setThreshold(int value);

    // Fills your array with true or false: is each sensor on the line?
    void readDigital(bool values[16]);

    // Checks right now whether sensor i is sitting on the line.
    bool onLine(int i);

    // Counts how many of the 16 sensors are sitting on the line.
    int countOnLine();

    // Tells the library whether your line is BLACK or WHITE.
    void setLine(int mode);

    // Changes how much each sensor counts. Keep every weight from -32 to +32.
    void setWeights(int w[16]);

    // Works out where the line is and gives you that position.
    int readLine();

    // Gives how far off the middle the line is. 0 means perfectly centred.
    int getError();

    // Says whether any sensor can see the line at all.
    bool lineFound();

    // Sets the three numbers that decide how hard the robot steers.
    void setPID(float kp, float ki, float kd);

    // Works out how hard to steer to get back onto the line.
    int getCorrection();

    // Prints the 16 raw readings to the Serial Monitor.
    void printRaw();

    // Prints the 16 stretched 0 to 1000 readings to the Serial Monitor.
    void printCalibrated();

    // Prints the line position and the error to the Serial Monitor.
    void printLine();

  private:
    int   _select[4];        // the four lines that pick a sensor, S0 to S3
    int   _enable;           // turns the multiplexer on. LOW means on.
    int   _signal;           // the one analog pin every sensor comes through

    int   _raw[16];          // the numbers from the last read(values[16])
    bool  _hasRead;          // has read(values[16]) ever been called?

    int   _calMin[16];       // smallest number each sensor has seen
    int   _calMax[16];       // biggest number each sensor has seen
    bool  _calibrated;       // has calibrate() ever been called?

    int   _threshold;        // the on-the-line / off-the-line dividing number
    int   _weights[16];      // how much each sensor counts towards the position
    int   _setpoint;         // the position number that means "dead centre"
    bool  _lineIsBlack;      // true for a black line, false for a white line
    int   _lastLineValue;    // the last position we were sure about

    float _kp, _ki, _kd;     // the three PID steering numbers
    int   _lastError;        // the error from the previous getCorrection()
    long  _integral;         // every error so far, added up. A long, because
                             // an int fills up after only a few loops

    // Each warning prints only once, so one wiring mistake cannot flood
    // the Serial Monitor thousands of times a second.
    bool  _warnedPinTwice;
    bool  _warnedBadIndex;
    bool  _warnedThreshold;
    bool  _warnedNoRead;
    bool  _warnedNotCalibrated;
    bool  _warnedBadCalibration;
    bool  _warnedLineMode;

    // Picks one sensor on the multiplexer and reads it.
    int  _readChannel(int i);

    // Stretches one raw reading onto the shared 0 to 1000 scale.
    int  _scale(int raw, int i);

    // Answers "is this stretched number on the line?".
    bool _isOnLine(int calValue);

    // Works out the newest line position and stores it in _lastLineValue.
    void _updateLine();

    // True if read(values[16]) has run. Warns once if it has not.
    bool _checkRead();

    // True if i is not a sensor number from 0 to 15. Warns once if so.
    bool _badIndex(int i);
};

#endif
