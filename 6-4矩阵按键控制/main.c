#include<reg52.h>

sbit ADDR0 = P1^0;
sbit ADDR1 = P1^1;
sbit ADDR2 = P1^2;
sbit ADDR3 = P1^3;
sbit ENLED = P1^4;

sbit KEY_IN_1 = P2^4;
sbit KEY_IN_2 = P2^5;
sbit KEY_IN_3 = P2^6;
sbit KEY_IN_4 = P2^7;
sbit KEY_OUT_1 = P2^3;
sbit KEY_OUT_2 = P2^2;
sbit KEY_OUT_3 = P2^1;
sbit KEY_OUT_4 = P2^0;

unsigned char code LedChar[]={
	0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8,
	0x80, 0x90, 0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E
	};
unsigned char KeySta[4][4] = { 
	{1,1,1,1},{1,1,1,1},{1,1,1,1},{1,1,1,1}
	};

void main()
{
	unsigned char i, j;
	unsigned char backup [4][4] = {
	{1,1,1,1},{1,1,1,1},{1,1,1,1},{1,1,1,1}
	};
	

	EA = 1;
	ENLED = 0;
	ADDR3 = 1;
	ADDR2 = 0;
	ADDR1 = 0;
	ADDR0 = 0;
	TMOD = 0x01;
	TH0 = 0xFC;
	TL0 = 0x67;
	ET0 = 1;
	TR0 = 1;
	P0 = LedChar[0];

	while(1)
	{
		for(i=0; i<4; i++)
		{
			for(j=0; j<4; j++)
			{
				if(backup[i][j] != KeySta[i][j])
				{
					if(backup[i][j] == 0)
					{
						P0 = LedChar[i*4+j];
					}
					backup[i][j] = KeySta[i][j];
				}
			}	
		}	
	}
}

void InterruptTimer0() interrupt 1
{
	static unsigned char keyout = 0;
	unsigned char i = 0;
	static unsigned char keybuf[4][4] = {
	{0xFF, 0xFF, 0xFF, 0xFF},
	{0xFF, 0xFF, 0xFF, 0xFF},
	{0xFF, 0xFF, 0xFF, 0xFF},
	{0xFF, 0xFF, 0xFF, 0xFF}
	};
	

	TH0 = 0xFC;
	TL0 = 0x67;

	keybuf[keyout][0] = (keybuf[keyout][0] <<1) | KEY_IN_1;
	keybuf[keyout][1] = (keybuf[keyout][1] <<1) | KEY_IN_2;
	keybuf[keyout][2] = (keybuf[keyout][2] <<1) | KEY_IN_3;
	keybuf[keyout][3] = (keybuf[keyout][3] <<1) | KEY_IN_4;
	
	for(i=0; i<4; i++)
	{
		if((keybuf[keyout][i] & 0x0F) == 0x00)
		{
			KeySta[keyout][i] = 0;
		}
		else if((keybuf[keyout][i] & 0x0F) == 0x0F)
		{
			KeySta[keyout][i] = 1;
		}
	}

	keyout++;
	keyout = keyout & 0x03;
	switch(keyout)
	{
		case 0: KEY_OUT_4 = 1; KEY_OUT_1 = 0; break;
		case 1: KEY_OUT_1 = 1; KEY_OUT_2 = 0;  break;
		case 2: KEY_OUT_2 = 1; KEY_OUT_3 = 0; break;
		case 3: KEY_OUT_3 = 1; KEY_OUT_4 = 0;  break;
		default: break;

	}
}