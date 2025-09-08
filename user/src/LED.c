#include "LED.h" 
#include "ch455g.h"

/**
 * @brief  Configures LED GPIO.
 * @param GPIOx x can be A to G to select the GPIO port.
 * @param Pin This parameter can be GPIO_PIN_0~GPIO_PIN_15.
 */
void Led_Init_All(){
	// 初始化所有LED
    LedInit(LEDG_PORT, LEDG_PIN);
    LedInit(LEDR_PORT, LEDR_PIN);
    LedInit(LEDY_PORT, LEDY_PIN);
    LedInit(LEDY1_PORT, LEDY1_PIN);
	// 初始化蜂鸣器
    BeepInit();
    
    // 蜂鸣器鸣叫
	BeepOn(BZ_PORT, BZ_PIN);        				
	Delay(0x1BFFFF);  
    
	// 关闭所有LED和蜂鸣器
    LedOff(LEDG_PORT, LEDG_PIN);
    LedOff(LEDR_PORT, LEDR_PIN);
    LedOff(LEDY_PORT, LEDY_PIN);
    LedOff(LEDY1_PORT, LEDY1_PIN);
    BeepOff(BZ_PORT, BZ_PIN);		
}
void LedInit(GPIO_Module* GPIOx, uint16_t Pin)
{
    GPIO_InitType GPIO_InitStructure;

    /* Check the parameters */
    assert_param(IS_GPIO_ALL_PERIPH(GPIOx));

    /* Enable the GPIO Clock */
    if (GPIOx == GPIOA)
    {
        RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);
    }
    else if (GPIOx == GPIOB)
    {
        RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);
    }
    else if (GPIOx == GPIOC)
    {
        RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOC, ENABLE);
    }
    else if (GPIOx == GPIOF)
    {
        RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOF, ENABLE);
    }
    else
    {
        return;
    }

    /* Configure the GPIO pin */
    if (Pin <= GPIO_PIN_ALL)
    {
        GPIO_InitStruct(&GPIO_InitStructure);
        GPIO_InitStructure.Pin = Pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitPeripheral(GPIOx, &GPIO_InitStructure);
    }
}

// 蜂鸣器
void BeepInit(void)
{
    GPIO_InitType GPIO_InitStructure;
    
    // 使能GPIOA时钟
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);
    
    // 配置PA1为推挽输出
    GPIO_InitStruct(&GPIO_InitStructure);
    GPIO_InitStructure.Pin = BZ_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitPeripheral(BZ_PORT, &GPIO_InitStructure);
}

/**
 * @brief  Turns selected Led on.
 * @param GPIOx x can be A to G to select the GPIO port.
 * @param Pin This parameter can be GPIO_PIN_0~GPIO_PIN_15.
 */
void LedOn(GPIO_Module *GPIOx, uint16_t Pin)
{
    GPIO_ResetBits(GPIOx, Pin);
}
/**
 * @brief  Turns selected Led Off.
 * @param GPIOx x can be A to G to select the GPIO port.
 * @param Pin This parameter can be GPIO_PIN_0~GPIO_PIN_15.
 */
void LedOff(GPIO_Module* GPIOx, uint16_t Pin)
{
    GPIO_SetBits(GPIOx, Pin);
}

/**
 * @brief  Toggles the selected Led.
 * @param GPIOx x can be A to G to select the GPIO port.
 * @param Pin This parameter can be GPIO_PIN_0~GPIO_PIN_15.
 */
void LedBlink(GPIO_Module* GPIOx, uint16_t Pin)
{
    GPIO_TogglePin(GPIOx, Pin);
}

void BeepOn(GPIO_Module *GPIOx, uint16_t Pin)
{
		GPIO_SetBits(GPIOx, Pin);
}
void BeepOff(GPIO_Module *GPIOx, uint16_t Pin)
{
		GPIO_ResetBits(GPIOx, Pin);
}
void BeepBlink(GPIO_Module* GPIOx, uint16_t Pin)
{
    GPIO_TogglePin(GPIOx, Pin);
}

/**
 * @brief  Inserts a delay time.
 * @param count specifies the delay time length.
 */
void Delay(uint32_t count)
{
    //for (; count > 0; count--)
        //;
	for (volatile uint32_t i = count; i > 0; i--)
        ;
}
