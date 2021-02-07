/*
  MidiVolts.h - Library for converting Midi to CV.
*/

#include "Arduino.h"
#include <Wire.h>
#include "MidiVolts.h"

#define dacAddress 0x60

MidiVolts::MidiVolts(uint8_t gatePin, uint8_t channel)
{
  pinMode(gatePin, OUTPUT);
  _gatePin = gatePin;
  _channel = channel;

  //auto calculating frequency original
  double a = pow(2, (double)1 / (double)12);
  _fo = (double)8.175798916 * pow(a, (double)_lowestMIDINote);
}

void MidiVolts::NoteOn(byte midiNum)
{
  MidiNum = midiNum;

  if (midiNum >= _lowestMIDINote) // Checks if midi note is in range
  {
    double freq = GetFrequency(midiNum);
    int16_t resolution = GetResolution(freq) + _bendedResolution; // gets resolution plus any pitch bend

    if (resolution > 4095) // verifies resolution is within min max constraints 0 - 4095
    {
      resolution = 4095;
    }
    else if (resolution < 0)
    {
      resolution = 0;
    }

    SendToDAC(resolution, _channel);

    if (midiNum >= _lowestMIDINote && midiNum <= (_lowestMIDINote + 60))
    {
      digitalWrite(_gatePin, HIGH);
      noteState = true;
    }
  }
  else
  {
    digitalWrite(_gatePin, LOW);
    noteState = false;
  }
}

void MidiVolts::NoteOff()
{
  digitalWrite(_gatePin, LOW);
  noteState = false;
}

void MidiVolts::VelocityOn(byte amount)
{
  if (VelocityPin != UNSET)
  {
    uint16_t resolution = (double)amount * (double)4095 / (double)127;
    SendToDAC(resolution, VelocityPin);
  }
}

void MidiVolts::Bend(uint16_t amount, uint8_t semitonesUp, uint8_t semitonesDown)
{
  // 8192 is center, factor by 8191 for non neg numbers

  if (amount > 8192)
  {
    _bendedResolution = round((((double)amount - (double)8192) / (double)8191) * ((double)semitonesUp * (double)68.25));
  }
  else if (amount < 8192)
  {
    _bendedResolution = round((((double)amount - (double)8192) / (double)8192) * ((double)semitonesDown * (double)68.25));
  }
  else
  {
    _bendedResolution = 0;
  }

  double freq = GetFrequency(MidiNum);
  int16_t resolution = GetResolution(freq) + _bendedResolution;

  if (resolution > 4095) // verifies resolution is within min max constraints 0 - 4095
  {
    resolution = 4095;
  }
  else if (resolution < 0)
  {
    resolution = 0;
  }

  SendToDAC(resolution, _channel);
}

void MidiVolts::CC(byte amount)
{
  uint16_t resolution = (double)amount * (double)4095 / (double)127;
  SendToDAC(resolution, _channel);
}

double MidiVolts::GetFrequency(int midiNum)
{
  int n = midiNum - _lowestMIDINote;
  double a = pow(2, (double)1 / (double)12);
  double freq = _fo * pow(a, n);

  return freq;
}

int16_t MidiVolts::GetResolution(double frequency)
{
  PitchVoltage = (log( frequency / _fo)) / log(2);

  double volts = Offset + PitchVoltage * Gain; // compensates for gain and offset

  int16_t resolution = (int)round((volts / RefVoltage) * (double)4095); // gets resolution

  return resolution;
}

void MidiVolts::SendToDAC(uint16_t resolution, uint8_t channel)
{
  Wire.beginTransmission(dacAddress);     // Device Address: 1 1 0 0 A2 A1 A0 - 1100000

  switch (channel) {                      // Multi Write Command w/o EEPROM: C2 C1 C0 W1 W0 DAC1 DAC0 UDAC
    case 0:
      Wire.write(0x40); // Channel A - 01000000
      break;
    case 1:
      Wire.write(0x42); // Channel B - 01000010
      break;
    case 2:
      Wire.write(0x44); // Channel C - 01000100
      break;
    case 3:
      Wire.write(0x46); // Channel D - 01000110
      break;
  }

  //Example: 110101100110
  Wire.write(0x00 | (resolution / 256));  // 3rd Byte: VREF PD1 PD0 Gx {D11 D10 D9 D8} - 0000{1101}
  Wire.write(resolution % 256);           // 4th Byte: {D7 D6 D5 D4 D3 D2 D1 D0} - {01100110}
  Wire.endTransmission();
}
