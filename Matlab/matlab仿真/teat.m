hp=zeros(160,1);
hm=zeros(160,1);
vv=p_voice(4400:4560)*2048;
for i=1:160
    hp(i)=(vv(i+1)-vv(i)*0.95);
    hm(i)=hp(i)*hamm(i)/1000;
end

f=fft(hm,1024);
f=abs(f);
plot(f(1:512));

for h=1:512
    f(h)=f(h)*f(h);	% 平方求能量谱
end



dct_arg=zeros(12,24);
for h=1:12
    for j=1:24
        dct_arg(h,j)=cos(h*pi*(j-0.5)/24);
    end
end

dct_arg=int32(dct_arg*100);
csvwrite('dct_arg.c',dct_arg);

