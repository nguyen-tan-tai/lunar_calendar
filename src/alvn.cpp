#include "alvn.h"

GregorianDate::GregorianDate(int _y, byte _m, byte _d) {
  y = _y;
  m = _m;
  d = _d;
}

String GregorianDate::formatYyyyMmDd() {
  return String(y) + (m < 10 ? "0" : "") + String(m) + (d < 10 ? "0" : "") + String(d);
}

byte GregorianDate::getDaysOfMonth(int y, byte m) {
  if(m == 2) {
   	bool isLeapYear = (y % 4 == 0) && ((y % 400 == 0) || (y % 100 != 0));
    return isLeapYear ? 29 : 28;
  } else if(m == 4 || m == 6 || m == 9 || m == 11) {
    return 30;
  } else {
    return 31;
  }
}

LunarDate GregorianDate::toLunarDate1(YearlyMetaData data) {
  int daysBeforeLunarNewYear = 0;
  if(data.lunarNewYearGregorianMonth == 1) {
    if(m == 1) {
      daysBeforeLunarNewYear = data.lunarNewYearGregorianDay - d;
       //Serial.println("1111 daysBeforeLunarNewYear = " + String(daysBeforeLunarNewYear));
    } else {} // Never happen: it is handler of toLunarDate2
  } else { // data.lunarNewYearGregorianMonth == 2
    if(m == 1) {
      daysBeforeLunarNewYear = data.lunarNewYearGregorianDay + (31 - d); // 31 - d is days of Jan
       //Serial.println("2211 daysBeforeLunarNewYear = " + String(daysBeforeLunarNewYear));
    } else { // m == 2
      daysBeforeLunarNewYear = data.lunarNewYearGregorianDay - d;
       //Serial.println("2222 daysBeforeLunarNewYear = " + String(daysBeforeLunarNewYear));
    }
  }
  daysBeforeLunarNewYear -= 1;
  //Serial.println("data.lunarLeapMonth = " + String(data.lunarLeapMonth));
  //Serial.println("daysBeforeLunarNewYear = " + String(daysBeforeLunarNewYear));
  YearlyMetaData previousYearData = YearlyMetaData(data.y - 1);
  int h = 0;
  for(byte lMon = 12; lMon >= 10; lMon--) {
    //Serial.println("lMon = " + String(lMon));
    if(lMon == previousYearData.lunarLeapMonth){
      if(h + previousYearData.lunarDaysOfMonth[0] <= daysBeforeLunarNewYear) {
        h += previousYearData.lunarDaysOfMonth[0];
        //Serial.println("BBBB h = " + String(h));
      } else {
        //Serial.println("DDDD h = " + String(h));
        return LunarDate(y - 1, lMon, previousYearData.lunarDaysOfMonth[0] - daysBeforeLunarNewYear + h, true);
      }
    }
    //Serial.println("previousYearData.lunarDaysOfMonth[lMon] = " + String(previousYearData.lunarDaysOfMonth[lMon]));
    if(h + previousYearData.lunarDaysOfMonth[lMon] <= daysBeforeLunarNewYear) {
      h += previousYearData.lunarDaysOfMonth[lMon];
      //Serial.println("AAAA h = " + String(h));
    } else {
      //Serial.println("CCCC h = " + String(h));
      return LunarDate(y - 1, lMon, previousYearData.lunarDaysOfMonth[lMon] - daysBeforeLunarNewYear + h, false);
    }
  }
  return LunarDate(y, 1, 1, false); // Dummy fallback only
}

