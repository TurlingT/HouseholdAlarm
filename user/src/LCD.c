#include "LCD.h"
#include "n32g031.h"
#include "n32g031_gpio.h"
#include "n32g031_rcc.h"

#define LCD_TSU_US 2u

/* 简单互斥/幂等 */
static volatile uint8_t s_inited = 0;
static volatile uint8_t s_busy   = 0;
static inline bool lock_bus(void){
    uint32_t p=__get_PRIMASK(); __disable_irq();
    if (s_busy){ __set_PRIMASK(p); return false; }
    s_busy=1; __set_PRIMASK(p); return true;
}
static inline void unlock_bus(void){ s_busy=0; }

/* 粗略微妙延时 */
static inline void delay_us(uint32_t us){
    while(us--){ for(volatile int i=0;i<40;i++) __NOP(); }
}

/* GPIO基础 */
static inline void SDA_out(void){
    GPIO_InitType gi; GPIO_InitStruct(&gi);
    gi.Pin=LCD_SDA_GPIO_PIN; gi.GPIO_Mode=GPIO_MODE_OUTPUT_OD;
    gi.GPIO_Pull=GPIO_PULL_UP; gi.GPIO_Speed=GPIO_SPEED_HIGH;
    GPIO_InitPeripheral(LCD_SDA_GPIO_PORT,&gi);
}
static inline void SDA_in(void){
    GPIO_InitType gi; GPIO_InitStruct(&gi);
    gi.Pin=LCD_SDA_GPIO_PIN; gi.GPIO_Mode=GPIO_MODE_INPUT;
    gi.GPIO_Pull=GPIO_PULL_UP; gi.GPIO_Speed=GPIO_SPEED_HIGH;
    GPIO_InitPeripheral(LCD_SDA_GPIO_PORT,&gi);
}
static inline void SCL_high(void){ GPIO_SetBits(LCD_SCL_GPIO_PORT, LCD_SCL_GPIO_PIN); }
static inline void SCL_low (void){ GPIO_ResetBits(LCD_SCL_GPIO_PORT, LCD_SCL_GPIO_PIN); }
static inline void SDA_high(void){ GPIO_SetBits(LCD_SDA_GPIO_PORT, LCD_SDA_GPIO_PIN); }
static inline void SDA_low (void){ GPIO_ResetBits(LCD_SDA_GPIO_PORT, LCD_SDA_GPIO_PIN); }

/* I2C */
static void i2c_start(void){
    SDA_out(); SDA_high(); SCL_high(); delay_us(LCD_TSU_US);
    SDA_low(); delay_us(LCD_TSU_US); SCL_low();
}
static void i2c_stop(void){
    SDA_out(); SDA_low(); SCL_low(); delay_us(LCD_TSU_US);
    SCL_high(); delay_us(LCD_TSU_US); SDA_high(); delay_us(LCD_TSU_US); SDA_in();
}
static void i2c_write_byte(uint8_t data){
    SDA_out();
    for(uint8_t i=0;i<8;i++){
        SCL_low(); delay_us(1);
        (data & 0x80) ? SDA_high() : SDA_low();
        data <<= 1; delay_us(1);
        SCL_high(); delay_us(LCD_TSU_US);
    }
    /* ACK位（不强制校验，防止偶发误判） */
    SCL_low(); SDA_in(); SDA_high(); delay_us(1);
    SCL_high(); delay_us(LCD_TSU_US);
    SCL_low();  SDA_out(); SDA_high();
}

/* 低层：写16位CH455命令（Start ->高字节->低字节->Stop） */
bool LCD_WriteCmd(uint16_t cmd){
    if (!s_inited) {
        RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);
        /* SCL:开漏上拉并置高 */
        GPIO_InitType gi; GPIO_InitStruct(&gi);
        gi.Pin=LCD_SCL_GPIO_PIN; gi.GPIO_Mode=GPIO_MODE_OUTPUT_OD;
        gi.GPIO_Pull=GPIO_PULL_UP; gi.GPIO_Speed=GPIO_SPEED_HIGH;
        GPIO_InitPeripheral(LCD_SCL_GPIO_PORT,&gi);
        SDA_out(); SCL_high(); SDA_high(); delay_us(10);
        s_inited = 1;
    }
    if (!lock_bus()) return false;

    i2c_start();
    i2c_write_byte( (uint8_t)(((cmd >> 7) & LCD_CH455_MASK) | LCD_I2C_ADDR8) ); /* 高字节 */
    i2c_write_byte( (uint8_t)(cmd & 0xFF) );                                     /* 低字节 */
    i2c_stop();

    unlock_bus();
    return true;
}

/* 上层API（仅两位） */
void LCD_Init(void){
    /* 上电：开显示/键盘 + 7段模式 + 8级亮度 */
    LCD_WriteCmd(CH455_7SEG_ON);
    LCD_WriteCmd(CH455_SYSON_8);
}

bool LCD_SetSystemOn(bool on){ return LCD_WriteCmd(on ? CH455_SYSON : CH455_SYSOFF); }
bool LCD_Set7SegMode(bool seg7){ return LCD_WriteCmd(seg7 ? CH455_7SEG_ON : (CH455_SYSON | 0x00)); }
bool LCD_SetBrightness(uint8_t level1to8){
    if(level1to8<1) level1to8=1; if(level1to8>8) level1to8=8;
    if(level1to8==8) return LCD_WriteCmd(CH455_SYSON_8);
    return LCD_WriteCmd(CH455_SYSON | ((uint16_t)level1to8<<4));
}

