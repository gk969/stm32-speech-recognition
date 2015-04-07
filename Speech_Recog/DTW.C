/*******       DTW.C         ********/

#include "stm32f10x.h"
#include <math.h>
#include "ADC.h"
#include "VAD.H"
#include "MFCC.H"
#include "DTW.H"
#include "USART.H"

/*	
	DTW算法 通过局部优化的方法实现加权距离总和最小
	
	时间规整函数：
	C={c(1),c(2),…,c(N)}
	N为路径长度，c(n)=(i(n),j(n))表示第n个匹配点是有参考模板的
第i(n)个特征矢量与待测模板的第j(n)个特征矢量构成的匹配点对，两
者之间的距离d(x(i(n)),y(j(n)))称为匹配距离。
	时间规整函数满足一下约束：
	1.单调性，规整函数单调增加。
	2.起点终点约束，起点对起点，终点对终点。
	3.连续性，不允许跳过任何一点。
	4.最大规整量不超过某一极限值。|i(n)-j(n)|<M,M为窗宽。规整
函数所处的区域位于平行四边形内。局部路径约束，用于限制当第n步
为(i(n),j(n))时，前几步存在几种可能的路径。

	DTW步骤：
	1.初始化。令i(0)=j(0)=0,i(N)=in_frm_num,j(N)=mdl_frm_num.
确定一个平行四边形，有两个位于(0,0)和(in_frm_num,mdl_frm_num)的顶点，相邻斜边斜
率分别是2和1/2.规整函数不可超出此平行四边形。
	2.递推求累计距离。
	
	若输入特征与特征模板的帧数差别过大，直接将匹配距离设为最大
	frm_in_num<(frm_mdl_num/2)||frm_in_num>(frm_mdl_num*2)
*/

/*
	获取两个特征矢量之间的距离
	参数
	frm_ftr1	特征矢量1
	frm_ftr2	特征矢量2
	返回值
	dis			矢量距离
*/
u32 get_dis(s16 *frm_ftr1, s16 *frm_ftr2)
{
	u8 	i;
	u32	dis;
	s32 dif;	//两矢量相同维度上的差值
	
	dis=0;
	for(i=0;i<mfcc_num;i++)
	{
		//USART1_printf("dis=%d ",dis);
		dif=frm_ftr1[i]-frm_ftr2[i];
		dis+=(dif*dif);
	}
	//USART1_printf("dis=%d ",dis);
	dis=sqrtf(dis);
	//USART1_printf("%d\r\n",dis);
	return dis;
}

//平行四边形两外两顶点 X坐标值
static u16	X1;			//上边交点
static u16	X2;			//下边交点
static u16	in_frm_num;	//输入特征帧数
static u16	mdl_frm_num;//特征模板帧数

#define ins		0
#define outs	1

/*
	范围控制
*/
u8 dtw_limit(u16 x, u16 y)
{
	if(x<X1)
	{
		if(y>=((2*x)+2))
		{
			return outs;
		}
	}
	else
	{
		if((2*y+in_frm_num-2*mdl_frm_num)>=(x+4))
		{
			return outs;
		}
	}
	
	if(x<X2)
	{
		if((2*y+2)<=x)
		{
			return outs;
		}
	}
	else
	{
		if((y+4)<=(2*x+mdl_frm_num-2*in_frm_num))
		{
			return outs;
		}
	}
	
	return ins;
}

/*	
	DTW 动态时间规整
	参数
	ftr_in	:输入特征值
	ftr_mdl	:特征模版
	返回值
	dis		:累计匹配距离
*/

