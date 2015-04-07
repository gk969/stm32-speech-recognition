#include "stm32f10x_conf.h"
#include "stm32f10x.h"
#include "SPI.h"
#include "touch_panel.h"
#include "TFTLCD.H" 

u8  touch_panel_read(u16 *X,u16 *Y)
{	
	u16 X_buffer[10],Y_buffer[10];
	u16 X_temp,Y_temp;
	float f_temp;
	u16 i=0;

	while(i<10&&touch)
	{
		Select_TSC();
		
			
		SPI1_Exchange_Data(CMD_RDY); 
		
		X_temp=SPI1_Exchange_Data(0);  
		X_temp=X_temp<<8;
		X_temp|=SPI1_Exchange_Data(0);
		X_temp>>=3; 
			  
	    SPI1_Exchange_Data(CMD_RDX);  
	
		Y_temp=SPI1_Exchange_Data(0);
		Y_temp=Y_temp<<8;
		Y_temp|=SPI1_Exchange_Data(0);
		Y_temp>>=3; 
		 
		DisSelect_TSC(); 
		
		if(X_temp>100&&X_temp<4000&&Y_temp>100&&Y_temp<4000)
		{
			X_buffer[i]=X_temp;
			Y_buffer[i]=Y_temp;
			i++;
			
		}
	}
	
	if(i==10)
	{
		for(i=0;i<9;i++)
		{
			if(X_buffer[i]>X_buffer[i+1])
			{
				X_temp=X_buffer[i];
				X_buffer[i]=X_buffer[i+1];
				X_buffer[i+1]=X_temp;
			}
			if(Y_buffer[i]>Y_buffer[i+1])
			{
				Y_temp=Y_buffer[i];
				Y_buffer[i]=Y_buffer[i+1];
				Y_buffer[i+1]=Y_temp;
			}
		}
		for(i=8;i>0;i--)
		{
			if(X_buffer[i]<X_buffer[i-1])
			{
				X_temp=X_buffer[i];
				X_buffer[i]=X_buffer[i-1];
				X_buffer[i-1]=X_temp;
			}
			if(Y_buffer[i]<Y_buffer[i-1])
			{
				Y_temp=Y_buffer[i];
				Y_buffer[i]=Y_buffer[i-1];
				Y_buffer[i-1]=Y_temp;
			}
		}
		X_temp=0;Y_temp=0;
		for(i=1;i<9;i++)
		{
			X_temp+=X_buffer[i];
			Y_temp+=Y_buffer[i];
		}
		
		f_temp=253-0.06676*(X_temp/8);
		*X=f_temp;
		f_temp=348-0.09027*(Y_temp/8);
		*Y=f_temp;

		if(LCD_COORD==COORD_320_240)
		{
			i=*X;
			*X=*Y;
			*Y=240-i;
		}

		return 1;
	}
	else
		return 0;

}

