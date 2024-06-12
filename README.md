# 基于STM32的孤立词语音识别

本设计研究孤立词语音识别系统及其在STM32嵌入式平台上的实现。识别流程是：预滤波、ADC、分帧、端点检测、预加重、加窗、特征提取、特征匹配。端点检测(VAD)采用短时幅度和短时过零率相结合。检测出有效语音后，根据人耳听觉感知特性,计算每帧语音的Mel频率倒谱系数(MFCC)。然后采用动态时间弯折(DTW)算法与特征模板相匹配,最终输出识别结果。先用Matlab对上述算法进行仿真，经多次试验得出算法中所需各系数的最优值。然后将算法移植到STM32嵌入式平台，移植过程中根据嵌入式平台存储空间相对较小、计算能力也相对较弱的实际情况，对算法进行优化。最终设计并制作出基于STM32的孤立词语音识别系统。

#### 详细介绍：
[基于STM32的孤立词语音识别.pdf](https://github.com/gk969/stm32-speech-recognition/blob/master/%E5%9F%BA%E4%BA%8ESTM32%E7%9A%84%E5%AD%A4%E7%AB%8B%E8%AF%8D%E8%AF%AD%E9%9F%B3%E8%AF%86%E5%88%AB.pdf)

[http://gk969.com/stm32-speech-recognition/](http://gk969.com/stm32-speech-recognition/)
