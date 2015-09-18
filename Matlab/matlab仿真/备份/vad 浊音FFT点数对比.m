% 8000Hz采样率 20ms 160dot 50Hz  10ms 80dot 100Hz  16ms 128dot  62.5Hz 
% 32ms 256dot 31.25Hz
% 预加重 6dB/倍频程 一阶高通滤波器  H(z)=1-uz^(-1) y(n)=x(n)-ux(n-1) u=0.94~0.97
% 端点检测 短时能量 短时过零率 
[y, fs, nbits] = wavread('H:\语音识别\语音样本\01234567989十百千万.wav');

samp=y(41500:44600);
subplot(2,2,1)
plot(samp)
title('语音段')

n=80;
voice_samp=samp(1500:1579); % 10ms sample
subplot(2,2,2)
plot(voice_samp)
title('浊音')

sf=fft(voice_samp,128);
af=abs(sf);
af=af(1:64);
f=(0:63)*4000/64;
subplot(2,2,4)
plot(f,af);
title('频谱')

sf=fft(voice_samp,256);
af=abs(sf);
af=af(1:128);
f=(0:127)*4000/128;
subplot(2,2,3)
plot(f,af);
title('频谱')