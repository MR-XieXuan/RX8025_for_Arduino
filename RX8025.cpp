#include "RX8025.h"
// These included for the DateTime class inclusion; will try to find a way to
// not need them in the future...
#if defined(__AVR__)
#include <avr/pgmspace.h>
#elif defined(ESP8266)
#include <pgmspace.h>
#endif
// Changed the following to work on 1.0
//#include "WProgram.h"
#include <Arduino.h>

// 8025I2C地址
#define RX8025_address 0x32
// 日期起始时间(这里为啥要减掉八个小时,因为用的日期所在时区和国内时区相差8小时,所以需要减掉八小时的时区时间)
#define SECONDS_FROM_1970_TO_2000 946684800 - (8 * 60 * 60)
//
static const uint8_t daysInMonth[] PROGMEM = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/**
   自2000/01/01起的天数，2001年有效。。2099
*/
static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d)
{
    if (y >= 2000)
        y -= 2000;
    uint16_t days = d;
    for (uint8_t i = 1; i < m; ++i)
        days += pgm_read_byte(daysInMonth + i - 1);
    if (m > 2 && isleapYear(y))
        ++days;
    return days + 365 * y + (y + 3) / 4 - 1;
}

static long time2long(uint16_t days, uint8_t h, uint8_t m, uint8_t s)
{
    return ((days * 24L + h) * 60 + m) * 60 + s;
}

DateTime::DateTime(uint32_t t)
{
    t -= 946684800; // SECONDS_FROM_1970_TO_2000; // bring to 2000 timestamp from 1970

    ss = t % 60;
    t /= 60;
    mm = t % 60;
    t /= 60;
    hh = t % 24;
    uint16_t days = t / 24;
    uint8_t leap;
    for (yOff = 0;; ++yOff)
    {
        leap = isleapYear(yOff);
        if (days < 365 + leap)
            break;
        days -= 365 + leap;
    }
    for (m = 1;; ++m)
    {
        uint8_t daysPerMonth = pgm_read_byte(daysInMonth + m - 1);
        if (leap && m == 2)
            ++daysPerMonth;
        if (days < daysPerMonth)
            break;
        days -= daysPerMonth;
    }
    d = days + 1;
}

DateTime::DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec)
{
    if (year >= 2000)
    {
        year -= 2000;
    }
    yOff = year;
    m = month;
    d = day;
    hh = hour;
    mm = min;
    ss = sec;
}

// supported formats are date "Mmm dd yyyy" and time "hh:mm:ss" (same as __DATE__ and __TIME__)
DateTime::DateTime(const char *date, const char *time)
{
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    static char buff[4] = {'0', '0', '0', '0'};
    int y;
    sscanf(date, "%s %c %d", buff, &d, &y);
    yOff = y >= 2000 ? y - 2000 : y;
    m = (strstr(month_names, buff) - month_names) / 3 + 1;
    sscanf(time, "%c:%c:%c", &hh, &mm, &ss);
}

// UNIX time: IS CORRECT ONLY WHEN SET TO UTC!!!
uint32_t DateTime::unixtime(void) const
{
    uint32_t t;
    uint16_t days = date2days(yOff, m, d);
    t = time2long(days, hh, mm, ss);
    t += SECONDS_FROM_1970_TO_2000; // seconds from 1970 to 2000
    return t;
}

// Slightly modified from JeeLabs / Ladyada
// Get all date/time at once to avoid rollover (e.g., minute/second don't match)
static uint8_t bcd2bin(uint8_t val)
{
    return val - 6 * (val >> 4);
}
// Commented to avoid compiler warnings, but keeping in case we want this
// eventually
static uint8_t bin2bcd(uint8_t val) { return val + 6 * (val / 10); }

/**
   判断是否是闰年
*/
bool isleapYear(const uint8_t y)
{
    //检查是否可以被4整除
    if (y & 3)
    {
        return false;
    }
    // 仅在第一次失败时检查其他
    return (y % 100 || y % 400 == 0);
}

RX8025::RX8025() // costruttore
{
    RX8025_Control[0] = 0x20;
    RX8025_Control[1] = 0x00;
}

