/*
  This is a library for the octo MAX31855 thermocouple breakout board.

  Look for the boards on eBay or Amazon.

  https://www.ebay.com/usr/neemtech
  https://www.amazon.com/s?me=AH0OQAWQBELD0&marketplaceID=ATVPDKIKX0DER


  MIT License

  Copyright (c) 2020 Mitchell Herbert

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files
  (the "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "MAX31855.h"
#include "Arduino.h"

enum Fault : int
{
  NO_FAULT,
  SCV_FAULT,
  SCG_FAULT,
  OC_FAULT,
};

/**
   Initializes DIO pins and instance variables.
   CS initializes to high because it is active low.
   SO is an input pin.

   @param SO The pin number of the SO line
   @param CS The pin number of the CS line
   @param SCK The pin number of the SCK line
   @param T0 The pin number of the T0 line
   @param T1 The pin number of the T1 line
   @param T2 The pin number of the T2 line
*/
MAX31855::MAX31855(int SCK, int CS, int SO, int T0, int T1, int T2)
{
  // Setup all of the DIO pins
  int activeHighPins[] {SCK, T0, T1, T2};
  for (int pin : activeHighPins)
  {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);
  pinMode(SO, INPUT);
  // Initialize instance variables
  m_SCK = SCK;
  m_CS = CS;
  m_SO = SO;
  m_T0 = T0;
  m_T1 = T1;
  m_T2 = T2;
  // Initialize the poll data to zero
  m_latestData = 0x0;
}

/**
   Communicates with the octo MAX31855 board to retrieve
   temperature and fault data. The data is stored in
   m_latest_data for later reference.

   @param thermId id of the thermocouple (0 - 7)
*/
void MAX31855::readData(int thermId)
{
  // Select the thermocouple using multiplexer
  digitalWrite(m_T2, thermId & 0x4);
  digitalWrite(m_T1, thermId & 0x2);
  digitalWrite(m_T0, thermId & 0x1);
  // Wait for the multiplexer to update
  delay(125);
  // Select the chip and record incoming data
  unsigned long data = 0;
  digitalWrite(m_CS, LOW);
  // Shift in 32 bits of data
  for (int shiftCount = 31; shiftCount >= 0; shiftCount--)
  {
    digitalWrite(m_SCK, HIGH);
    data += (unsigned long) digitalRead(m_SO) << shiftCount;
    digitalWrite(m_SCK, LOW);
  }
  digitalWrite(m_CS, HIGH);
  m_latestData = data;
}

/**
   Gets the temperature of the most recently polled
   thermocouple. Should be called after calling
   readData(int) on the appropriate thermocouple.

   @return float representing the temperature in celsius
*/
float MAX31855::getThermocoupleTemp()
{
  unsigned long data = m_latestData;
  // Select appropriate bits
  data = data >> 18;
  // Handle twos complement
  if (data >= 0x2000)
    data = -((data ^ 0x3fff) + 1);
  // Divide by 4.0f to handle fractional component
  return data / 4.0f;
}

/**
   Gets the temperature of the reference junction
   from the most recent poll. Should be called
   after calling readData(int) on any thermocouple.

   @return float representing the temperature in celsius
*/
float MAX31855::getReferenceTemp()
{
  unsigned long data = m_latestData;
  // Select appropriate bits
  data = (data & 0xfff0) >> 4;
  // Handle twos complement
  if (data >= 0x800)
    data = -((data ^ 0xfff) + 1);
  // Divide by 16.0f to handle fractional component
  return data / 16.0f;
}

/**
   Returns a value signifiying a particular fault in the most
   recent poll. Should be called after calling readData(int)
   on the appropriate thermocouple.

   NO_FAULT (0) indicates that no fault exists
   SCV_FAULT (1) indicates the thermocouple is shorted to VCC
   SCG_FAULT (2) indicates the thermocouple is shorted to GND
   OC_FAULT (3) indicates the thermocouple is not connected

   @return an integer representing the fault
*/
int MAX31855::getFaults() {
  unsigned long data = m_latestData;
  if (data & 0x00010000) {
    if (data & 0b100) {
      return SCV_FAULT;
    } else if (data & 0b10) {
      return SCG_FAULT;
    } else if (data & 0b1) {
      return OC_FAULT;
    }
  }
  return NO_FAULT;
}

/**
   Returns the value of m_latestData

   @return the value of m_latestData
*/
unsigned long MAX31855::getLatestData() {
  return m_latestData;
}
