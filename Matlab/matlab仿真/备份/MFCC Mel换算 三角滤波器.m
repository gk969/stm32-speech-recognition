%%%%%%%%%%%%% MFCC %%%%%%%%%%%%%% MFCC %%%%%%%%%%%%%%% MFCC %%%%%%%%%%%%%% 
% Mel=2595*lg(1+f/700)
% 1000Hz以下按线性刻度 1000Hz以上按对数刻度
% 三角型滤波器中心频率 在Mel频率刻度上等间距排列 
% 步骤：
% 1.对语音信号预加重、分帧、加汉明窗处理，然后进行短时傅里叶变换，得出频谱
% 2.取频谱平方，得能量谱。并用24个Mel带通滤波器进行滤波，输出功率谱
% 3.对每个滤波器的输出值取对数，得到相应频带的对数功率谱。然后对24个对数功率进行
% 反离散余弦变换得到12个MFCC系数
% 4.对每帧的12个MFCC系数进行一阶差分，得到一共24个MFCC系数
%
% f=(10^(mel/2595)-1)*700=(e^(ln10*mel/2595)-1)*700
% (exp(log(10)*mel/2595)-1)*700
f_max=fs/2;                         % 最大频率分量 
mel_max=2595*log10(1+f_max/700);    % 最大mel频率
mel_num=20;                         % Mel三角滤波器个数。
                                    % 减小会导致MFCC系数分辨率下降
                                    % FFT点数不变，此个数增加会因为整形数舍入导致误差增大
                                    
mel_step=mel_max/(mel_num+1);  % Mel三角滤波器中心频率间隔
mel_thl=1000;                       % Mel变换线性对数临界点

fft_point=256;            % FFT点数

tri_flt=zeros(mel_num,1);           % 三角滤波器中心频率

for i=1:mel_num
    if i<(mel_thl/mel_step)             % 小于1000Hz线性尺度
        tri_flt(i)=mel_step*i;
    else                                % 大于1000Hz对数尺度
        tri_flt(i)=(exp(log(10)*(mel_step*i)/2595)-1)*700; % Mel运算逆运算
    end
end

tri_flt=int16(tri_flt/(f_max/(fft_point/2)));
plot(tri_flt);




f=(0:4000);
mel=2595*log10(1+f/700);
plot(f,mel)