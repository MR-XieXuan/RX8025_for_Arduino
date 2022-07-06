#include "RX8025.h"

RX8025 rtc;

void setup(){
    Serial.begin(9600);
    Serial.println("on the setup");
    Wire.begin(5,4);
    rtc.RX8025_init();
    uint8_t week_my = (22 -2000) + (22 -2000)/4) + (13 * ( 4 +1) / 5) + 4 - 36;
    rtc.setRtcTime(0, 24, 21, week_my , 4, 4, 22);


}

void loop(){
    Serial.print(rtc.getYear());
    Serial.print("年");
    Serial.print(rtc.getMonth());
    Serial.print("月");
    Serial.print(rtc.getDate());
    Serial.print("日");
    Serial.print(rtc.getHour());
    Serial.print(":");
    Serial.print(rtc.getMinute());
    Serial.print(":");
    Serial.println(rtc.getSecond());
    Serial.printf("Week : %d " , rtc.getDoW()  );
    delay(1000);
    Serial.println(rtc.getUnixtime());
    rtc.Conversion(0,rtc.getYear(),rtc.getMonth(),rtc.getDate());
    Serial.printf("Moon: %02d%02d / %02d / %02d " 20, 22 , rtc.month_moon , rtc.day_moon);
}





















