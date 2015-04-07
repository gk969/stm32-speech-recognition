#ifndef _MFCC_H
#define _MFCC_H
#include "stm32f10x.h"
#include "VAD.H"


#define hp_ratio	95/100			//预加重系数 0.95
#define fft_point	1024			//FFT点数
#define frq_max		(fft_point/2)	//最大频率
#define hamm_top	10000			//汉明窗最大值
#define	tri_top		1000			//三角滤波器顶点值
#define tri_num		24				//三角滤波器个数
#define mfcc_num	12				//MFCC阶数

#define vv_tim_max	1200	//单段有效语音最长时间 ms
#define vv_frm_max	((vv_tim_max-frame_time)/(frame_time-frame_mov_t)+1)	//单段有效语音最长帧数

#pragma pack(1)
typedef struct
{
	u16 save_sign;						//存储标记 用于判断flash中特征模板是否有效
	u16 frm_num;						//帧数
	s16 mfcc_dat[vv_frm_max*mfcc_num];	//MFCC转换结果
}v_ftr_tag;								//语音特征结构体
#pragma pack()

void get_mfcc(valid_tag *valid, v_ftr_tag *v_ftr, atap_tag *atap_arg);

#endif
