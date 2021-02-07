/*
  MidiVolts - Sample code for public use by Space Brain Circuits.
*/

// ************************************* USER DEFINE PARAMETERS *************************************

// PLEASE CHOOSE MODE
// 1 VOICE MONOPHONIC ON V0 WITH VELOCITY ON V1, AFTERTOUCH ON V2, AND CC1 (MOD WHEEL) ON V3: ......1
// 2 VOICE DUOPHONIC ON V0, V1 WITH VELOCITY ON V2 AND V3 RESPECTIVELY: ............................2
// 3 VOICE POLYPHONIC ON V0, V1, V2 WITH CC1 (MOD WHEEL) ON V3: ....................................3
// 4 VOICE POLYPHONIC ON V0, V1, V2, V3: ...........................................................4
// 4 VOICE UNISON ON V0, V1, V2, V3: ...............................................................5
// 1 VOICE MONOPHONIC ON V0 WITH CC2 ON V1, CC3 ON V2, CC4 ON V3: ..................................6
// CC CONTROLLER WITH CC1 ON V0, CC2 ON V1, CC3 ON V2, CC4 ON V3: ..................................7
#define MODE 4

// CHOOSE RANGE OF SEMITONES FOR PITCH BEND WHEEL (DEFAULT UP: 2, DEFAULT DOWN: 12)
#define PITCH_BEND_SEMITONES_UP 2
#define PITCH_BEND_SEMITONES_DOWN 2

// CHOOSE CONTROL FUNCTION DURING CC STATUS BYTE 176
#define CC1 1  // Mod Wheel
#define CC2 49
#define CC3 50
#define CC4 53

// CHOOSE PIN TO OUTPUT CLOCK PULSE (DEFAULT: 8)
#define CLOCK_PIN 8

// PIN NUMBER ASSIGNMENTS
#define V0 0
#define V1 1
#define V2 2
#define V3 3

// **************************************************************************************************

#include "MidiVolts.h"
#include "Midi.h"

Midi midi; //Initialize class for listening to Midi Messages
uint8_t UnusedNotes[8]; //Used to track held notes
uint8_t voices; //Number of voices set per MODE selection

MidiVolts voice[4] = {MidiVolts(10, V0), MidiVolts(11, V1), MidiVolts(12, V2), MidiVolts(13, V3)}; // MidiVolts(GatePin, Channel)

void setup() {

  // for fine tune calibration, please see README or Calibration Guide
  // sample code for calibration
  //    voice[0].Gain = 1;
  //    voice[0].Offset = 0;
  //    voice[1].Gain = 1.008;
  //    voice[1].Offset = 0;
  //    voice[2].Gain = 1;
  //    voice[2].Offset = 0;
  //    voice[3].Gain = 1.004;
  //    voice[3].Offset = 0.003;

  Serial.begin(31250); //Midi baud
  Wire.begin();

  pinMode(CLOCK_PIN, OUTPUT);

  memset(UnusedNotes, 128, sizeof(UnusedNotes));

  if (MODE == 1) {
    voice[V0].VelocityPin = V1; //pitch CV set on V0 will have corresponding Velocity on V1
  }
  else if (MODE == 2)
  {
    voice[V0].VelocityPin = V2; //pitch CV set on V0 will have corresponding Velocity on V2
    voice[V1].VelocityPin = V3; //pitch CV set on V1 will have corresponding Velocity on V3
  }

  if (MODE == 5 || MODE == 6)
  {
    voices = 1;
  }
  else if (MODE == 7)
  {
    voices = 0;
  }
  else
  {
    voices = MODE;
  }
}

