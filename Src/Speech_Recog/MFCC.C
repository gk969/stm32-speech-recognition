/*******   MFCC.C    *******/

#include "stm32f10x.h"
#include <math.h>
#include "ADC.h"
#include "VAD.H"
#include "MFCC.H"
#include "MFCC_Arg.h"
#include <float.h>
#include "USART.H"

void cr4_fft_1024_stm32(void *pssOUT, void *pssIN, u16 Nbin);

u32 fft_out[fft_point];	
u32	fft_in[fft_point];	

/*	
	cr4_fft_1024_stm32输入参数是有符号数

	cr4_fft_1024_stm32输入参数包括实数和虚数
	但语音数据只包括实数部分 虚数用0填充
	fft点数超出输入数据长度时 超过部分用0填充
	
	cr4_fft_1024_stm32输出数据包括实数和虚数
	应该取其绝对值 即平方和的根
*/
u32* fft(s16* dat_buf, u16 buf_len)
{
	u16 i;
	s32 real,imag;
	
	if(buf_len>fft_point)
	{
		return (void*)0;
	}
	
	for(i=0;i<buf_len;i++)
	{
		fft_in[i]=*(u16*)(dat_buf+i);//虚部高位 实部低位
		//USART1_printf("fft_in[%d]=%d\r\n",i,(s16)fft_in[i]);
	}
	for(;i<fft_point;i++)
	{
		fft_in[i]=0;//超出部分用0填充
	}
	
	cr4_fft_1024_stm32(fft_out,fft_in,fft_point);
	
	for(i=0;i<frq_max;i++)
	{
		real=(s16)(fft_out[i]);
		imag=(s16)((fft_out[i])>>16);

		//USART1_printf("%d %d",real,imag);

		real=real*real+imag*imag;
		//USART1_printf(" %d",real);
		fft_out[i]=sqrtf((float)real)*10;
		//USART1_printf(" %d\r\n",fft_out[i]);
	}
	return fft_out;
}


/*
	MFCC：Mel频率倒谱系数
	
	参数：
	valid	有效语音段起点终点
	
	返回值：
	v_ftr	MFCC值，帧数
	
	Mel=2595*lg(1+f/700)
	1000Hz以下按线性刻度 1000Hz以上按对数刻度
	三角型滤波器中心频率 在Mel频率刻度上等间距排列
	预加重:6dB/倍频程 一阶高通滤波器  H(z)=1-uz^(-1) y(n)=x(n)-ux(n-1) u=0.94~0.97

	MFCC 步骤：
	1.对语音信号预加重、分帧、加汉明窗处理，然后进行短时傅里叶变换，得出频谱
	2.取频谱平方，得能量谱。并用24个Mel带通滤波器进行滤波，输出Mel功率谱
	3.对每个滤波器的输出值取对数，得到相应频带的对数功率谱。然后对24个对数功率进行
	  反离散余弦变换得到12个MFCC系数
*/

void get_mfcc(valid_tag *valid, v_ftr_tag *v_ftr, atap_tag *atap_arg)
{
	u16 *vc_dat;
	u16 h,i;
	u32 *frq_spct;			//频谱
	s16	vc_temp[frame_len];	//语音暂存区
	s32	temp;
	
	u32	pow_spct[tri_num];	//三角滤波器输出对数功率谱
	u16 frm_con;
	s16 *mfcc_p;
	s8	*dct_p;
	s32 mid;
	u16 v_frm_num;
	
	//USART1_printf("start=%d end=%d",(u32)(valid->start),(u32)(valid->end));
	v_frm_num=(((u32)(valid->end)-(u32)(valid->start))/2-frame_len)/(frame_len-frame_mov)+1;
	if(v_frm_num>vv_frm_max)
	{
		USART1_printf("frm_num=%d ",v_frm_num);
		v_ftr->frm_num=0;
	}
	else
	{
		mid=(s32)atap_arg->mid_val;
		mfcc_p=v_ftr->mfcc_dat;
		frm_con=0;
		for(vc_dat=(u16*)(valid->start);vc_dat<=((u16*)(valid->end-frame_len));vc_dat+=(frame_len-frame_mov))
		{
			for(i=0;i<frame_len;i++)
			{
				//预加重
				//USART1_printf("vc_dat[%d]=%d ",i,*(vc_dat+i)-mid);
				temp=((s32)(*(vc_dat+i))-mid)-((s32)(*(vc_dat+i-1))-mid)*hp_ratio; 
				//USART1_printf("vc_hp[%d]=%d ",i,temp);
				//加汉明窗 并放大10倍
				vc_temp[i]=(s16)(temp*hamm[i]/(hamm_top/10));
				//USART1_printf("vc_hm[%d]=%d\r\n",i,vc_temp[i]);
			}
			
			frq_spct=fft(vc_temp,frame_len);
			
			for(i=0;i<frq_max;i++)
			{
				//USART1_printf("frq_spct[%d]=%d ",i,frq_spct[i]);
				frq_spct[i]*=frq_spct[i];//能量谱
				//USART1_printf("E_spct[%d]=%d\r\n",i,frq_spct[i]);
			}
			
			//加三角滤波器
			pow_spct[0]=0;
			for(i=0;i<tri_cen[1];i++)
			{
				pow_spct[0]+=(frq_spct[i]*tri_even[i]/(tri_top/10));
			}
			for(h=2;h<tri_num;h+=2)
			{
				pow_spct[h]=0;
				for(i=tri_cen[h-1];i<tri_cen[h+1];i++)
				{
					pow_spct[h]+=(frq_spct[i]*tri_even[i]/(tri_top/10));
				}
			}
			
			for(h=1;h<(tri_num-2);h+=2)
			{
				pow_spct[h]=0;
				for(i=tri_cen[h-1];i<tri_cen[h+1];i++)
				{
					pow_spct[h]+=(frq_spct[i]*tri_odd[i]/(tri_top/10));
				}
			}
			pow_spct[tri_num-1]=0;
			for(i=tri_cen[tri_num-2];i<(fft_point/2);i++)
			{
				pow_spct[tri_num-1]+=(frq_spct[i]*tri_odd[i]/(tri_top/10));
			}
			
			//三角滤波器输出取对数
			for(h=0;h<tri_num;h++)
			{
				//USART1_printf("pow_spct[%d]=%d ",h,pow_spct[h]);
				pow_spct[h]=(u32)(log(pow_spct[h])*100);//取对数后 乘100 提升数据有效位数
				//USART1_printf("%d\r\n",pow_spct[h]);
			}
			
			//反离散余弦变换
			dct_p=(s8 *)dct_arg;
			for(h=0;h<mfcc_num;h++)
			{
				mfcc_p[h]=0;
				for(i=0;i<tri_num;i++)
				{
					mfcc_p[h]+=(((s32)pow_spct[i])*((s32)dct_p[i])/100);
				}
				//USART1_printf("%d,",mfcc_p[h]);
				dct_p+=tri_num;
			}
			//USART1_printf("\r\n");
			mfcc_p+=mfcc_num;
			frm_con++;
		}
		
		v_ftr->frm_num=frm_con;
	}
}

	