/**
 * 向时钟芯片设置时间
 * @param s  秒钟
 * @param m  分钟
 * @param h 时钟
 * @param w 星期
 * @param d  天
 * @param mh 月
 * @param y 年
 */
void RX8025::setRtcTime(uint8_t s, uint8_t m, uint8_t h, uint8_t w, uint8_t d, uint8_t mh, uint8_t y)
{
    // 使用指定的地址开始向I2C从设备进行传输。
    Wire.beginTransmission(RX8025_address);
    Wire.write((byte)0x00);
    Wire.write(decToBcd(s));
    Wire.write(decToBcd(m));
    Wire.write(decToBcd(h));
    Wire.write(WeekToBdc(w));
    Wire.write(decToBcd(d));
    Wire.write(decToBcd(mh));
    Wire.write(decToBcd(y));
    // 停止与从机的数据传输
    Wire.endTransmission();
}

/**
 * 获取寄存器里面的数据
 * @param regaddr
 * @return byte
 */
byte RX8025::getData(byte regaddr)
{
    // 使用指定的地址开始向I2C从设备进行传输。
    Wire.beginTransmission(RX8025_address);
    Wire.write(regaddr);
    // 停止与从机的数据传输
    Wire.endTransmission();
    // 由主设备用来向从设备请求字节。
    Wire.requestFrom(RX8025_address, 1);
    // 读取数据
    return Wire.read();
}

/**
 * @brief 初始化函数
 *
 */
void RX8025::RX8025_init(void)
{
    // Wire初始化
    Wire.begin();
    // 使用指定的地址开始向I2C从设备进行传输。
    Wire.beginTransmission(RX8025_address);
    Wire.write(0xe0);
    for (unsigned char i = 0; i < 2; i++)
    {
        Wire.write(RX8025_Control[i]);
    }
    // 使用指定的地址开始向I2C从设备进行传输。
    Wire.endTransmission();
}

/**
 * @brief 将十进制编码的二进制数转换为普通十进制数
 *
 * @param val
 * @return byte
 */
byte RX8025::decToBcd(byte val)
{
    // 将十进制编码的二进制数转换为普通十进制数
    return ((val / 10 * 16) + (val % 10));
}

byte RX8025::WeekToBdc(byte val)
{
    return 0x01 << val;
}

byte RX8025::WeekToNum(byte val)
{
    return log2(val);
}

/**
 * 将二进制编码的十进制数转换为普通十进制数
 *
 * @param val
 * @return byte
 */
byte RX8025::bcdToDec(byte val)
{
    // 将二进制编码的十进制数转换为普通十进制数
    return ((val / 16 * 10) + (val % 16));
}

/**
 * 获取秒钟
 */
byte RX8025::getSecond()
{
    byte buff = getData(RX8025_SEC);
    return bcdToDec(buff & 0x7f);
}

/**
 * 获取分钟数
 * @return byte
 */
byte RX8025::getMinute()
{
    byte buff = getData(RX8025_MIN);
    return bcdToDec(buff & 0x7f);
}

/**
 * 获取小时数
 * @return byte
 */
byte RX8025::getHour()
{
    byte buff = getData(RX8025_HR);
    return bcdToDec(buff & 0x3f);
}

/**
 * 获取星期数
 * @return byte
 */
byte RX8025::getDoW()
{
    byte buff = getData(RX8025_WEEK);
    return WeekToNum(buff & 0x7f);
}

/**
 * 获取日期
 * @return byte
 */
byte RX8025::getDate()
{
    byte buff = getData(RX8025_DATE);
    return bcdToDec(buff & 0x3f);
}

/**
 * 获取月份
 * @return byte
 */
byte RX8025::getMonth()
{
    byte buff = getData(RX8025_MTH);
    return bcdToDec(buff & 0x1f);
}

/**
 * 获取年份
 * @return byte
 */
byte RX8025::getYear()
{
    byte buff = getData(RX8025_YR);
    return bcdToDec(buff & 0xff);
}

