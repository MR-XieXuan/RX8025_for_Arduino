# RX8025_for_Arduino
RX8025芯片在Arduino下的驱动库
# 1  RX8025 
rx8025是一个拥有I2C接口和温度补偿功能的新型实时时钟芯片。芯片可以将时间发送给rx8025。该芯片能被用来设定和读取年，月，日，星期，时，分，秒 时间信息。年份为后两位数字表示， 任何可以被4 整除的年份被当成闰年处理。（2000 年到2099 年)。
*** 
# 2 RX8025 库
## 2.1 库介绍
&emsp;**本库可以与RX8025以与芯片进行I2C通讯的形式设定RX8025的时、分、秒、年、月、日以及星期。而且库内拥有<font color="red"> 阳历 转 农历  </font> 的 函数 (** ***仅支持1900年-2099年*** **)**

## 2.2 库下载地址
&emsp;[RX8025 for Arduino : https://github.com/MR-XieXuan/RX8025_for_Arduino](https://github.com/MR-XieXuan/RX8025_for_Arduino) <br/>
&emsp;<font color="red"> 如果下载或者安装遇到困难，欢迎在评论区留言寻求大家的帮助，或者给作者发私信。作者的联系方式将放在文章底部。谢谢！ </font>
***
# 3 库的使用
## 3.1 开始使用
&emsp;如果您正确的安装这个库在Arduino上了呢，就已经可以使用这个库了。
使用这个库的第一步就是需要引入这个库并且创建一个RX8025对象；
> #include <RX8025.h>
> RX8025 rtc;
## 3.2 初始化设备
&emsp;正常使用前我们需要前初始化一下RX8025芯片，这里的初始化并不是清空RX8025芯片里面原有的信息，而是与RX8025建立IIC连接。
>rtc.RX8025_init();
## 3.3 给芯片写入时间
&emsp;给芯片写入时间的接口为
```c
/******************************************
* Function name : setRtcTime 
* Return : void ;
* @ s : 秒
* @ m : 分
* @ h : 时
* @ w : 星期 (星期天为0x00)
* @ d : 日
* @ mh : 月
* @ y : 年 (20xx) 如2022年 就是 22
********************************************/
void RX8025 :: setRtcTime(uint8_t s, uint8_t m, uint8_t h, uint8_t w ,uint8_t d, uint8_t mh, uint8_t y);

```
&emsp;如 ：
>rtc.setRtcTime(12,12,12,0,8,7,22); // 向RX8025芯片写入时间 2022年7月8日12点12分12秒
## 3.4 从芯片读取时间
&emsp; 调用以下函数即可读取芯片内部的时间：
&emsp; 读取当前年份；
> rtc.getYear(); 

&emsp; 读取当前月份；
> rtc.getMonth(); 

&emsp; 读取当前为几日；
> rtc.getDate();  

&emsp; 读取当前星期；
> rtc.getDoW();


&emsp; 读取当前几时；
> rtc.getHour();  

&emsp; 读取当前几分；
> rtc.getMinute();
 
 &emsp; 读取当前几秒；
> rtc.getSecond();

## 3.5 库内彩蛋
### 3.5.1 阳历阴历转换器
&emsp; 库内有从 <font color="red">阳历</font> 转换为 <font color="red">阴历(农历)</font> 的接口，方便使用者直接进行转换，超长有效使用时间1900年到2099年。
&emsp;使用方式: 调用接口输入参数 后 读取
&emsp;输入接口 ： 
```c
/*******************************
* Function : Conversion
*  @ c 可能出现以下参数
* 		$ 0x00 : 20xx年
* 		$ 0x01 : 19xx年
*  @ year 当前世纪的第几年
*  @ month 几月
*  @ day   几日 
*******************************/
void Conversion(char c,uint8_t year,uint8_t month,uint8_t day); 
```
> rtc.Conversion(0,22,7,8);

&emsp; 读取方式  : 直接读取对象内的成员；
```c
	unsigned char 	c_moon,		// 农历世纪标志位
					year_moon,	// 农历年份
					month_moon,	// 农历月份
					day_moon;	// 农历日
```
### 3.5.2 时间戳解析
&emsp; 库内还有一个时间戳解析对象 DataTime 最简单的使用方式为：
&emsp; <font color="red">伪代码：</font>
> // t 为时间戳 1970年 起后的某一秒钟
> DataTime( t ).year();  			// 获取 时间戳的年
> DataTime( t ).month();			// 获取 时间戳的月
> DataTime( t ).day();				// 获取 时间戳的日
> DataTime( t ).hour();			// 获取 时间戳的时
> DataTime( t ).minute();			// 获取 时间戳的分
> DataTime( t ).second();		// 获取 时间戳的秒

&emsp; 另外还有计算现在为星期几的方法 : 
&emsp; <font color="red">now() 为 时间戳 </font>
> (DateTime(now()).year() -2000) + ((DateTime(now()).year() -2000)/4) + (13 * ( DateTime(now()).month() +1) / 5) + DateTime(now()).day() - 36;

***
# 4 联系作者
QQ : 3325629928
E-mail : Mr_Xie_@outlook.com
Web : https://main.mrxie.xyz

如果在任何方面遇到问题，欢迎联系作者，或者在下方留言寻求大家的帮助，如果库在使用过程中出现问题，可以在 GitHub 社区内创立 Issues 让开发者更快的修复您提出的问题。


