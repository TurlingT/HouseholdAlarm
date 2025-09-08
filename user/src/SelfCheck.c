#include "SelfCheck.h"                    // Device header
#include "LED.h" 
#include "LCD.h"
#include <stdio.h>
#include <stdint.h>


// 全局变量定义
volatile uint8_t button_pressed = 0;
volatile uint8_t led_step = 0;

void KeyInputExtiInit(GPIO_Module* GPIOx, uint16_t Pin)
{
    GPIO_InitType GPIO_InitStructure;
    EXTI_InitType EXTI_InitStructure;
    NVIC_InitType NVIC_InitStructure;

    /* Check the parameters */
    assert_param(IS_GPIO_ALL_PERIPH(GPIOx));

    /* Enable the GPIO Clock */
    if (GPIOx == GPIOA)
    {
        RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA | RCC_APB2_PERIPH_AFIO, ENABLE);
    }
    else if (GPIOx == GPIOB)
    {
        RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB | RCC_APB2_PERIPH_AFIO, ENABLE);
    }
    else if (GPIOx == GPIOC)
    {
        RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOC | RCC_APB2_PERIPH_AFIO, ENABLE);
    }
    else if (GPIOx == GPIOF)
    {
        RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOF | RCC_APB2_PERIPH_AFIO, ENABLE);
    }
    else
    {
        return;
    }

    /*Configure the GPIO pin as input floating*/
    if (Pin <= GPIO_PIN_ALL)
    {
        GPIO_InitStruct(&GPIO_InitStructure);
        GPIO_InitStructure.Pin          = Pin;
        GPIO_InitStructure.GPIO_Pull    = GPIO_PULL_UP;
		GPIO_InitStructure.GPIO_Mode    = GPIO_MODE_INPUT;
        GPIO_InitPeripheral(GPIOx, &GPIO_InitStructure);
    }

    /*Configure key EXTI Line to key input Pin*/
    GPIO_ConfigEXTILine(KEY_INPUT_PORT_SOURCE, KEY_INPUT_PIN_SOURCE);

    /*Configure key EXTI line*/
    EXTI_InitStructure.EXTI_Line = KEY_INPUT_EXTI_LINE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitPeripheral(&EXTI_InitStructure);

    /*Set key input interrupt priority*/
    NVIC_InitStructure.NVIC_IRQChannel = KEY_INPUT_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void SelfCheck(void){
		LedOff(LEDG_PORT, LEDG_PIN);
    LedOff(LEDR_PORT, LEDR_PIN);
    LedOff(LEDY_PORT, LEDY_PIN);
    LedOff(LEDY1_PORT, LEDY1_PIN);
    while (1)
    {
        if (button_pressed)
        {
            button_pressed = 0;  // 清除标志
            // 依次点亮四个LED并鸣叫蜂鸣器
			LCD_Display_88dot();
            for (led_step = 0; led_step < 4; led_step++)
            {
                switch (led_step)
                {
                    case 0:
                        LedOn(LEDG_PORT, LEDG_PIN);
                        break;
                    case 1:
                        LedOn(LEDR_PORT, LEDR_PIN);
                        break;
                    case 2:
                        LedOn(LEDY_PORT, LEDY_PIN);
                        break;
                    case 3:
                        LedOn(LEDY1_PORT, LEDY1_PIN);
                        break;
                }
                
                BeepOn(BZ_PORT, BZ_PIN);        // 蜂鸣器鸣叫
               Delay(0x0F0000);   // 保持亮灯和鸣叫
                
                // 关闭所有LED和蜂鸣器
                LedOff(LEDG_PORT, LEDG_PIN);
                LedOff(LEDR_PORT, LEDR_PIN);
                LedOff(LEDY_PORT, LEDY_PIN);
                LedOff(LEDY1_PORT, LEDY1_PIN);
                BeepOff(BZ_PORT, BZ_PIN);
                Delay(0x0F0000);
                //Delay(100000);   // 短暂间隔
            }
            LCD_DisplayBCD2(00);
        }
    }

}

