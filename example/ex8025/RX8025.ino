#include "RX8025.h"

RX8025 rtc;

void setup(){
    Serial.begin(9600);
    Serial.println("on the setup");
    Wire.begin(5,4);
    rtc.RX8025_init();
    rtc.setRtcTime(0, 24, 21,0,4, 4, 22);

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
    delay(1000);
    Serial.println(rtc.getUnixtime());
}









