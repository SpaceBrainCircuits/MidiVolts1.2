/*
  Midi.h - Library for Midi.
*/

#include "Arduino.h"
#include "Midi.h"

void Midi::Listen()
{
  On = false;
  Off = false;
  ClockOn = false;
  ClockOff = false;
  BendOn = false;
  ControlOn = false;
  AftertouchOn = false;

  if (Serial.available() > 0) // if serial exists
  {
    byte currentByte = Serial.read(); // gets currentByte

    if (currentByte > 127) // if currentByte is a status byte (> 127)
    {
      _statusByte = currentByte; // set statusByte
    }

    if (currentByte < 128 && _statusByte == 144) // note on and pitch byte
    {
      while (!Serial.available()) {} // waits until serial exists
      Velocity = Serial.read();

      if (Velocity != 0)
      {
        On = true;
        _noteTracking++;
      }
      else
      {
        Off = true;

        if (_noteTracking > 0)
        {
          _noteTracking--;
        }
      }

      MidiNum = currentByte;
    }
    else if (currentByte < 128 && _statusByte == 128) // note off and pitch byte
    {
      while (!Serial.available()) {} // waits until serial exists
      Serial.read(); //dispose velocity on Note off
      Off = true;
      MidiNum = currentByte;

      if (_noteTracking > 0)
      {
        _noteTracking--;
      }
    }

    if (_noteTracking == 0)
    {
      AllNotesOff = true;
    }
    else
    {
      AllNotesOff = false;
    }

    // Pitch bend

    if (currentByte < 128 && _statusByte == 224) // Pitch bend
    {
      BendOn = true;
      byte LSB = currentByte;
      while (!Serial.available()) {} // waits until serial exists
      byte MSB = Serial.read();
      Bend = LSB + (MSB << 7);
    }

    //Control Change

    if (currentByte < 128 && _statusByte == 176)
    {
      ControlOn = true;
      ControlFunction = currentByte;
      while (!Serial.available()) {} // waits until serial exists
      Control = Serial.read();
    }

    //Aftertouch

    if (currentByte < 128 && _statusByte == 208)
    {
      AftertouchOn = true;
      Aftertouch = currentByte;
    }

    // Midi Clock

    if (_statusByte == 250 || _statusByte == 251) // Midi Start or Continue
    {
      ClockOn = true;
      ClockEnabled = true;
      _clockCount = 0;
      _clockCount++;
    }
    else if (_statusByte == 252) // Midi Stop
    {
      ClockEnabled = false;
      ClockOff = true;
    }

    if (_statusByte == 248 && ClockEnabled == true) // Midi Clock
    {
      if (_clockCount == 0)
      {
        ClockOn = true;
      }
      else if (_clockCount == 11)
      {
        ClockOff = true;
      }
      if (_clockCount == 23)
      {
        _clockCount = 0;
      }
      else
      {
        _clockCount++;
      }
    }
  }
}
