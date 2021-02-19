/*
  This is a library for the octo MAX31855 thermocouple breakout board.
 
  
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

#ifndef MAX31855_H
#define MAX31855_H
class MAX31855
{
  int m_SCK;
  int m_SO;
  int m_CS;
  int m_T0;
  int m_T1;
  int m_T2;
  unsigned long m_latestData;

public:
  enum Fault : int;
  MAX31855(int, int, int, int, int, int);
  void readData(int);
  float getThermocoupleTemp();
  float getReferenceTemp();
  int getFaults();
  unsigned long getLatestData();
};
#endif
