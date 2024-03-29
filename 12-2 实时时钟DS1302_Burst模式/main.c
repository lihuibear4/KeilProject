
#include <reg52.h>

sbit DS1302_CE = P1^7;
sbit DS1302_CK = P3^5;
sbit DS1302_IO = P3^4;

bit flag200ms = 0;       //200ms定时标志
unsigned char T0RH = 0;  //T0重载值的高字节
unsigned char T0RL = 0;  //T0重载值的低字节

void ConfigTimer0(unsigned int ms);
void InitDS1302();
void DS1302BurstRead(unsigned char *dat);
extern void InitLcd1602();
extern void LcdShowStr(unsigned char x, unsigned char y, unsigned char *str);

void main()
{
    unsigned char psec=0xAA;  //秒备份，初值AA确保首次读取时间后会刷新显示
    unsigned char time[8];    //当前时间数组
    unsigned char str[12];    //字符串转换缓冲区

    EA = 1;           //开总中断
    ConfigTimer0(1);  //T0定时1ms
    InitDS1302();     //初始化实时时钟
    InitLcd1602();    //初始化液晶
    
    while (1)
    {
        if (flag200ms)  //每200ms读取依次时间
        {
            flag200ms = 0;
            DS1302BurstRead(time); //读取DS1302当前时间
            if (psec != time[0])   //检测到时间有变化时刷新显示
            {
                str[0] = '2';  //添加年份的高2位：20
                str[1] = '0';
                str[2] = (time[6] >> 4) + '0';  //“年”高位数字转换为ASCII码
                str[3] = (time[6]&0x0F) + '0';  //“年”低位数字转换为ASCII码
                str[4] = '-';  //添加日期分隔符
                str[5] = (time[4] >> 4) + '0';  //“月”
                str[6] = (time[4]&0x0F) + '0';
                str[7] = '-';
                str[8] = (time[3] >> 4) + '0';  //“日”
                str[9] = (time[3]&0x0F) + '0';
                str[10] = '\0';
                LcdShowStr(0, 0, str);  //显示到液晶的第一行
                
                str[0] = (time[5]&0x0F) + '0';  //“星期”
                str[1] = '\0';
                LcdShowStr(11, 0, "week");
                LcdShowStr(15, 0, str);  //显示到液晶的第一行
                
                str[0] = (time[2] >> 4) + '0';  //“时”
                str[1] = (time[2]&0x0F) + '0';
                str[2] = ':';  //添加时间分隔符
                str[3] = (time[1] >> 4) + '0';  //“分”
                str[4] = (time[1]&0x0F) + '0';
                str[5] = ':';
                str[6] = (time[0] >> 4) + '0';  //“秒”
                str[7] = (time[0]&0x0F) + '0';
                str[8] = '\0';
                LcdShowStr(4, 1, str);  //显示到液晶的第二行
                
                psec = time[0];  //用当前值更新上次秒数
            }
        }
    }
}

/* 发送一个字节到DS1302通信总线上 */
void DS1302ByteWrite(unsigned char dat)
{
    unsigned char mask;
    
    for (mask=0x01; mask!=0; mask<<=1)  //低位在前，逐位移出
    {
        if ((mask&dat) != 0) //首先输出该位数据
            DS1302_IO = 1;
        else
            DS1302_IO = 0;
        DS1302_CK = 1;       //然后拉高时钟
        DS1302_CK = 0;       //再拉低时钟，完成一个位的操作
    }
    DS1302_IO = 1;           //最后确保释放IO引脚
}
/* 由DS1302通信总线上读取一个字节 */
unsigned char DS1302ByteRead()
{
    unsigned char mask;
    unsigned char dat = 0;
    
    for (mask=0x01; mask!=0; mask<<=1)  //低位在前，逐位读取
    {
        if (DS1302_IO != 0)  //首先读取此时的IO引脚，并设置dat中的对应位
        {
            dat |= mask;
        }
        DS1302_CK = 1;       //然后拉高时钟
        DS1302_CK = 0;       //再拉低时钟，完成一个位的操作
    }
    return dat;              //最后返回读到的字节数据
}
/* 用单次写操作向某一寄存器写入一个字节，reg-寄存器地址，dat-待写入字节 */
void DS1302SingleWrite(unsigned char reg, unsigned char dat)
{
    DS1302_CE = 1;                   //使能片选信号
    DS1302ByteWrite((reg<<1)|0x80);  //发送写寄存器指令
    DS1302ByteWrite(dat);            //写入字节数据
    DS1302_CE = 0;                   //除能片选信号
}
/* 用单次读操作从某一寄存器读取一个字节，reg-寄存器地址，返回值-读到的字节 */
unsigned char DS1302SingleRead(unsigned char reg)
{
    unsigned char dat;
    
    DS1302_CE = 1;                   //使能片选信号
    DS1302ByteWrite((reg<<1)|0x81);  //发送读寄存器指令
    dat = DS1302ByteRead();          //读取字节数据
    DS1302_CE = 0;                   //除能片选信号
    
    return dat;
}
void  DS1302BurstWrite(unsigned char *dat)
{
	unsigned char i;

	DS1302_CE = 1;
	DS1302ByteWrite(0xBE);
	for(i=0; i<8; i++)
	{
		DS1302ByteWrite(dat[i]);
	}
	DS1302_CE = 0;
}

void  DS1302BurstRead(unsigned char *dat)
{
	unsigned char i;

	DS1302_CE = 1;
	DS1302ByteWrite(0xBF);
	for(i=0; i<8; i++)
	{
		dat[i] = DS1302ByteRead();
	}
	DS1302_CE = 0;
}
/* DS1302初始化，如发生掉电则重新设置初始时间 */
void InitDS1302()
{
    unsigned char dat;
    unsigned char code InitTime[] = {  //
        0x00,0x30,0x12, 0x08, 0x10, 0x02, 0x25
    };
    
    DS1302_CE = 0;  //初始化DS1302通信引脚
    DS1302_CK = 0;
    dat = DS1302SingleRead(0);  //读取秒寄存器
    if ((dat & 0x80) != 0)      //由秒寄存器最高位CH的值判断DS1302是否已停止
    {
        DS1302SingleWrite(7, 0x00);  //撤销写保护以允许写入数据
        DS1302BurstWrite(InitTime);  //设置DS1302为默认的初始时间
    }
}
/* 配置并启动T0，ms-T0定时时间 */
void ConfigTimer0(unsigned int ms)
{
    unsigned long tmp;  //临时变量
    
    tmp = 11059200 / 12;      //定时器计数频率
    tmp = (tmp * ms) / 1000;  //计算所需的计数值
    tmp = 65536 - tmp;        //计算定时器重载值
    tmp = tmp + 12;           //补偿中断响应延时造成的误差
    T0RH = (unsigned char)(tmp>>8);  //定时器重载值拆分为高低字节
    T0RL = (unsigned char)tmp;
    TMOD &= 0xF0;   //清零T0的控制位
    TMOD |= 0x01;   //配置T0为模式1
    TH0 = T0RH;     //加载T0重载值
    TL0 = T0RL;
    ET0 = 1;        //使能T0中断
    TR0 = 1;        //启动T0
}
/* T0中断服务函数，执行200ms定时 */
void InterruptTimer0() interrupt 1
{
    static unsigned char tmr200ms = 0;
    
    TH0 = T0RH;  //重新加载重载值
    TL0 = T0RL;
    tmr200ms++;
    if (tmr200ms >= 200)  //定时200ms
    {
        tmr200ms = 0;
        flag200ms = 1;
    }
}