/* 一段码表 */
static const uint8_t SEG_CODE1[11] = { 
    //
    0xD7, 0xC0, 0xB5, 0xF1, 0xE2, 0x73, 0x77, 0xC1, 0xF7, 0xF3, 0x08
};
static const uint8_t BCD_TAB[16] = { 
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x58,0x5E,0x79,0x71
};

bool LCD_WriteDigit2(uint8_t pos0to1, uint8_t seg){
    uint16_t base = (pos0to1==0) ? CH455_DIG0 :
                    (pos0to1==1) ? CH455_DIG1 : 0xFFFF;
    if(base==0xFFFF) return false;
    return LCD_WriteCmd(base | seg);
}

bool LCD_DisplayRaw2(uint8_t seg_left, uint8_t seg_right){
    bool ok=true;
    ok &= LCD_WriteDigit2(0, seg_left);
    ok &= LCD_WriteDigit2(1, seg_right);
    return ok;
}

bool LCD_DisplayBCD2(uint8_t value00to99){
    if(value00to99>99) value00to99=99;
    uint8_t d0 = (value00to99/10)%10;
    uint8_t d1 = value00to99%10;

    /* ????????(???? 7 ????? BCD_TAB) */
    const uint8_t* tab = SEG_CODE1;
    bool ok=true;
    ok &= LCD_WriteDigit2(0, tab[d0]);
    ok &= LCD_WriteDigit2(1, tab[d1]);
    return ok;
}

/* 8.8.*/
bool LCD_Display_88dot(void){
    uint8_t eight = SEG_CODE1[8];
    uint8_t dp    = SEG_CODE1[10];   
    return LCD_DisplayRaw2(eight | dp, eight | dp);
}



/* ===== 增加Countdown implementation (2-digit) ===== */
static volatile uint8_t  cd_running = 0;
static volatile uint8_t  cd_seconds = 0;     /* 剩余秒数：0..99 */
static volatile uint16_t cd_ms_acc  = 0;     /* 毫秒累加到1000ms -> 1秒节拍 */
static LCD_CountdownCallback cd_cb   = 0;    /* 结束回调（可选） */
static uint8_t dp_blink = 0;                 /* 小数点闪烁标志 */

void LCD_CountdownOnFinished(LCD_CountdownCallback cb){ cd_cb = cb; }

static inline void lcd_cd_render(void){
    /* 显示 当前秒数，并让小数点1Hz闪烁作为“在计时中”的提示 */
    uint8_t tens = (cd_seconds / 10) % 10;
    uint8_t ones = cd_seconds % 10;

    /* 选择与你连线匹配的段码表：这里沿用你之前的 SEG_CODE1 */
    extern bool LCD_WriteDigit2(uint8_t pos0to1, uint8_t seg);
    extern bool LCD_DisplayRaw2(uint8_t seg_left, uint8_t seg_right);

    /* 取两位段码（和你现有 LCD_DisplayBCD2 相同逻辑） */
    extern const uint8_t SEG_CODE1[11]; /* 0..9 + dp (或你定义) */
    uint8_t L = SEG_CODE1[tens];
    uint8_t R = SEG_CODE1[ones];

//    if (cd_running && dp_blink){
//        /* 让两侧小数点闪烁；若你的连线中小数点不在bit7，请改 LCD_SEG_DP_MASK */
//        L |= LCD_SEG_DP_MASK;
//        R |= LCD_SEG_DP_MASK;
//    }

    LCD_DisplayRaw2(L, R);
}

void LCD_Tick1ms(void){
    /* 供外部SysTick调用或内部SysTick转调 */
    if (!cd_running) return;

    if (cd_ms_acc < 1000){
        cd_ms_acc++;
    }
    if (cd_ms_acc >= 1000){
        cd_ms_acc = 0;

        /* 1Hz 节拍：翻转小数点闪烁 */
        dp_blink ^= 1;

        if (cd_seconds > 0){
            cd_seconds--;
            /* 每1秒更新显示 */
            lcd_cd_render();
        }else{
            /* 计时到0：固定显示 00 并点亮小数点，触发回调一次 */
            dp_blink = 1;
            lcd_cd_render();
            cd_running = 0;
            if (cd_cb) cd_cb();
        }
    }
}

void LCD_CountdownStart(uint8_t seconds0to99){
    if (seconds0to99 > 99) seconds0to99 = 99;
    cd_seconds = seconds0to99;
    cd_ms_acc  = 0;
    dp_blink   = 1;  /* 立刻点亮一次小数点，表示“已开始” */
    cd_running = 1;
    lcd_cd_render();
}

void LCD_CountdownStop(void){
    cd_running = 0;
    /* 保持当前显示不变（需要清屏可自行调用显示函数） */
}

void LCD_CountdownReset(uint8_t seconds0to99){
    if (seconds0to99 > 99) seconds0to99 = 99;
    cd_seconds = seconds0to99;
    cd_ms_acc  = 0;
    dp_blink   = 0;
    lcd_cd_render();
}
