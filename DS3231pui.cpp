#include <Arduino.h>
#include <DS3231pui.h>
#include <Wire.h>

DS3231pui::DS3231pui(uint8_t adr) {
  _adr = adr;
  //
  // uint8_t opt[2] = {B00000111, B00001000};
  // set3231(opt, 0x0E, 2);
}

void DS3231pui::set3231(uint8_t *rArr, uint8_t reg, uint8_t num) {
  Wire.beginTransmission(_adr);
  Wire.write(reg);
  for (uint8_t i = 0; i < num; i++) {
    Wire.write(*rArr);
    rArr++;
  }
  Wire.endTransmission();
}

void DS3231pui::set3231byte(uint8_t byt, uint8_t reg) {
  uint8_t dat[1];
  dat[0] = byt;
  set3231(dat, reg, 1);
}

void DS3231pui::get3231(uint8_t *tArr, uint8_t reg, uint8_t num,
                        uint8_t nTime) {
  Wire.beginTransmission(_adr);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(_adr, num);

  for (uint8_t i = 0; i < num; i++) {
    uint8_t tmpt = Wire.read();
    if (nTime == 1) {
      *tArr = ((tmpt & 0x0F) + ((tmpt >> 4) * 10));
    } else {
      *tArr = tmpt;
    }
    tArr++;
  }
}

uint8_t DS3231pui::get3231byte(uint8_t reg) {
  uint8_t datt[1];
  get3231(datt, reg, 1, 0);
  return datt[0];
}

void DS3231pui::get3231Time(uint8_t *tArr) { get3231(tArr, 0x00, 3, 1); }

void DS3231pui::get3231Date(uint8_t *tArr) { get3231(tArr, 0x03, 4, 1); }

void DS3231pui::get3231Alarm(uint8_t *tArr, uint8_t alm) {
  uint8_t fg = 0x08;
  if (alm == 1) {
    fg = 0x0B;
  }
  get3231(tArr, fg, 2, 1);
}

float DS3231pui::get3231Temp() {
  uint8_t mlSB[2];
  get3231(mlSB, 0x11, 2, 0);
  return ((mlSB[0] & B01111111) + ((mlSB[1] >> 6) * 0.25));
}

uint8_t DS3231pui::resetAlrm() {
  uint8_t datt = get3231byte(0x0F);
  set3231byte(B00, 0x0F);

  if (datt & 0x01) {
    return 1;
  } else if ((datt >> 1) & 0x01) {
    return 0;
  }
}

uint8_t DS3231pui::reqwYNtmr() {
  while (true) {
    uint8_t tmr = 9;
    while (!Serial.available()) {
      delay(1000);
      tmr--;
      Serial.write('\b');
      Serial.print(tmr);
      if (tmr == 0) {
        Serial.write('\n');
        return false;
      }
    }
    uint8_t dt = Serial.read();
    switch (dt) {
    case 'y':
      Serial.print("\by\n");
      return true;

    case 'n':
      Serial.print("\bn\n");
      return false;

    default:
      continue;
    }
  }
}

void DS3231pui::reqwTime(uint8_t *timeVal) {
  uint8_t tVal[4];
  for (int8_t i = 0; i < 4; i++) {
    while (!Serial.available()) {
      delay(50);
    }
    uint8_t dat_reciv = Serial.read();
    tVal[i] = dat_reciv;
    Serial.write('\b');
    Serial.write(dat_reciv);
    if (i == 1) {
      if (((tVal[0] - 48) * 10) + (tVal[1] - 48) < 24) {
        timeVal++;
        *timeVal = ((tVal[0] - 48) << 4) | (tVal[1] - 48);
        timeVal--;
        Serial.print(":");
      } else {
        Serial.print("\b\b");
        i = -1;
      }
    }
    if (i == 3) {
      if (((tVal[2] - 48) * 10) + (tVal[3] - 48) < 60) {
        *timeVal = ((tVal[2] - 48) << 4) | (tVal[3] - 48);
      } else {
        Serial.print("\b\b_");
        i = 1;
      }
    } else {
      Serial.print("_");
    }
  }
}

void DS3231pui::rsetTime() {
  uint8_t tme[2];
  Serial.print("Input 24h time _");
  reqwTime(tme);
  set3231(tme, 0x01, 2);
}

void DS3231pui::rsetAlarm() {
  const char *stts[2] = {"ON", "OFF"};
  uint8_t reg = 0x08;

  for (uint8_t a = 0; a < 2; a++) {
    uint8_t alrm[3];
    Serial.println();
    Serial.print("Alarm ");
    Serial.print(stts[a]);
    Serial.print(" set:'\n'_");

    reqwTime(alrm);
    alrm[2] = B10000000;
    set3231(alrm, reg, 3);
    reg += 3;
    Serial.print(" ok\n");
  }
}

void DS3231pui::reqwSetAC() {
  Serial.print("[c]_for set time\t[a]_for set alarm\t[x]_cancel\n");
  while (true) {
    while (!Serial.available()) {
      delay(50);
    }
    uint8_t serl = Serial.read();
    if (serl == 'c') {
      Serial.print("Set time");
      rsetTime();
      break;
    } else if (serl == 'a') {
      Serial.print("Set alarm");
      rsetAlarm();
      break;
    } else if (serl == 'x') {
      Serial.print("canel\n");
      break;
    } else {
      continue;
    }
  }
}

void DS3231pui::printRTCdata() {
  uint8_t timearr[3];
  uint8_t datearr[4];
  uint8_t alarr[2];
  get3231Time(timearr);
  get3231Date(datearr);

  Serial.print("\n");
  Serial.print(timearr[2]);
  Serial.print(":");
  Serial.print(timearr[1]);
  Serial.print(":");
  Serial.println(timearr[0]);
  Serial.print(datearr[0]);
  Serial.print("/ ");
  Serial.print(datearr[1]);
  Serial.print("/");
  Serial.print(datearr[2]);
  Serial.print("/");
  Serial.println(datearr[3]);

  Serial.print("\non\toff\n");
  get3231Alarm(alarr, 0);
  Serial.print(alarr[1]);
  Serial.print(":");
  Serial.print(alarr[0]);
  Serial.print("\t");
  get3231Alarm(alarr, 1);
  Serial.print(alarr[1]);
  Serial.print(":");
  Serial.println(alarr[0]);
  Serial.print("\nTemp: ");
  Serial.println(get3231Temp());
}
