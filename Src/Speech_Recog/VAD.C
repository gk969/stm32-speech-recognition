/********     VAD.C       *******/

#include "stm32f10x.h"
#include "ADC.h"
#include "VAD.H"
#include "USART.H"

#define n_thl_ratio	1		//噪声门限系数 n_thl=n_max_mean*n_thl_ratio
#define s_thl_ratio	11/10	//短时幅度判决门限系数 s_thl=sum_mean*s_thl_ratio
#define z_thl_ratio	2/160	//短时过零率 判决门限系数 常数


#define atap_frm_t		30						//背景噪音自适应时间帧长度 ms
#define atap_frm_len	((fs/1000)*atap_frm_t)	//背景噪音自适应帧长度


/* 	求取自适应参数
	noise	：噪声起始点
	n_len	：噪声长度
	atap	；自适应参数
*/
void noise_atap(const u16* noise,u16 n_len,atap_tag* atap)
{
	u32 h,i;
	u32	n_max;	
	u32 max_sum;//每一帧噪声最大值 累加取平均 求噪声阈值
	u32	n_sum;	//所有数值之和 求平均值 确定零(中)值
	u32 mid;	//中值
	u32 abs;	//绝对值
	u32 abs_sum;//绝对值和
	u32 frm_num;
	
	if((n_len%atap_frm_len)!=0)	//参数检查
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
			if(abs>n_max)	//取每帧最大绝对值
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
#define	v_durmin_t	80	//有效语音最短时间门限 ms
#define	v_durmin_f	v_durmin_t/(frame_time-frame_mov_t)	//有效语音最短帧数
#define	s_durmax_t	110	//无声段最长时间门限 ms
#define	s_durmax_f	s_durmax_t/(frame_time-frame_mov_t)//无声段最长帧数

/*******
	VAD	(Voice activity detection) 语音活动检测
	检测出一段声音中的有效语音 起始点和长度 最多3段语音
	
	短时幅度 短时过零率求取：
	短时幅度直接累加
	短时过零率改进为过门限率，设置正负两个绝对值相等的门限。
	构成门限带，穿过门限带计作过零

	端点判决：
	1.判断语音起始点，要求能够滤除突发性噪声
	突发性噪声可以引起短时能量或过零率的数值很高，但是往往不能维持足够长的时间，
	如门窗的开关，物体的碰撞等引起的噪声，这些都可以通过设定最短时间门限来判别。
	超过两门限之一或全部，并且持续时间超过有效语音最短时间门限，
	返回最开始超过门限的时间点，将其标记为有效语音起始点。
	
	2.判断语音结束点，要求不能丢弃连词中间短暂的有可能被噪声淹没的“寂静段”
	同时低于两门限，并且持续时间超过无声最长时间门限，
	返回最开始低于门限的时间点，将其标记为有效语音结束点。
*********/
void VAD(const u16 *vc, u16 buf_len, valid_tag *valid_voice, atap_tag *atap_arg)
{
	u8	last_sig=0;	// 上次跃出门限带的状态 1:门限带以下；2:门限带以上
	u8	cur_stus=0;	// 当前语音段状态 0无声段  1前端过渡段  2语音段  3后端过渡段
	u16 front_duration=0;//前端过渡段超过门限值持续帧数
	u16 back_duration=0;//后端过渡段低于门限值持续帧数
	u32 h,i;
	u32 frm_sum;	// 短时绝对值和
	u32 frm_zero;	// 短时过零(门限)率
	u32 a_thl;	// 上门限值
	u32 b_thl;	// 下门限值
	
	u8 	valid_con=0;//语音段计数 最大max_vc_con
	u32 frm_con=0;	//帧计数
	
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
		for(h=0;h<frame_len;h++)//短时绝对值和
		{
			frm_sum+=(*(vc+i+h)>(atap_arg->mid_val))?(*(vc+i+h)-(atap_arg->mid_val)):((atap_arg->mid_val)-*(vc+i+h));
		}
		
		frm_zero=0;
		for(h=0;h<(frame_len-1);h++)//短时过门限率
		{
			if(*(vc+i+h)>=a_thl)			//大于上门限值
			{
				last_sig=2;
			}
			else if(*(vc+i+h)<b_thl)	//小于下门限值
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
		//至少有一个参数超过其门限值
		{
			if(cur_stus==0) //如果当前是无声段
			{
				cur_stus=1; //进入前端过渡段 
				front_duration=1; //前端过渡段持续帧数置1 第一帧
			}
			else if(cur_stus==1) //当前是前端过渡段
			{
				front_duration++;
				if(front_duration>=v_durmin_f) //前端过渡段帧数超过最短有效语音帧数
				{
					cur_stus=2; //进入语音段
					((valid_tag*)(valid_voice+valid_con))->start=(u16*)vc+i-((v_durmin_f-1)*(frame_len-frame_mov));//记录起始帧位置
					front_duration=0; //前端过渡段持续帧数置0
				}
			}
			else if(cur_stus==3) //如果当前是后端过渡段 两参数都回升到门限值以上
			{
				back_duration=0;
				cur_stus=2; //记为语音段
			}
		}
		else //两参数都在门限值以下
		{
			if(cur_stus==2) //当前是语音段
			{
				cur_stus=3;//设为后端过渡段
				back_duration=1; //前端过渡段持续帧数置1 第一帧
			}
			else if(cur_stus==3)//当前是后端过渡段
			{
				back_duration++;
				if(back_duration>=s_durmax_f) //后端过渡段帧数超过最长无声帧数
				{
					cur_stus=0; //进入无声段
					((valid_tag*)(valid_voice+valid_con))->end=(u16*)vc+i-(s_durmax_f*(frame_len-frame_mov))+frame_len;//记录结束帧位置
					valid_con++;
					if(valid_con==max_vc_con)
					{
						return;
					}
					back_duration=0;
				}
			}
			else if(cur_stus==1)//当前是前端过渡段 两参数都回落到门限值以下   
								//持续时间低于语音最短时间门限 视为短时噪声
			{
				front_duration=0;
				cur_stus=0; //记为无声段
			}
		}
	}
}




