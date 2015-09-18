% 8000Hz采样率 20ms 160dot 50Hz  10ms 80dot 100Hz  16ms 128dot  62.5Hz 
% 32ms 256dot 31.25Hz
% 预加重 6dB/倍频程 一阶高通滤波器  H(z)=1-uz^(-1) y(n)=x(n)-ux(n-1) u=0.94~0.97
% 端点检测 短时能量 短时过零率 
% 信噪比较低时 短时能量和短时绝对值效果类似
% 信噪比较高时 短时能量优于短时绝对值
clear

[y, fs, nbits] = wavread('H:\语音识别\语音样本\语音库样本 男 8KHz 16bit.wav');

data_max=32768;%语音采样分辨率 所允许的最大值 有符号

voice_end=length(y);%-104000;
voice_length=104000;
voice_start=voice_end-voice_length+1;


frame_time=20;						% 每帧时间长度 单位ms
frame_mov_rtio=0.5;					% 帧移系数
frame_len=frame_time*fs/1000;		% 帧长 
frame_mov=frame_len*frame_mov_rtio;	% 帧移，相邻帧交叠部分
frame_con=0; % 帧数

noise_val=0.02;						%白噪声大小 0~1
noise_thl=0; %噪声门限
n_thl_ratio=1;						%噪声门限系数 noise_thl=n_max_mean*n_thl_ratio
noise_time=20;						%噪声取样时间长度 单位ms
noise_len=noise_time*fs/1000;		%噪声取样点数
noise_num=10;						%噪声样本数
noise_start=1;						%噪声取样起始点
noise_max=0; %噪声极大值
n_max_mean=0; %噪声极大值平均值

valid_start=zeros(20,1); %有效语音起始点
valid_end=zeros(20,1); %有效语音结束点
valid_con=0; %有效语音段计数
cur_stus=0; %当前语音状态：0无声段  1前端过渡段  2语音段  3后端过渡段
font_duration=0; %前端过渡段超过门限值持续帧数
back_duration=0; %后端过渡段低于门限值持续帧数
v_durmin_t=50; 												%有效语音最短时间门限 ms
v_durmin_f=v_durmin_t/(frame_time*(1-frame_mov_rtio)); 		%有效语音最短帧数
s_durmax_t=125; 											%无声段最长时间门限 ms
s_durmax_f=s_durmax_t/(frame_time*(1-frame_mov_rtio)); 		%无声段最长帧数

s_thl_ratio=0.35;											% 短时幅度判决门限系数 常数
															% s_thl=frame_len*noise_thl*s_thl_ratio
s_thl=0;% 短时幅度 判决门限
z_thl_ratio=2.2/160;										% 短时过零率 判决门限系数 常数
z_thl=frame_len*z_thl_ratio/n_thl_ratio; 					% 短时过零率判决门限 直接用于端点检测

last_sig=0;%计算短时过零率 上一次过零之后采样点与噪声门限比较的结果 
           % 1 在门限带以上 -1 在门限带以下

p_voice=y(voice_start:voice_end);%纯净语音
n_voice=p_voice+noise_val*randn(voice_length,1);%加噪
%sound(n_voice,fs);
n_voice=n_voice*data_max;

% 门限值求取
% 门限值应能对背景噪声的变化具有自适应能力
% 采集若干段背景噪声 求出各自最大值 然后取平均值
% 此平均值与噪声判决门限成正比
i=noise_start;
for h=1:noise_num
	for i=i:(i+noise_len)
		if abs(n_voice(i))>noise_max
			noise_max=abs(n_voice(i));
		end
	end
	n_max_mean=n_max_mean+noise_max;
    noise_max=0;
end
n_max_mean=n_max_mean/noise_num; %取均值
noise_thl=n_max_mean*n_thl_ratio; %直接用于判断过零

s_thl=frame_len*noise_thl*s_thl_ratio; %短时幅度门限值 直接用于端点检测

% 短时幅度短时过零率求取：
% 短时幅度直接累加
% 短时过零率改进为过门限率，设置正负两个绝对值相等的门限。
% 构成门限带，穿过门限带计作过零

%端点判决：
% 1.判断语音起始点，要求能够滤除突发性噪声
%突发性噪声可以引起短时能量或过零率的数值很高，但是往往不能维持足够长的时间，
%如门窗的开关，物体的碰撞等引起的噪声，这些都可以通过设定最短时间门限来判别。
%超过两门限之一或全部，并且持续时间超过有效语音最短时间门限，
%返回最开始超过门限的时间点，将其标记为有效语音起始点。
%
% 2.判断语音结束点，要求不能丢弃连词中间短暂的有可能被噪声淹没的“寂静段”
%同时低于两门限，并且持续时间超过无声最长时间门限，
%返回最开始低于门限的时间点，将其标记为有效语音结束点。
S=zeros((voice_length-frame_len)/(frame_len-frame_mov)+1,1);% 短时幅度
Z=zeros((voice_length-frame_len)/(frame_len-frame_mov)+1,1);% 短时过零率

