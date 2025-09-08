/*****************************************************************************
 * Copyright (c) 2019, Nations Technologies Inc.
 *
 * All rights reserved.
 * ****************************************************************************
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Nations' name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY NATIONS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL NATIONS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ****************************************************************************/

/**
 * @file main.c
 * @author Nations 
 * @version v1.0.0
 *
 * @copyright Copyright (c) 2019, Nations Technologies Inc. All rights reserved.
 */
#include "main.h"
#include <stdio.h>
#include <stdint.h>
#include "SelfCheck.h"  
#include "LED.h" 
#include "n32g031_rtc.h"  // 用于延时函数
#include "LCD.h"
#include "n32g031.h"
/**
 * @brief  Main program.
 */
 //测试推送git
 //测试推送git22222
 /* 统一把系统时钟更新一下，确保 SystemCoreClock 正确 */
static void Setup_SysTick_1ms(void)
{
    SystemCoreClockUpdate();                     /* 若你的系统库没有此函数可略过 */
    SysTick->LOAD  = (SystemCoreClock / 1000u) - 1u;  /* 1ms */
    SysTick->VAL   = 0u;
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk
                   | SysTick_CTRL_TICKINT_Msk
                   | SysTick_CTRL_ENABLE_Msk;    /* 开启中断与计数 */
}
 
static void on_cd_finished(void){
    /* 到时响铃/拉GPIO/置标志等…(注意尽量短小) */
    BeepOn(BZ_PORT, BZ_PIN);
    Delay(0x1B01FF);
    BeepOff(BZ_PORT, BZ_PIN);
}
 
int main(void)
 {
    SystemInit();
    LCD_Init();                          /* 只需一次；重复调也没事 */
    Led_Init_All();
    BeepInit();
		
    KeyInputExtiInit(KEY_INPUT_PORT, KEY_INPUT_PIN);
    Setup_SysTick_1ms();
    LCD_SetBrightness(8);      /* 亮度 1..8 */
    LCD_Display_88dot();
    Delay(0x1B0F00);
    LCD_DisplayBCD2(10);
    Delay(0x1B0F00);
    LCD_CountdownOnFinished(on_cd_finished);

    /* 启动 30 秒倒计时 */
    LCD_CountdownStart(3);
	 //Delay(0x1B01FF);
	 //
    while (1) {
			LedOn(LEDG_PORT, LEDG_PIN);
      // 保留LED闪烁逻辑
      //LedBlink(LEDR_PORT, LEDR_PIN);		
      //BeepBlink(BZ_PORT, BZ_PIN);
      Delay(0x1B0F00);
    }
}