LunarDate GregorianDate::toLunarDate2(YearlyMetaData data) {
  int daysFromLunarNewYear = 0;
  for(byte gMon = data.lunarNewYearGregorianMonth; gMon < m; gMon++) {
    daysFromLunarNewYear += getDaysOfMonth(y, gMon);
  }
  daysFromLunarNewYear = daysFromLunarNewYear + d - data.lunarNewYearGregorianDay + 1;
  // Serial.println("data.lunarLeapMonth = " + String(data.lunarLeapMonth));
  // Serial.println("daysFromLunarNewYear = " + String(daysFromLunarNewYear));
  int h = 0;
  for(byte lMon = 1; lMon <= 12; lMon++) {
    //Serial.println("lMon = " + String(lMon));
    if(h + data.lunarDaysOfMonth[lMon] < daysFromLunarNewYear) {
      h += data.lunarDaysOfMonth[lMon];
      // Serial.println("AAAA h = " + String(h));
    } else {
      // Serial.println("CCCC h = " + String(h));
      return LunarDate(y, lMon, daysFromLunarNewYear - h, false);
    }
    if(lMon == data.lunarLeapMonth){
      if(h + data.lunarDaysOfMonth[0] < daysFromLunarNewYear) {
        h += data.lunarDaysOfMonth[0];
        // Serial.println("BBBB h = " + String(h));
      } else {
        // Serial.println("DDDD h = " + String(h));
        return LunarDate(y, lMon, daysFromLunarNewYear - h, true);
      }
    }
  }
  return LunarDate(y, 1, 1, false); // Dummy fallback only
}

LunarDate GregorianDate::toLunarDate() {
  YearlyMetaData data = YearlyMetaData(y);
  if(m < data.lunarNewYearGregorianMonth 
      || (m == data.lunarNewYearGregorianMonth && d < data.lunarNewYearGregorianDay)) {
    return toLunarDate1(data);
  }
  return toLunarDate2(data);
}

LunarDate::LunarDate(int _y, byte _m, byte _d, bool _isLeapMon) {
  y = _y;
  m = _m;
  d = _d;
  isLeapMon = _isLeapMon;
}

String LunarDate::formatYyyyMmDd() {
  return String(y) + (m < 10 ? "0" : "") + String(m) + (d < 10 ? "0" : "") + String(d);
}


/*
Luu am lich 3byte/nam
byte1:    [Ngay bat dau 1/1 AL - 5 bit][thang bat dau 1/1 AL - 1 bit][So ngay thang nhuan - 1 bit]
byte2:    [Thang nhuan - 4 bit][so ngay thang 12,11,10,9 - 4bit]
byte3:    [So ngay thang 8,7,6,5,4,3,2,1 - 8bit]
*/
YearlyMetaData::YearlyMetaData(int _y) {
  y = _y;
  byte data1 = YEARLY_META_DATA[y - ROOTYEAR][0];
  byte data2 = YEARLY_META_DATA[y - ROOTYEAR][1];
  byte data3 = YEARLY_META_DATA[y - ROOTYEAR][2];
  lunarNewYearGregorianDay = data1 >> 3;
  lunarNewYearGregorianMonth = data1 & B00000100 ? 2 : 1;
  lunarLeapMonth = data2 >> 4;

  lunarDaysOfMonth[0]  = data1 & B00000010 ? 30 : 29; // leap month days of month
  lunarDaysOfMonth[12] = data2 & B00001000 ? 30 : 29;
  lunarDaysOfMonth[11] = data2 & B00000100 ? 30 : 29;
  lunarDaysOfMonth[10] = data2 & B00000010 ? 30 : 29;
  lunarDaysOfMonth[9]  = data2 & B00000001 ? 30 : 29;
  lunarDaysOfMonth[8]  = data3 & B10000000 ? 30 : 29;
  lunarDaysOfMonth[7]  = data3 & B01000000 ? 30 : 29;
  lunarDaysOfMonth[6]  = data3 & B00100000 ? 30 : 29;
  lunarDaysOfMonth[5]  = data3 & B00010000 ? 30 : 29;
  lunarDaysOfMonth[4]  = data3 & B00001000 ? 30 : 29;
  lunarDaysOfMonth[3]  = data3 & B00000100 ? 30 : 29;
  lunarDaysOfMonth[2]  = data3 & B00000010 ? 30 : 29;
  lunarDaysOfMonth[1]  = data3 & B00000001 ? 30 : 29;
}
