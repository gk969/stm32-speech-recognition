clear;
fs=8000;

noise=load('H:\语音识别\语音样本\STM32 noise.txt');
noise=(noise-2048)/2048;
sound(noise,fs);

Voice123=load('H:\语音识别\语音样本\STM32 123.txt');
Voice123=(Voice123-2048)/2048;
sound(Voice123,fs);

Voice456=load('H:\语音识别\语音样本\STM32 456.txt');
Voice456=(Voice456-2048)/2048;
sound(Voice456,fs);

subplot(311)
plot(noise)
ylabel('噪声')
grid on

subplot(312)
plot(Voice123)
ylabel('语音')
grid on

subplot(313)
plot(Voice456)
ylabel('语音')
grid on