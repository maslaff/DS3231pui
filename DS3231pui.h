#ifndef DS3231pui_h
#define DS3231pui_h

#include <Arduino.h>
#include <Wire.h>

class DS3231pui {
public:
  DS3231pui(uint8_t adr);
  uint8_t reqwYNtmr();
  void rsetTime();
  void rsetAlarm();
  void reqwSetAC();
  void printRTCdata();
  void get3231Time(uint8_t *tArr);
  void get3231Date(uint8_t *tArr);
  void get3231Alarm(uint8_t *tArr, uint8_t alm);
  float get3231Temp();
  uint8_t resetAlrm();

private:
  uint8_t _adr;
  void reqwTime(uint8_t *timeVal);
  void set3231(uint8_t *rArr, uint8_t reg, uint8_t num);
  void set3231byte(uint8_t byt, uint8_t reg);
  void get3231(uint8_t *tArr, uint8_t reg, uint8_t num, uint8_t nTime = 1);
  uint8_t get3231byte(uint8_t reg);
};

#endif
