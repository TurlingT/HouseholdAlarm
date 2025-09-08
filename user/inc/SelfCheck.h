#ifndef __SELFCHECK_H__
#define __SELFCHECK_H__

#include "n32g031.h"
#include <stdint.h>


// 按钮引脚定义
#define KEY_INPUT_PORT        GPIOA
#define KEY_INPUT_PIN         GPIO_PIN_6
#define KEY_INPUT_EXTI_LINE   EXTI_LINE6
#define KEY_INPUT_PORT_SOURCE GPIOA_PORT_SOURCE
#define KEY_INPUT_PIN_SOURCE  GPIO_PIN_SOURCE6
#define KEY_INPUT_IRQn        EXTI4_15_IRQn

// 函数声明
//void Delay(uint32_t count);
void KeyInputExtiInit(GPIO_Module* GPIOx, uint16_t Pin);
void SelfCheck();

// 全局变量声明
extern volatile uint8_t button_pressed;
extern volatile uint8_t led_step;

#endif