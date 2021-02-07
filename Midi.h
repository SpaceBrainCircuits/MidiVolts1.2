/*
  Midi.h - Library for Midi.
*/

#ifndef Midi_h
#define Midi_h

#include "Arduino.h"

class Midi
{
  public:
    void Listen();
    byte MidiNum;
    bool On;
    bool Off;
    byte Velocity;
    
    uint16_t Bend;
    bool BendOn;

    byte ControlFunction;
    byte Control;
    bool ControlOn;

    byte Aftertouch;
    bool AftertouchOn;

    bool ClockOn;
    bool ClockOff;

    bool ClockEnabled = false;

    bool AllNotesOff = true;

  private:

    int _clockCount = 0;
    byte _statusByte; // tracks statusByte, 128 or 144 (chan 1)
    int _noteTracking = 0;
};

#endif