frame_con=0;
for h=1:frame_len-frame_mov:(voice_length-frame_len+1)
    frame=n_voice(h:h+frame_len-1); 
	frame_con=frame_con+1;
    
    for i=1:frame_len
        S(frame_con)=S(frame_con)+abs(frame(i)); %累加求取短时幅度和
    end
    
    for i=1:(frame_len-1) %求取过零率
        if frame(i)>=noise_thl
            last_sig=1;
        elseif frame(i)<(0-noise_thl)
            last_sig=-1;
        end
        
        if last_sig==-1
            if frame(i+1)>=noise_thl
                Z(frame_con)=Z(frame_con)+1;
            end
        elseif last_sig==1
            if frame(i+1)<(0-noise_thl)
                Z(frame_con)=Z(frame_con)+1;
            end
        end
    end
    last_sig=0;
    
    if S(frame_con)>s_thl || Z(frame_con)>z_thl %至少有一个参数超过其门限值
        if cur_stus==2 %如果当前是语音段
            %空操作
        elseif cur_stus==0 %如果当前是无声段
            cur_stus=1; %进入前端过渡段 
            font_duration=1; %前端过渡段持续帧数置1 第一帧
        elseif cur_stus==1; %当前是前端过渡段
            font_duration=font_duration+1;
            if font_duration>=v_durmin_f %前端过渡段帧数超过最短有效语音帧数
                cur_stus=2; %进入语音段
                valid_con=valid_con+1; %语音段计数
                valid_start(valid_con)=frame_con-v_durmin_f; %记录起始帧位置
                font_duration=0; %前端过渡段持续帧数置0
            end
        elseif cur_stus==3 %如果当前是后端过渡段
            back_duration=0;
            cur_stus=2; %记为语音段
        end
    else %两参数都在门限值以下
        if cur_stus==0 %当前是无声段
            %空操作
        elseif cur_stus==2 %当前是语音段
            cur_stus=3;%设为后端过渡段
            back_duration=1; %前端过渡段持续帧数置1 第一帧
        elseif cur_stus==3
            back_duration=back_duration+1;
            if back_duration>=s_durmax_f %后端过渡段帧数超过最长无声帧数
                cur_stus=0; %进入无声段
                valid_end(valid_con)=frame_con-s_durmax_f; %记录结束帧位置
                back_duration=0;
            end
        elseif cur_stus==1 %当前是前端过渡段 两参数都回落到门限值以下 
                           %持续时间低于语音最短时间门限 视为短时噪声
            font_duration=0;
            cur_stus=0; %记为无声段
        end
    end
end


%%%%%%%%%% 数据可视化 %%%%%%%%% 数据可视化 %%%%%%%%%% 数据可视化 %%%%%%%%%%%%
x=(1:voice_length);
vert=linspace(-0.4,0.4,100);

i=1;
valid_start=valid_start*(frame_len-frame_mov);
valid_end=valid_end*(frame_len-frame_mov);
subplot(411)
plot(x,p_voice,valid_start(1),vert,...
               valid_end(1),vert,...
               valid_start(2),vert,...
               valid_end(2),vert,...
               valid_start(3),vert,...
               valid_end(3),vert,...
               valid_start(4),vert,...
               valid_end(4),vert,...
               valid_start(5),vert,...
               valid_end(5),vert,...
               valid_start(6),vert,...
               valid_end(6),vert,...
               valid_start(7),vert,...
               valid_end(7),vert,...
               valid_start(8),vert,...
               valid_end(8),vert,...
               valid_start(9),vert,...
               valid_end(9),vert,...
               valid_start(10),vert,...
               valid_end(10),vert)
ylabel('纯净语音段')
axis([0 voice_length -inf inf])
%grid on 

temp=zeros(voice_length,1);
temp=temp+noise_thl;

subplot(412)
plot(x,n_voice,x,temp,x,-temp)
ylabel('加噪语音段')
axis([0 voice_length -inf inf])
grid on


x=(1:frame_con)*voice_length/frame_con;

temp=zeros(frame_con,1);
temp=temp+s_thl;

subplot(413)
plot(x,S, x,temp)
ylabel('短时幅度')
axis([0 voice_length -inf inf])
grid on

temp=zeros(frame_con,1);
temp=temp+z_thl;

subplot(414)
plot(x,Z, x,temp)
ylabel('短时过零率')
axis([0 voice_length -inf inf])
grid on