long RX8025::getUnixtime()
{
    // 使用指定的地址开始向I2C从设备进行传输。
    Wire.beginTransmission(RX8025_address);
    Wire.write(0x00);
    // 停止与从机的数据传输
    Wire.endTransmission();
    // 由主设备用来向从设备请求字节。
    Wire.requestFrom(RX8025_address, 7);
    // 读取数据
    uint16_t ss = bcdToDec(Wire.read() & 0x7F);
    uint16_t mm = bcdToDec(Wire.read() & 0x7f);
    uint16_t hh = bcdToDec(Wire.read() & 0x3f);
    Wire.read();
    uint16_t d = bcdToDec(Wire.read() & 0x3f);
    uint16_t m = bcdToDec(Wire.read() & 0x1f);
    uint16_t y = bcdToDec(Wire.read() & 0xff) + 2000;
    return DateTime(y, m, d, hh, mm, ss).unixtime();
}


/*子函数,用于读取数据表中农历月的大月或小月,如果该月为大返回1,为小返回0*/
char RX8025 :: get_moon_day(uint8_t month_p,uint32_t table_addr)
{
uint8_t temp;
    switch (month_p){
        case 1:{temp=year_code[table_addr]&0x08;
             if (temp==0)return(0);else return(1);}
        case 2:{temp=year_code[table_addr]&0x04;
             if (temp==0)return(0);else return(1);}
        case 3:{temp=year_code[table_addr]&0x02;
             if (temp==0)return(0);else return(1);}
        case 4:{temp=year_code[table_addr]&0x01;
             if (temp==0)return(0);else return(1);}
        case 5:{temp=year_code[table_addr+1]&0x80;
             if (temp==0) return(0);else return(1);}
        case 6:{temp=year_code[table_addr+1]&0x40;
             if (temp==0)return(0);else return(1);}
        case 7:{temp=year_code[table_addr+1]&0x20;
             if (temp==0)return(0);else return(1);}
        case 8:{temp=year_code[table_addr+1]&0x10;
             if (temp==0)return(0);else return(1);}
        case 9:{temp=year_code[table_addr+1]&0x08;
             if (temp==0)return(0);else return(1);}
        case 10:{temp=year_code[table_addr+1]&0x04;
             if (temp==0)return(0);else return(1);}
        case 11:{temp=year_code[table_addr+1]&0x02;
             if (temp==0)return(0);else return(1);}
        case 12:{temp=year_code[table_addr+1]&0x01;
             if (temp==0)return(0);else return(1);}
        case 13:{temp=year_code[table_addr+2]&0x80;
             if (temp==0)return(0);else return(1);}
    }
    return 0;
}