void loop() {
  midi.Listen(); // Listen for Midi Messages

  if (midi.On == true)
  {
    NoteOn(midi.MidiNum, midi.Velocity); // Set Voltage to Voice, Velocity and turn on Gate
  }

  if (midi.Off == true)
  {
    NoteOff(midi.MidiNum); // Turn off Gate
  }

  if (midi.BendOn == true) // Bend notes
  {
    if (MODE == 1 || MODE == 6)
    {
      voice[V0].Bend(midi.Bend, PITCH_BEND_SEMITONES_UP, PITCH_BEND_SEMITONES_DOWN);
    }
    else if (MODE == 2)
    {
      voice[V0].Bend(midi.Bend, PITCH_BEND_SEMITONES_UP, PITCH_BEND_SEMITONES_DOWN);
      voice[V1].Bend(midi.Bend, PITCH_BEND_SEMITONES_UP, PITCH_BEND_SEMITONES_DOWN);
    }
    else if (MODE == 3)
    {
      voice[V0].Bend(midi.Bend, PITCH_BEND_SEMITONES_UP, PITCH_BEND_SEMITONES_DOWN);
      voice[V1].Bend(midi.Bend, PITCH_BEND_SEMITONES_UP, PITCH_BEND_SEMITONES_DOWN);
      voice[V2].Bend(midi.Bend, PITCH_BEND_SEMITONES_UP, PITCH_BEND_SEMITONES_DOWN);
    }
    else if (MODE == 4 || MODE == 5)
    {
      voice[V0].Bend(midi.Bend, PITCH_BEND_SEMITONES_UP, PITCH_BEND_SEMITONES_DOWN);
      voice[V1].Bend(midi.Bend, PITCH_BEND_SEMITONES_UP, PITCH_BEND_SEMITONES_DOWN);
      voice[V2].Bend(midi.Bend, PITCH_BEND_SEMITONES_UP, PITCH_BEND_SEMITONES_DOWN);
      voice[V3].Bend(midi.Bend, PITCH_BEND_SEMITONES_UP, PITCH_BEND_SEMITONES_DOWN);
    }
  }

  if (midi.ControlOn == true) // CC
  {
    if (midi.ControlFunction == CC1)
    {
      if (MODE == 1 || MODE == 3)
      {
        voice[V3].CC(midi.Control);
      }
      else if (MODE == 7)
      {
        voice[V0].CC(midi.Control);
      }
    }
    if (midi.ControlFunction == CC2)
    {
      if (MODE == 6 || MODE == 7)
      {
        voice[V1].CC(midi.Control);
      }
    }
    if (midi.ControlFunction == CC3)
    {
      if (MODE == 6 || MODE == 7)
      {
        voice[V2].CC(midi.Control);
      }
    }
    if (midi.ControlFunction == CC4)
    {
      if (MODE == 6 || MODE == 7)
      {
        voice[V3].CC(midi.Control);
      }
    }
  }

  if (midi.AftertouchOn == true)
  {
    if (MODE == 1)
    {
      voice[V2].CC(midi.Aftertouch);
    }
  }

  if (midi.ClockOn == true) // Clock
  {
    digitalWrite(CLOCK_PIN, HIGH);
  }
  else if (midi.ClockOff == true)
  {
    digitalWrite(CLOCK_PIN, LOW);
  }
}

void NoteOn(byte midiNum, byte velocity) // Sets voltage to next available voice.
{
  for (int i = 0; i < voices; i++) {
    if (voice[i].noteState == false)
    {
      if (MODE == 5)
      {
        voice[V0].NoteOn(midiNum);
        voice[V1].NoteOn(midiNum);
        voice[V2].NoteOn(midiNum);
        voice[V3].NoteOn(midiNum);
      }
      else
      {
        voice[i].NoteOn(midiNum);
        voice[i].VelocityOn(velocity);
      }

      return;
    }
    else if (i == (voices - 1))
    {
      for (int u = 0; u < 8; u++)
      {
        if (UnusedNotes[u] == 128)
        {
          UnusedNotes[u] = voice[i].MidiNum;
          break;
        }
      }

      if (MODE == 5)
      {
        voice[V0].NoteOn(midiNum);
        voice[V1].NoteOn(midiNum);
        voice[V2].NoteOn(midiNum);
        voice[V3].NoteOn(midiNum);
      }
      else
      {
        voice[i].NoteOn(midiNum);
        voice[i].VelocityOn(velocity);
      }
    }
  }
}

void NoteOff(byte midiNum)
{
  for (int i = 0; i < voices; i++) {
    if (voice[i].noteState == true && voice[i].MidiNum == midiNum)
    {
      if (MODE == 5)
      {
        voice[V0].NoteOff();
        voice[V1].NoteOff();
        voice[V2].NoteOff();
        voice[V3].NoteOff();
      }
      else
      {
        voice[i].NoteOff();
      }

      for (int u = 7; u >= 0; u--) {
        {
          if (UnusedNotes[u] != 128)
          {
            if (MODE == 5)
            {
              voice[V0].NoteOn(UnusedNotes[u]);
              voice[V1].NoteOn(UnusedNotes[u]);
              voice[V2].NoteOn(UnusedNotes[u]);
              voice[V3].NoteOn(UnusedNotes[u]);
            }
            else
            {
              voice[i].NoteOn(UnusedNotes[u]);
            }

            UnusedNotes[u] = 128;
            break;
          }
        }
      }

      return;
    }
  }

  for (int i = 0; i < 8; i++)
  {
    if (UnusedNotes[i] == midiNum)
    {
      UnusedNotes[i] = 128;
      break;
    }
  }
}
