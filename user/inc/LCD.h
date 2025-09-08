#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include <stdbool.h>

/* —— ??:??? PA15=SCL?PA12=SDA,??????? —— */
#define LCD_SCL_GPIO_PORT     GPIOA
#define LCD_SDA_GPIO_PORT     GPIOA
#define LCD_SCL_GPIO_PIN      GPIO_PIN_15
#define LCD_SDA_GPIO_PIN      GPIO_PIN_12

/* —— CH455 ????(???????) —— */
#define LCD_I2C_ADDR8         0x40u   /* CH455 8-bit ?? */
#define LCD_CH455_MASK        0x3Eu

#define CH455_SYSOFF          0x0400
#define CH455_SYSON           (CH455_SYSOFF | 0x01)
#define CH455_SYSON_8         (CH455_SYSON | 0x00)   /* 8??? */
#define CH455_7SEG_ON         (CH455_SYSON | 0x08)

#define CH455_DIG0            0x1400  /* ?? 0?1 ?? */
#define CH455_DIG1            0x1500

#ifdef __cplusplus
extern "C" {
#endif

void  LCD_Init(void);                            /* ?? */
bool  LCD_WriteCmd(uint16_t cmd);                /* ???????? */

bool  LCD_SetSystemOn(bool on);
bool  LCD_Set7SegMode(bool seg7);
bool  LCD_SetBrightness(uint8_t level1to8);

/* —— ?“?????”? API —— */
bool  LCD_WriteDigit2(uint8_t pos0to1, uint8_t seg); /* pos=0/1 */
bool  LCD_DisplayRaw2(uint8_t seg_left, uint8_t seg_right);
bool  LCD_DisplayBCD2(uint8_t value00to99);

/* ??:????“8.8.”(?????8??????) */
bool  LCD_Display_88dot(void);

#ifdef __cplusplus
}
#endif

#ifndef LCD_ENABLE_INTERNAL_SYSTICK
#define LCD_ENABLE_INTERNAL_SYSTICK   0   /* 1=??????SysTick?1ms */
#endif

#ifndef LCD_SEG_DP_MASK
#define LCD_SEG_DP_MASK               0x00 
//#define LCD_SEG_DP_MASK               0x80 /* ???????,????????? */
#endif

/*  */
void LCD_CountdownStart(uint8_t seconds0to99);
void LCD_CountdownStop(void);
void LCD_CountdownReset(uint8_t seconds0to99);
uint8_t LCD_CountdownIsRunning(void);

/* ??:???????(????????????) */
typedef void (*LCD_CountdownCallback)(void);
void LCD_CountdownOnFinished(LCD_CountdownCallback cb);

/* ????????1ms??,??SysTick_Handler??????? */
void LCD_Tick1ms(void);

#endif /* LCD_H */