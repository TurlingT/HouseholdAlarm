#ifndef __LED_H__
#define __LED_H__

#endif

#include "n32g031.h"
/*Led1-PB1,Led2-PB6,Led3-PB7*/
#define PORT_GROUP  GPIOA
#define PORT_GROUP2  GPIOB
#define LEDG_PORT   PORT_GROUP2
#define LEDR_PORT   PORT_GROUP
#define LEDY_PORT   PORT_GROUP2
#define LEDY1_PORT   PORT_GROUP2
#define BZ_PORT   PORT_GROUP2

#define LEDG_PIN    GPIO_PIN_8
#define LEDR_PIN    GPIO_PIN_7
#define LEDY_PIN    GPIO_PIN_0
#define LEDY1_PIN    GPIO_PIN_1

#define BZ_PIN    GPIO_PIN_4
void Led_Init_All();
void LedInit(GPIO_Module* GPIOx, uint16_t Pin);
void BeepInit();

void LedOn(GPIO_Module *GPIOx, uint16_t Pin);
void LedOff(GPIO_Module* GPIOx, uint16_t Pin);
void LedBlink(GPIO_Module* GPIOx, uint16_t Pin);

void BeepOn(GPIO_Module *GPIOx, uint16_t Pin);
void BeepOff(GPIO_Module* GPIOx, uint16_t Pin);
void BeepBlink(GPIO_Module* GPIOx, uint16_t Pin);

void Delay(uint32_t count);
