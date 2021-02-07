/*
  MidiVoltsCalibration - Code for calibrating DAC offset and gain error.

  Connect Pitch Pins

  Pitch V0 -> Calibrate V0
  Pitch V1 -> Calibrate V1
  Pitch V2 -> Calibrate V2
  Pitch V3 -> Calibrate V3
*/

#include <Wire.h>

#define dacAddress 0x60

void setup() {

  Serial.begin(9600);
  Wire.begin();

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);

  double volts = 0;

  for (int n = 0; n <= 60; n++) {

    double v0 = readVoice(0, A3, volts, 0, 1);
    double v1 = readVoice(1, A2, volts, 0, 1);
    double v2 = readVoice(2, A1, volts, 0, 1);
    double v3 = readVoice(3, A0, volts, 0, 1);

    Serial.print(v0, DEC);
    Serial.print("\t");
    Serial.print(v1, DEC);
    Serial.print("\t");
    Serial.print(v2, DEC);
    Serial.print("\t");
    Serial.println(v3, DEC);

    volts = volts + (double)1 / (double)12;
  }
}

double readVoice(int channel, int readPin, double voltage, double offset, double gain)
{
  double compensated = offset + voltage * gain; // compensates for gain and offset
  int resolution = (int)round(((compensated / 5) * (double)4095));

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

  delay(200);
  double sample = (double)analogRead(readPin) / (double)1023 * (double)5;

  return sample;
}

void loop() {
  delay(1000);
}
