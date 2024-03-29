#include<reg52.h>

sbit LED = P0^0;
sbit ADDR0 = P1^0;
sbit ADDR1 = P1^1;
sbit ADDR2 = P1^2;
sbit ADDR3 = P1^3;
sbit ENLED = P1^4;

void main()
{
	unsigned char cnt = 0;
	unsigned char flag = 0; 
	//unsigned int i=0;
	ENLED = 0;
	ADDR3 = 1;
	ADDR2 = 1;
	ADDR1 = 1;
	ADDR0 = 0;

	TMOD = 0x01;
	TH0 = 0xB8; // x * 12 /11059200  = 0.2 x = 18432 TH0 = 65535 + 1 - x 
	TL0 = 0x00;
	TR0 = 1;
	
	while(1)
	{
		if(TF0 == 1)
		{
			TF0 = 0;
			TH0 = 0xB8;
			TL0 = 0x00;
			cnt++;
			if(cnt>=50)
			{
				cnt = 0;
				P0 = ~(0x01<<flag);
				flag++;
				if(flag>=8){
					flag = 0;
			}
		}
	}
 }
}