% 8000Hz采样率 20ms 160dot 50Hz  10ms 80dot 100Hz  16ms 128dot  62.5Hz 
% 32ms 256dot 31.25Hz

% 端点检测 短时能量 短时过零率 
% 信噪比较低时 短时能量和短时绝对值效果类似
% 信噪比较高时 短时能量优于短时绝对值
clear

%[y, fs, nbits] = wavread('H:\语音识别\语音样本\语音库样本 男 8KHz 16bit.wav');

data_max=2048;%语音采样分辨率 所允许的最大值 有符号
nbits=16;
fs=8000;
y=load('H:\语音识别\语音样本\STM32 123.txt');
y=y';

f=fft(y(9100:9260),1024);
f=abs(f);
subplot(311);
plot(f(1:512));

f=fft(y(9100:9260)-2214,1024);
f=abs(f);
subplot(312);
plot(f(1:512));

f=fft(y(9100:9260)-2214,256);
f=abs(f);
subplot(313);
plot(f(1:128));

sum=0;
for h=1:1600
	sum=sum+y(h);
end
sum=int16(sum/1600);
sum=2214;%double(sum);

voice_end=length(y);
voice_length=16000;
voice_start=voice_end-voice_length+1;

frame_time=20;						% 每帧时间长度 单位ms
frame_mov_rtio=0.5;					% 帧移系数
frame_len=frame_time*fs/1000;		% 帧长 
frame_mov=frame_len*frame_mov_rtio;	% 帧移，相邻帧交叠部分
frame_con=0; % 帧数



%%%%%%%%%%% 端点检测 %%%%%%%%%%%% 端点检测 %%%%%%%%%%%% 端点检测 %%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%短时幅度 短时过零率 背景噪声自适应自适应%%%%%%%%%%%%%%%%%%%%%

noise_val=0;						%白噪声大小 0~1
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
v_durmin_t=80; 											%有效语音最短时间门限 ms
v_durmin_f=v_durmin_t/(frame_time*(1-frame_mov_rtio)); 		%有效语音最短帧数
s_durmax_t=110; 											%无声段最长时间门限 ms
s_durmax_f=s_durmax_t/(frame_time*(1-frame_mov_rtio)); 		%无声段最长帧数

s_thl_ratio=0.37;											% 短时幅度判决门限系数 常数
															% s_thl=frame_len*noise_thl*s_thl_ratio
s_thl=0;% 短时幅度 判决门限
z_thl_ratio=2/160;										% 短时过零率 判决门限系数 常数
z_thl=frame_len*z_thl_ratio/n_thl_ratio; 					% 短时过零率判决门限 直接用于端点检测

last_sig=0;%计算短时过零率 上一次过零之后采样点与噪声门限比较的结果 
           % 1 在门限带以上 -1 在门限带以下

p_voice=y-sum;
p_voice=p_voice/data_max;%纯净语音
n_voice=y-sum+data_max*noise_val*randn(voice_length,1);%加噪
%sound(p_voice,fs);
%p_voice=p_voice*data_max;

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
        S(frame_con)=S(frame_con)+int32(abs(frame(i))); %累加求取短时幅度和
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
    %last_sig=0;
    
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
                valid_start(valid_con)=frame_con-v_durmin_f-1; %记录起始帧位置
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
                valid_end(valid_con)=frame_con-s_durmax_f-1; %记录结束帧位置
                back_duration=0;
            end
        elseif cur_stus==1 %当前是前端过渡段 两参数都回落到门限值以下 
                           %持续时间低于语音最短时间门限 视为短时噪声
            font_duration=0;
            cur_stus=0; %记为无声段
        end
    end
end

valid_start=valid_start*(frame_len-frame_mov);
valid_end=valid_end*(frame_len-frame_mov);


sound(p_voice(valid_start(1):valid_end(1)),fs);

%%%%%%%%%%%% 预加重 %%%%%%%%%%%%%% 预加重 %%%%%%%%%%%%%% 预加重 %%%%%%%%%%%%
%6dB/倍频程 一阶高通滤波器  H(z)=1-uz^(-1) y(n)=x(n)-ux(n-1) u=0.94~0.97

u=0.95;                      %预加重系数
hf_voice=p_voice(valid_start(1)+1:valid_end(1));

for i=1:(valid_end(1)-valid_start(1)-2)
    hf_voice(i)=hf_voice(i+1)-u*hf_voice(i);
