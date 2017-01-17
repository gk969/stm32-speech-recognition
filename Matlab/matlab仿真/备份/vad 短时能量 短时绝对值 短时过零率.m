% 8000Hz采样率 20ms 160dot 50Hz  10ms 80dot 100Hz  16ms 128dot  62.5Hz 
% 32ms 256dot 31.25Hz
% 预加重 6dB/倍频程 一阶高通滤波器  H(z)=1-uz^(-1) y(n)=x(n)-ux(n-1) u=0.94~0.97
% 端点检测 短时能量 短时过零率 
% 信噪比较低时 短时能量和短时绝对值效果类似
% 信噪比较高时 短时能量优于短时绝对值
[y, fs, nbits] = wavread('F:\语音识别\语音样本\01234567989十百千万.wav');

len=3200;
v_start=19100;
voice=y(v_start:v_start+len-1)+0.02*randn(len,1);
%sound(voice,fs)
n=80;% 帧长 10ms
m=0;% 帧移，相邻帧交叠部分
E=zeros((length(voice)-n)/(n-m)+1,1);% 短时能量
S=zeros((length(voice)-n)/(n-m)+1,1);% 短时平均幅度
Z=zeros((length(voice)-n)/(n-m)+1,1);% 短时过零率
c=0;

for h=1:n-m:(length(voice)-n+1)
    samp=voice(h:h+n-1); 
	c=c+1;
    for i=1:n
        E(c)=E(c)+samp(i)^2;
        S(c)=S(c)+abs(samp(i));
    end
    for i=1:(n-1)
        if samp(i)>=0
            if samp(i+1)<0
                Z(c)=Z(c)+1;
            end
        else
            if samp(i+1)>=0
                Z(c)=Z(c)+1;
            end
        end
    end
end

subplot(411)
plot(voice)
title('语音段')
axis([0 length(voice) -0.5 0.5])
grid on

x=(1:length(E))*len/length(E);

subplot(412)
plot(x,E)
title('短时能量')
axis([0 length(voice) 0 3])
grid on

subplot(413)
plot(x,S)
title('短时绝对值')
axis([0 length(voice) 0 15])
grid on

subplot(414)
plot(x,Z)
title('短时过零率')
axis([0 length(voice) 0 50])
grid on
