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