end
hf_voice(i+1)=0;
hf_voice(i+2)=0;


%%%%%%%%%%%% 汉明窗 %%%%%%%%%%%% 汉明窗 %%%%%%%%%%%%%% 汉明窗 %%%%%%%%%%%%%%
%        w(n)=0.54-0.46*cos[2*pi*n/(N-1)],   0<=n<=(N-1)
%移植到MCU上时应先计算好窗函数在每一点的取值 然后存储于程序中 程序运行时直接取用

ham_top=10000;
hamm=zeros(frame_len,1);
for i=1:frame_len
    hamm(i)=0.54-0.46*cos(2*pi*(i-1)/(frame_len-1));
end
hamm=int32(hamm*ham_top);
hamm=double(hamm);
plot(hamm);
csvwrite('hamm.c',hamm');

%%%%%%%%%%%%% MFCC %%%%%%%%%%%%%% MFCC %%%%%%%%%%%%%%% MFCC %%%%%%%%%%%%%% 
% Mel=2595*lg(1+f/700)
% 1000Hz以下按线性刻度 1000Hz以上按对数刻度
% 三角型滤波器中心频率 在Mel频率刻度上等间距排列 
% 步骤：
% 1.对语音信号预加重、分帧、加汉明窗处理，然后进行短时傅里叶变换，得出频谱
% 2.取频谱平方，得能量谱。并用M个Mel带通滤波器进行滤波，输出Mel功率谱
% 3.对每个滤波器的输出值取对数，得到相应频带的对数功率谱。然后对M个对数功率进行
% 反离散余弦变换得到12个MFCC系数
% 4.对每帧的12个MFCC系数进行一阶差分，得到一共24个MFCC系数
%
% f=(10^(mel/2595)-1)*700=(e^(ln10*mel/2595)-1)*700
% (exp(log(10)*mel/2595)-1)*700
f_max=fs/2;                         % 最大频率分量 
mel_max=2595*log10(1+f_max/700);    % 最大mel频率
tri_num=24;                         % Mel三角滤波器个数。必须是偶数
                                    % 减小会导致MFCC系数分辨率下降
                                    % FFT点数不变，此数增加会因为整形数舍入导致误差增大
mfcc_num=12;                        % MFCC阶数
t_max=2000;                         % 语音最长时间 单位ms
frm_max=t_max/(frame_time*(1-frame_mov_rtio));% 最大帧数
mfcc=zeros(frm_max,mfcc_num);       % mfcc系数
pow_spct=zeros(frm_max,tri_num);    % 经三角滤波器输出的对数功率谱

mel_step=mel_max/(tri_num+1);  		% Mel三角滤波器中心频率间隔
mel_thl=1000;                       % Mel变换线性对数临界点

fft_point=1024;            		% FFT点数

tri_cen=zeros(tri_num,1);		% 三角滤波器中心频率
tri_top=1000;					% 三角滤波器顶点值 过大会导致溢出 过小会导致三角滤波器输出精度下降
tri_odd=zeros(fft_point/2,1);	% 奇数三角滤波器首尾相连的折线
tri_even=zeros(fft_point/2,1);	% 偶数三角滤波器首尾相连的折线

for i=1:tri_num
    if i<(mel_thl/mel_step)             % 小于1000Hz线性尺度
        tri_cen(i)=mel_step*i;
    else                                % 大于1000Hz对数尺度
        tri_cen(i)=(exp(log(10)*(mel_step*i)/2595)-1)*700; % Mel运算逆运算
    end
end

tri_cen=int32(tri_cen/(f_max/(fft_point/2)));
%plot(tri_cen);
csvwrite('tri_cen.c',tri_cen');
tri_cen=double(tri_cen);

for j=1:tri_cen(1)
	tri_odd(j)=tri_top*j/tri_cen(1);
end
for j=tri_cen(1)+1:tri_cen(2)
	tri_odd(j)=tri_top*(tri_cen(2)-j)/(tri_cen(2)-tri_cen(1));
end
for h=3:2:tri_num
	for j=tri_cen(h-1):tri_cen(h)
		tri_odd(j)=tri_top*(j-tri_cen(h-1))/(tri_cen(h)-tri_cen(h-1));
	end
	for j=(tri_cen(h)+1):tri_cen(h+1)
		tri_odd(j)=tri_top*(tri_cen(h+1)-j)/(tri_cen(h+1)-tri_cen(h));
	end
end


for h=2:2:tri_num-2
	for j=tri_cen(h-1):tri_cen(h)
		tri_even(j)=tri_top*(j-tri_cen(h-1))/(tri_cen(h)-tri_cen(h-1));
	end
	for j=(tri_cen(h)+1):tri_cen(h+1)
		tri_even(j)=tri_top*(tri_cen(h+1)-j)/(tri_cen(h+1)-tri_cen(h));
	end
end
for j=tri_cen(tri_num-1):tri_cen(tri_num)
    tri_even(j)=tri_top*(j-tri_cen(tri_num-1))/(tri_cen(tri_num)-tri_cen(tri_num-1));
end
for j=(tri_cen(tri_num)+1):fft_point/2
    tri_even(j)=tri_top*(fft_point/2-j)/(fft_point/2-tri_cen(tri_num));
end


tri_even=int32(tri_even);
tri_odd=int32(tri_odd);

csvwrite('tri_even.c',tri_even');
csvwrite('tri_odd.c',tri_odd');

tri_even=double(tri_even);
tri_odd=double(tri_odd);

x=1:fft_point/2;
plot(x,tri_even,x,tri_odd);


hf_voice=hf_voice*data_max; %还原到-2048~2047

valid_frm_num=(valid_end(1)-valid_start(1)-frame_len)/(frame_len-frame_mov)+1;	% 有效语音帧数
valid_frm=zeros(frame_len,1);

for i=0:(valid_frm_num-1)  % 逐帧计算有效语音段的MFCC系数
	for h=1:frame_len
		valid_frm(h)=hf_voice((i*(frame_len-frame_mov)+h));   % 取帧值
		valid_frm(h)=valid_frm(h)*hamm(h)/ham_top;                              % 加汉明窗
	end

    frq_spct=fft(valid_frm,fft_point);			% 取频谱
	frq_spct=abs(frq_spct);	
	
	for h=1:fft_point/2
		frq_spct(h)=frq_spct(h)*frq_spct(h);	% 平方求能量谱
    end
	
    % 加三角滤波器
	pow_spct(i+1,1)=0;
	for j=1:tri_cen(2)
		pow_spct(i+1,1)=pow_spct(i+1,1)+frq_spct(j)*tri_odd(j);
	end
	for h=3:2:tri_num
		pow_spct(i+1,h)=0;
		for j=tri_cen(h-1):tri_cen(h+1)
			pow_spct(i+1,h)=pow_spct(i+1,h)+frq_spct(j)*tri_odd(j);
		end
	end
	for h=2:2:tri_num-2
		pow_spct(i+1,h)=0;
		for j=tri_cen(h-1):tri_cen(h+1)
			pow_spct(i+1,h)=pow_spct(i+1,h)+frq_spct(j)*tri_even(j);
		end
	end
	pow_spct(i+1,tri_num)=0;
	for j=tri_cen(tri_num-1):fft_point/2
		pow_spct(i+1,tri_num)=pow_spct(i+1,tri_num)+frq_spct(j)*tri_even(j);
    end
    
    % 取对数
	for h=1:tri_num
        pow_spct(i+1,h)=log(pow_spct(i+1,h));
    end
    
    % 反离散余弦变换
    for h=1:mfcc_num
        for j=1:tri_num
            mfcc(i+1,h)=mfcc(i+1,h)+cos(h*pi*(j-0.5)/tri_num)*pow_spct(i+1,j);
        end
    end
end

%csvwrite('mfcc_tem.c',mfcc(1:valid_frm_num,1:mfcc_num));

%%%%%%% 读取模板 DTW特征匹配
voice_up=load('mfcc_tem.c');

f=(0:4000);
mel=2595*log10(1+f/700);
plot(f,mel)

%%%%%%%%%% 数据可视化 %%%%%%%%% 数据可视化 %%%%%%%%%% 数据可视化 %%%%%%%%%%%%
x=(1:voice_length);
vert=linspace(min(p_voice)*1.1,max(p_voice)*1.1,100);

subplot(411)
plot(x,p_voice,valid_start(1),vert,valid_end(1),vert)
ylabel('纯净语音段')
axis([0 voice_length -inf inf])
grid on 

temp=zeros(voice_length,1);
temp=temp+noise_thl;

subplot(412)
plot(x,n_voice,x,temp,x,-temp)
ylabel('加噪语音段')
axis([0 voice_length -inf inf])
grid on


x=(0:(frame_con-1))*(frame_len-frame_mov);

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