/*
函数功能:输入BCD阳历数据,输出BCD阴历数据(只允许1901-2099年)
调用函数示例:Conversion(c_sun,year_sun,month_sun,day_sun)
如:计算2004年10月16日Conversion(0,0x4,0x10,0x16);
c_sun,year_sun,month_sun,day_sun均为BCD数据,c_sun为世纪标志位,c_sun=0为21世
纪,c_sun=1为19世纪
调用函数后,原有数据不变,读c_moon,year_moon,month_moon,day_moon得出阴历BCD数据
*/
void RX8025 :: Conversion(char c,uint8_t year,uint8_t month,uint8_t day)
{   //c=0 为21世纪,c=1 为19世纪 输入输出数据均为BCD数据
    year = decToBcd(year);
    month = decToBcd(month);
    day = decToBcd(day);
    uint8_t temp1,temp2,temp3,month_p;
    uint32_t temp4,table_addr;
    char flag2,flag_y;
    temp1=year/16;   //BCD->hex 先把数据转换为十六进制
    temp2=year%16;
    year=temp1*10+temp2;
    temp1=month/16;
    temp2=month%16;
    month=temp1*10+temp2;
    temp1=day/16;
    temp2=day%16;
    day=temp1*10+temp2;

    //定位数据表地址
    if(c==0){                   
        table_addr=(year+0x64-1)*0x3;
    }
    else {
        table_addr=(year-1)*0x3;
    }
    //定位数据表地址完成

    //取当年春节所在的公历月份
    temp1=year_code[table_addr+2]&0x60; 
    temp1=temp1>>5;
    //取当年春节所在的公历月份完成

    //取当年春节所在的公历日
    temp2=year_code[table_addr+2]&0x1f; 
    //取当年春节所在的公历日完成

    // 计算当年春年离当年元旦的天数,春节只会在公历1月或2月
    if(temp1==0x1){  
        temp3=temp2-1;  
    }  
    else{
        temp3=temp2+0x1f-1;        
    }
    // 计算当年春年离当年元旦的天数完成

    //计算公历日离当年元旦的天数,为了减少运算,用了两个表
    //day_code1[9],day_code2[3]
    //如果公历月在九月或前,天数会少于0xff,用表day_code1[9],
    //在九月后,天数大于0xff,用表day_code2[3]
    //如输入公历日为8月10日,则公历日离元旦天数为day_code1[8-1]+10-1
    //如输入公历日为11月10日,则公历日离元旦天数为day_code2[11-10]+10-1
    if (month<10){ 
        temp4=day_code1[month-1]+day-1;
    }
    else{
        temp4=day_code2[month-10]+day-1;
    }
    if ((month>0x2)&&(year%0x4==0)){  //如果公历月大于2月并且该年的2月为闰月,天数加1
        temp4+=1;
    }
    //计算公历日离当年元旦的天数完成

    //判断公历日在春节前还是春节后
    if (temp4>=temp3){ //公历日在春节后或就是春节当日使用下面代码进行运算
        temp4-=temp3;
        month=0x1;
        month_p=0x1;  //month_p为月份指向,公历日在春节前或就是春节当日month_p指向首月
        flag2=get_moon_day(month_p,table_addr); //检查该农历月为大小还是小月,大月返回1,小月返回0
        flag_y=0;
        if(flag2==0)temp1=0x1d; //小月29天
        else temp1=0x1e; //大小30天
        temp2=year_code[table_addr]&0xf0;
        temp2=temp2>>4;  //从数据表中取该年的闰月月份,如为0则该年无闰月
        while(temp4>=temp1){
            temp4-=temp1;
            month_p+=1;
            if(month==temp2){
            flag_y=~flag_y;
            if(flag_y==0)month+=1;
            }
            else month+=1;
            flag2=get_moon_day(month_p,table_addr);
            if(flag2==0)temp1=0x1d;
            else temp1=0x1e;
        }
        day=temp4+1;
    }
    else{  //公历日在春节前使用下面代码进行运算
        temp3-=temp4;
        if (year==0x0){year=0x63;c=1;}
        else year-=1;
        table_addr-=0x3;
        month=0xc;
        temp2=year_code[table_addr]&0xf0;
        temp2=temp2>>4;
        if (temp2==0)month_p=0xc; 
        else month_p=0xd; //
        /*
        month_p为月份指向,如果当年有闰月,一年有十三个月,月指向13,
无闰月指向12
        */
        flag_y=0;
        flag2=get_moon_day(month_p,table_addr);
        if(flag2==0)temp1=0x1d;
        else temp1=0x1e;
        while(temp3>temp1){
            temp3-=temp1;
            month_p-=1;
            if(flag_y==0)month-=1;
            if(month==temp2)flag_y=~flag_y;
            flag2=get_moon_day(month_p,table_addr);
            if(flag2==0)temp1=0x1d;
            else temp1=0x1e;
         }
        day=temp1-temp3+1;
    }
    c_moon=c;                 //HEX->BCD ,运算结束后,把数据转换为BCD数据
    temp1=year/10;
    temp1=temp1<<4;
    temp2=year%10;
    year_moon=temp1|temp2;
    temp1=month/10;
    temp1=temp1<<4;
    temp2=month%10;
    month_moon=temp1|temp2;
    temp1=day/10;
    temp1=temp1<<4;
    temp2=day%10;
    day_moon=temp1|temp2;
    
     c_moon     = bcdToDec( c_moon      ); 
     year_moon  = bcdToDec( year_moon   );     
     month_moon = bcdToDec( month_moon  );     
     day_moon   = bcdToDec( day_moon    );     
     week       = bcdToDec( week        ); 

}

