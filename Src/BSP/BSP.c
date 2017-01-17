#include "includes.h"

#define DEBUG
#ifdef DEBUG
#define DBP(fmt,arg...)  USART1_printf(fmt,##arg)
#define DBPLN(fmt,arg...)  USART1_printf_line(fmt,##arg)
#define DBPH(src, len)  USART1_print_hex(src, len)
#else
#define DBP(fmt,arg...)
#define DBPLN(fmt,arg...)
#define DBPH(src, len)
#endif

void RCC_Configuration(void) {
    /* Setup the microcontroller system. Initialize the Embedded Flash Interface,
     initialize the PLL and update the SystemFrequency variable. */
    //SystemInit();


    /* Enable USART1, GPIOA, GPIOx and AFIO …… clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_SPI1 |
                           RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA |
                           RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |
                           RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOG |
                           RCC_APB2Periph_AFIO  | RCC_APB2Periph_ADC1, ENABLE);

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
}

/**
  * @brief  Configures the different GPIO ports.
  * @param  None
  * @retval : None
  */
void GPIO_Configuration(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    /*  For LED1 LED2   Use */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* For LED3, LED4    use */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_6;
    GPIO_Init(GPIOD, &GPIO_InitStructure);


    /* For key use*/
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
}

void testFontStorage(){
    const char *cnStr="欢迎使用孤立词语音识别测试系统";
    u8 i;
    
    for(i=0; i<15; i++){
        printGB2312Font24Dot(cnStr+i*2);
    }
}

void BSP_Init(void) {
    RCC_Configuration();
    
    GPIO_Configuration();

    USART1_configuration();

    DBPLN("start");

    spiFlashInit();
    
    touchPanelInit();
    LCD_Init();

    ADC_DMA_Init();
}

uint32_t  BSP_CPU_ClkFreq(void) {
    RCC_ClocksTypeDef  rcc_clocks;

    RCC_GetClocksFreq(&rcc_clocks);

    return ((uint32_t)rcc_clocks.HCLK_Frequency);
}

uint32_t  OS_CPU_SysTickClkFreq(void) {
    uint32_t  freq;
    freq = BSP_CPU_ClkFreq();
    return (freq);
}
