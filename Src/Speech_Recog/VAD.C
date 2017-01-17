/********     VAD.C       *******/

#include "stm32f10x.h"
#include "ADC.h"
#include "VAD.H"
#include "USART.H"

#define n_thl_ratio	1		//��������ϵ�� n_thl=n_max_mean*n_thl_ratio
#define s_thl_ratio	11/10	//��ʱ�����о�����ϵ�� s_thl=sum_mean*s_thl_ratio
#define z_thl_ratio	2/160	//��ʱ������ �о�����ϵ�� ����


#define atap_frm_t		30						//������������Ӧʱ��֡���� ms
#define atap_frm_len	((fs/1000)*atap_frm_t)	//������������Ӧ֡����


/* 	��ȡ����Ӧ����
	noise	��������ʼ��
	n_len	����������
	atap	������Ӧ����
*/
void noise_atap(const u16* noise,u16 n_len,atap_tag* atap)
{
	u32 h,i;
	u32	n_max;	
	u32 max_sum;//ÿһ֡�������ֵ �ۼ�ȡƽ�� ��������ֵ
	u32	n_sum;	//������ֵ֮�� ��ƽ��ֵ ȷ����(��)ֵ
	u32 mid;	//��ֵ
	u32 abs;	//����ֵ
	u32 abs_sum;//����ֵ��
	u32 frm_num;
	
	if((n_len%atap_frm_len)!=0)	//�������
	{
		return;
	}
	frm_num=n_len/atap_frm_len;
	
	n_sum=0;
	max_sum=0;
	for(i=0;i<n_len;i++)
	{
		n_sum+=*(noise+i);
	}
	mid=n_sum/i;
	
	abs_sum=0;
	for(i=0;i<n_len;i+=atap_frm_len)
	{
		n_max=0;
		for(h=0;h<atap_frm_len;h++)
		{
			abs=(*(noise+i+h)>mid)?(*(noise+i+h)-mid):(mid-*(noise+i+h));
			if(abs>n_max)	//ȡÿ֡������ֵ
			{
				n_max=abs;
			}
			abs_sum+=abs;
		}
		max_sum+=n_max;
		//USART1_printf("n_max=%d ", n_max);
		//USART1_printf("max_sum=%d\r\n", max_sum);
	}
	
	abs_sum/=(n_len/frame_len);
	max_sum/=frm_num;
	atap->mid_val=mid;
	atap->n_thl=max_sum*n_thl_ratio;
	atap->s_thl=abs_sum*s_thl_ratio;
	atap->z_thl=frame_len*z_thl_ratio/n_thl_ratio;
}
#define	v_durmin_t	80	//��Ч�������ʱ������ ms
#define	v_durmin_f	v_durmin_t/(frame_time-frame_mov_t)	//��Ч�������֡��
#define	s_durmax_t	110	//�������ʱ������ ms
#define	s_durmax_f	s_durmax_t/(frame_time-frame_mov_t)//�������֡��

