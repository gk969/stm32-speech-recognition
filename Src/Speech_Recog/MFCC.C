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
	cr4_fft_1024_stm32����������з�����

	cr4_fft_1024_stm32�����������ʵ��������
	����������ֻ����ʵ������ ������0���
	fft���������������ݳ���ʱ ����������0���
	
	cr4_fft_1024_stm32������ݰ���ʵ��������
	Ӧ��ȡ�����ֵ ��ƽ���͵ĸ�
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
		fft_in[i]=*(u16*)(dat_buf+i);//�鲿��λ ʵ����λ
		//USART1_printf("fft_in[%d]=%d\r\n",i,(s16)fft_in[i]);
	}
	for(;i<fft_point;i++)
	{
		fft_in[i]=0;//����������0���
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
	MFCC��MelƵ�ʵ���ϵ��
	
	������
	valid	��Ч����������յ�
	
	����ֵ��
	v_ftr	MFCCֵ��֡��
	
	Mel=2595*lg(1+f/700)
	1000Hz���°����Կ̶� 1000Hz���ϰ������̶�
	�������˲�������Ƶ�� ��MelƵ�ʿ̶��ϵȼ������
	Ԥ����:6dB/��Ƶ�� һ�׸�ͨ�˲���  H(z)=1-uz^(-1) y(n)=x(n)-ux(n-1) u=0.94~0.97

	MFCC ���裺
	1.�������ź�Ԥ���ء���֡���Ӻ�����������Ȼ����ж�ʱ����Ҷ�任���ó�Ƶ��
	2.ȡƵ��ƽ�����������ס�����24��Mel��ͨ�˲��������˲������Mel������
	3.��ÿ���˲��������ֵȡ�������õ���ӦƵ���Ķ��������ס�Ȼ���24���������ʽ���
	  ����ɢ���ұ任�õ�12��MFCCϵ��
*/

void get_mfcc(valid_tag *valid, v_ftr_tag *v_ftr, atap_tag *atap_arg)
{
	u16 *vc_dat;
	u16 h,i;
	u32 *frq_spct;			//Ƶ��
	s16	vc_temp[frame_len];	//�����ݴ���
	s32	temp;
	
	u32	pow_spct[tri_num];	//�����˲����������������
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
				//Ԥ����
				//USART1_printf("vc_dat[%d]=%d ",i,*(vc_dat+i)-mid);
				temp=((s32)(*(vc_dat+i))-mid)-((s32)(*(vc_dat+i-1))-mid)*hp_ratio; 
				//USART1_printf("vc_hp[%d]=%d ",i,temp);
				//�Ӻ����� ���Ŵ�10��
				vc_temp[i]=(s16)(temp*hamm[i]/(hamm_top/10));
				//USART1_printf("vc_hm[%d]=%d\r\n",i,vc_temp[i]);
			}
			
			frq_spct=fft(vc_temp,frame_len);
			
			for(i=0;i<frq_max;i++)
			{
				//USART1_printf("frq_spct[%d]=%d ",i,frq_spct[i]);
				frq_spct[i]*=frq_spct[i];//������
				//USART1_printf("E_spct[%d]=%d\r\n",i,frq_spct[i]);
			}
			
			//�������˲���
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
			
			//�����˲������ȡ����
			for(h=0;h<tri_num;h++)
			{
				//USART1_printf("pow_spct[%d]=%d ",h,pow_spct[h]);
				pow_spct[h]=(u32)(log(pow_spct[h])*100);//ȡ������ ��100 ����������Чλ��
				//USART1_printf("%d\r\n",pow_spct[h]);
			}
			
			//����ɢ���ұ任
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

	