u32 dtw(v_ftr_tag *ftr_in, v_ftr_tag *frt_mdl)
{
	u32 dis;
	u16 x,y;
	u16 step;
	s16 *in;
	s16 *mdl;
	u32 up,right,right_up;
	u32 min;
	
	in_frm_num=ftr_in->frm_num;
	mdl_frm_num=frt_mdl->frm_num;
	
	if((in_frm_num>(mdl_frm_num*2))||((2*in_frm_num)<mdl_frm_num))
	{
		//USART1_printf("in_frm_num=%d mdl_frm_num=%d\r\n", in_frm_num,mdl_frm_num);
		return dis_err;
	}
	else
	{
		// 计算约束平行四边形顶点值
		X1=(2*mdl_frm_num-in_frm_num)/3;
		X2=(4*in_frm_num-2*mdl_frm_num)/3;
		in=ftr_in->mfcc_dat;
		mdl=frt_mdl->mfcc_dat;

		dis=get_dis(in,mdl);
		x=1;
		y=1; 
		step=1;
		do
		{
			up=(dtw_limit(x,y+1)==ins)?get_dis(mdl+mfcc_num,in):dis_err;
			right=(dtw_limit(x+1,y)==ins)?get_dis(mdl,in+mfcc_num):dis_err;
			right_up=(dtw_limit(x+1,y+1)==ins)?get_dis(mdl+mfcc_num,in+mfcc_num):dis_err;
			
			min=right_up;
			if(min>right)
			{
				min=right;
			}
			if(min>up)
			{
				min=up;
			}
			
			dis+=min;
			
			if(min==right_up)
			{
				in+=mfcc_num;
				x++;
				mdl+=mfcc_num;
				y++;
			}
			else if(min==up)
			{
				mdl+=mfcc_num;
				y++;
			}
			else
			{
				in+=mfcc_num;
				x++;
			}
			step++;	
			//USART1_printf("x=%d y=%d\r\n",x,y);
		} 
		while((x<in_frm_num)&&(y<mdl_frm_num));
		//USART1_printf("step=%d\r\n",step);
	}
	return (dis/step); //步长归一化
}


void get_mean(s16 *frm_ftr1, s16 *frm_ftr2, s16 *mean)
{
	u8 	i;

	for(i=0;i<mfcc_num;i++)
	{
		mean[i]=(frm_ftr1[i]+frm_ftr2[i])/2;
		USART1_printf("x=%d y=%d ",frm_ftr1[i],frm_ftr2[i]);
		USART1_printf("mean=%d\r\n",mean[i]);
	}
}

/*	
	从两特征矢量获取特征模板
	参数
	ftr_in1	:输入特征值
	ftr_in2	:输入特征值
	ftr_mdl	:特征模版
	返回值
	dis		:累计匹配距离
*/

u32 get_mdl(v_ftr_tag *ftr_in1, v_ftr_tag *ftr_in2, v_ftr_tag *ftr_mdl)
{
	u32 dis;
	u16 x,y;
	u16 step;
	s16 *in1;
	s16 *in2;
	s16 *mdl;
	u32 up,right,right_up;
	u32 min;
	
	in_frm_num=ftr_in1->frm_num;
	mdl_frm_num=ftr_in2->frm_num;
	
	if((in_frm_num>(mdl_frm_num*2))||((2*in_frm_num)<mdl_frm_num))
	{
		return dis_err;
	}
	else
	{
		// 计算约束平行四边形顶点值
		X1=(2*mdl_frm_num-in_frm_num)/3;
		X2=(4*in_frm_num-2*mdl_frm_num)/3;
		in1=ftr_in1->mfcc_dat;
		in2=ftr_in2->mfcc_dat;
		mdl=ftr_mdl->mfcc_dat;

		dis=get_dis(in1,in2);
		get_mean(in1, in2, mdl);
		x=1;
		y=1; 
		step=1;
		do
		{
			up=(dtw_limit(x,y+1)==ins)?get_dis(in2+mfcc_num,in1):dis_err;
			right=(dtw_limit(x+1,y)==ins)?get_dis(in2,in1+mfcc_num):dis_err;
			right_up=(dtw_limit(x+1,y+1)==ins)?get_dis(in2+mfcc_num,in1+mfcc_num):dis_err;
			
			min=right_up;
			if(min>right)
			{
				min=right;
			}
			if(min>up)
			{
				min=up;
			}
			
			dis+=min;
			
			if(min==right_up)
			{
				in1+=mfcc_num;
				x++;
				in2+=mfcc_num;
				y++;
			}
			else if(min==up)
			{
				in2+=mfcc_num;
				y++;
			}
			else
			{
				in1+=mfcc_num;
				x++;
			}
			step++;	
			
			mdl+=mfcc_num;
			get_mean(in1, in2, mdl);
			
			USART1_printf("x=%d y=%d\r\n",x,y);
		}
		while((x<in_frm_num)&&(y<mdl_frm_num));
		USART1_printf("step=%d\r\n",step);
		ftr_mdl->frm_num=step;
	}
	return (dis/step); //步长归一化
}

