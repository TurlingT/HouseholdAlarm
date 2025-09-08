#ifndef __CH455G_H
#define __CH455G_H

#include "n32g031.h"
#include <stdint.h>

// I²C 引脚定义
#define CH455_SCL_PIN    GPIO_PIN_15
#define CH455_SDA_PIN    GPIO_PIN_12
#define CH455_SCL_PORT   GPIOA
#define CH455_SDA_PORT   GPIOA

// CH455G 命令定义
#define CH455_SYS_CMD    0x48    // 系统参数命令
#define CH455_DIG0_CMD   0x68    // 第1位数码管命令
#define CH455_DIG1_CMD   0x6A    // 第2位数码管命令
#define CH455_DIG2_CMD   0x6C    // 第3位数码管命令
#define CH455_DIG3_CMD   0x6E    // 第4位数码管命令

extern const uint8_t SEG_CODE1[];

// 函数声明
void I2C_Self_Init(void);
void CH455G_Init(void);
void CH455G_WriteCmd(uint8_t cmd, uint8_t data);
void Display_88(void);

// 两位设置（推荐）
void CH455G_Set2(uint8_t d0, uint8_t d1);

// 兼容旧的四位接口：后两位忽略（不需要可删）
static inline void CH455G_SetDigits(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3) {
    (void)d2; (void)d3;
    CH455G_Set2(d0, d1);
}
void CH455G_RequestScan(void);
void CH455G_PollScan(void);
// 每次扫描一位（给 SysTick 用）
void CH455G_ScanOnce(void);

#endif