/*******
	VAD	(Voice activity detection) ��������
	����һ�������е���Ч���� ��ʼ��ͳ��� ���3������
	
	��ʱ���� ��ʱ��������ȡ��
	��ʱ����ֱ���ۼ�
	��ʱ�����ʸĽ�Ϊ�������ʣ�����������������ֵ��ȵ����ޡ�
	�������޴����������޴���������

	�˵��о���
	1.�ж�������ʼ�㣬Ҫ���ܹ��˳�ͻ��������
	ͻ�����������������ʱ����������ʵ���ֵ�ܸߣ�������������ά���㹻����ʱ�䣬
	���Ŵ��Ŀ��أ��������ײ���������������Щ������ͨ���趨���ʱ���������б�
	����������֮һ��ȫ�������ҳ���ʱ�䳬����Ч�������ʱ�����ޣ�
	�����ʼ�������޵�ʱ��㣬������Ϊ��Ч������ʼ�㡣
	
	2.�ж����������㣬Ҫ���ܶ��������м���ݵ��п��ܱ�������û�ġ��ž��Ρ�
	ͬʱ���������ޣ����ҳ���ʱ�䳬�������ʱ�����ޣ�
	�����ʼ�������޵�ʱ��㣬������Ϊ��Ч���������㡣
*********/
void VAD(const u16 *vc, u16 buf_len, valid_tag *valid_voice, atap_tag *atap_arg)
{
	u8	last_sig=0;	// �ϴ�Ծ�����޴���״̬ 1:���޴����£�2:���޴�����
	u8	cur_stus=0;	// ��ǰ������״̬ 0������  1ǰ�˹��ɶ�  2������  3��˹��ɶ�
	u16 front_duration=0;//ǰ�˹��ɶγ�������ֵ����֡��
	u16 back_duration=0;//��˹��ɶε�������ֵ����֡��
	u32 h,i;
	u32 frm_sum;	// ��ʱ����ֵ��
	u32 frm_zero;	// ��ʱ����(����)��
	u32 a_thl;	// ������ֵ
	u32 b_thl;	// ������ֵ
	
	u8 	valid_con=0;//�����μ��� ���max_vc_con
	u32 frm_con=0;	//֡����
	
	a_thl=atap_arg->mid_val+atap_arg->n_thl;
	b_thl=atap_arg->mid_val-atap_arg->n_thl;
	
	for(i=0;i<max_vc_con;i++)
	{
		((valid_tag*)(valid_voice+i))->start=(void *)0;
		((valid_tag*)(valid_voice+i))->end=(void *)0;
	}
	
	for(i=0;i<(buf_len-frame_len);i+=(frame_len-frame_mov))
	{
		frm_con++;

		frm_sum=0;
		for(h=0;h<frame_len;h++)//��ʱ����ֵ��
		{
			frm_sum+=(*(vc+i+h)>(atap_arg->mid_val))?(*(vc+i+h)-(atap_arg->mid_val)):((atap_arg->mid_val)-*(vc+i+h));
		}
		
		frm_zero=0;
		for(h=0;h<(frame_len-1);h++)//��ʱ��������
		{
			if(*(vc+i+h)>=a_thl)			//����������ֵ
			{
				last_sig=2;
			}
			else if(*(vc+i+h)<b_thl)	//С��������ֵ
			{
				last_sig=1;
			}
			
			if(*(vc+i+h+1)>=a_thl)
			{
				if(last_sig==1)
				{
					frm_zero++;
				}
			}
			else if(*(vc+i+h+1)<b_thl)
			{
				if(last_sig==2)
				{
					frm_zero++;
				}
			}
		}
		/*	 	
		USART1_printf("frm_con=%d ",frm_con);
		USART1_printf("frm_sum=%d ",frm_sum);
		USART1_printf("frm_zero=%d\r\n",frm_zero); 
		*/ 
		
		if((frm_sum>(atap_arg->s_thl))||(frm_zero>(atap_arg->z_thl)))
		//������һ����������������ֵ
		{
			if(cur_stus==0) //�����ǰ��������
			{
				cur_stus=1; //����ǰ�˹��ɶ� 
				front_duration=1; //ǰ�˹��ɶγ���֡����1 ��һ֡
			}
			else if(cur_stus==1) //��ǰ��ǰ�˹��ɶ�
			{
				front_duration++;
				if(front_duration>=v_durmin_f) //ǰ�˹��ɶ�֡�����������Ч����֡��
				{
					cur_stus=2; //����������
					((valid_tag*)(valid_voice+valid_con))->start=(u16*)vc+i-((v_durmin_f-1)*(frame_len-frame_mov));//��¼��ʼ֡λ��
					front_duration=0; //ǰ�˹��ɶγ���֡����0
				}
			}
			else if(cur_stus==3) //�����ǰ�Ǻ�˹��ɶ� ������������������ֵ����
			{
				back_duration=0;
				cur_stus=2; //��Ϊ������
			}
		}
		else //��������������ֵ����
		{
			if(cur_stus==2) //��ǰ��������
			{
				cur_stus=3;//��Ϊ��˹��ɶ�
				back_duration=1; //ǰ�˹��ɶγ���֡����1 ��һ֡
			}
			else if(cur_stus==3)//��ǰ�Ǻ�˹��ɶ�
			{
				back_duration++;
				if(back_duration>=s_durmax_f) //��˹��ɶ�֡�����������֡��
				{
					cur_stus=0; //����������
					((valid_tag*)(valid_voice+valid_con))->end=(u16*)vc+i-(s_durmax_f*(frame_len-frame_mov))+frame_len;//��¼����֡λ��
					valid_con++;
					if(valid_con==max_vc_con)
					{
						return;
					}
					back_duration=0;
				}
			}
			else if(cur_stus==1)//��ǰ��ǰ�˹��ɶ� �����������䵽����ֵ����   
								//����ʱ������������ʱ������ ��Ϊ��ʱ����
			{
				front_duration=0;
				cur_stus=0; //��Ϊ������
			}
		}
	}
}



