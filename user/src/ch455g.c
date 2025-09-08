#include "ch455g.h"
#include "n32g031_gpio.h"
#include "n32g031_i2c.h"
#include "n32g031_rcc.h"
static volatile uint8_t g_need_scan = 0;  // 新增：刷屏标志
const uint8_t SEG_CODE1[] = {
    0xD7, 0xC0, 0xB5, 0xF1, 0xE2, 0x73, 0x77, 0xC1, 0xF7, 0xF3, 0x37
};
/**0x7F  6.
0xFF		8.
0x3F    E.

*/
/**
// ========= 两位显示缓冲与扫描 =========
static volatile uint8_t g_digits2[2] = {10, 10}; // 10 作为“空白/自定义”，按你的段码表调整
static volatile uint8_t g_scan_idx2 = 0;

// 安全取段码（越界/空白返回 0x00）
static inline uint8_t seg_of(uint8_t d){
    extern const uint8_t SEG_CODE1[]; // 你已有的段码表
    // 若 10 不是空白，请把这里的返回 0x00 逻辑保留，SEG_CODE1[10] 无需改
    if (d < 10) return SEG_CODE1[d];
    return 0x00; // 空白
}

// 对外：设置两位内容
void CH455G_Set2(uint8_t d0, uint8_t d1){
    g_digits2[0] = d0;
    g_digits2[1] = d1;
}

// 对外：每次刷新一位（放在 SysTick_Handler 里调用）
void CH455G_ScanOnce(void){
    switch (g_scan_idx2) {
        case 0: // 第 1 位
            CH455G_WriteCmd(CH455_DIG0_CMD, seg_of(g_digits2[0]));
            break;
        case 1: // 第 2 位
            CH455G_WriteCmd(CH455_DIG1_CMD, seg_of(g_digits2[1]));
            break;
        default:
            break;
    }
    g_scan_idx2 ^= 1; // 0/1 交替
}

void CH455G_RequestScan(void){            // 新增：只置标志
    g_need_scan = 1;
}

void CH455G_PollScan(void){               // 新增：主循环里调用
    if (g_need_scan){
        g_need_scan = 0;
        CH455G_ScanOnce();                // 真正的 I2C 刷新放到这里
    }
}
*/

// 简单微秒延时（用于I²C时序）
void Delay_us(uint32_t us) {
    us *= 12; // 根据主频调整（N32G031默认72MHz）
    while (us--) __NOP();
}

// I²C初始化（PB6=SCL, PB7=SDA）
void I2C_Self_Init(void) {
    GPIO_InitType gpio_init;
		GPIO_InitStruct(&gpio_init);
		//增加+++++++
//		RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_I2C1, ENABLE);
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOA, ENABLE);
    
    // 配置SCL和SDA为开漏输出（支持I²C）
    gpio_init.Pin = CH455_SCL_PIN | CH455_SDA_PIN;
    gpio_init.GPIO_Mode = GPIO_MODE_OUTPUT_OD;	// 开漏输出GPIO_MODE_OUTPUT_OD || GPIO_MODE_AF_OD
		gpio_init.GPIO_Pull = GPIO_PULL_UP;         // 启用内部上拉GPIO_PULL_UP   GPIO_NO_PULL
    gpio_init.GPIO_Speed = GPIO_SPEED_HIGH;
	//#define GPIO_AF_I2C GPIO_AF6_I2C1
		 //gpio_init.GPIO_Alternate = GPIO_AF_I2C1;
		GPIO_InitPeripheral(CH455_SCL_PORT, &gpio_init);
	/*
	 GPIO_InitStruct(&GPIO_InitStructure);
        GPIO_InitStructure.Pin = CH455_SCL_PIN | CH455_SDA_PIN;
        GPIO_InitStructure.GPIO_Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitPeripheral(GPIOx, &GPIO_InitStructure);
    GPIO_Init(GPIOB, &gpio_init);
    */
    // 初始拉高总线
		//GPIO_ResetBits(CH455_SCL_PORT, CH455_SCL_PIN);
    //GPIO_ResetBits(CH455_SDA_PORT, CH455_SDA_PIN);
    GPIO_SetBits(CH455_SCL_PORT, CH455_SCL_PIN);
    GPIO_SetBits(CH455_SDA_PORT, CH455_SDA_PIN);
		Delay_us(10);  // 确保稳定
}


// 发送I²C起始信号
void I2C_Start(void) {
    GPIO_SetBits(CH455_SDA_PORT, CH455_SDA_PIN);
    GPIO_SetBits(CH455_SCL_PORT, CH455_SCL_PIN);
    Delay_us(5);
    GPIO_ResetBits(CH455_SDA_PORT, CH455_SDA_PIN); // SDA下降沿
    Delay_us(5);
    GPIO_ResetBits(CH455_SCL_PORT, CH455_SCL_PIN);
}

// 发送I²C停止信号
void I2C_Stop(void) {
    GPIO_ResetBits(CH455_SDA_PORT, CH455_SDA_PIN);
    GPIO_ResetBits(CH455_SCL_PORT, CH455_SCL_PIN);
    Delay_us(5);
    GPIO_SetBits(CH455_SCL_PORT, CH455_SCL_PIN);
    Delay_us(5);
    GPIO_SetBits(CH455_SDA_PORT, CH455_SDA_PIN); // SDA上升沿
		Delay_us(5);  // 确保停止信号完成
}

// 检测ACK响应（返回0表示成功）
uint8_t I2C_CheckAck(void) {
    uint8_t ack = 0;
    GPIO_SetBits(CH455_SDA_PORT, CH455_SDA_PIN);  // 释放SDA
    Delay_us(2);
    GPIO_SetBits(CH455_SCL_PORT, CH455_SCL_PIN);  // SCL拉高
    Delay_us(5);
    
    // 读取SDA状态（0=ACK, 1=NACK）
    if (GPIO_ReadInputDataBit(CH455_SDA_PORT, CH455_SDA_PIN)) 
        ack = 1;  // NACK
    
    GPIO_ResetBits(CH455_SCL_PORT, CH455_SCL_PIN); // SCL拉低
    return ack;
}

// 发送单字节（含ACK检测）
void I2C_WriteByte(uint8_t data) {
    for (uint8_t i = 0; i < 8; i++) {
        GPIO_ResetBits(CH455_SCL_PORT, CH455_SCL_PIN);	// SCL先拉低
				Delay_us(2);
				// 准备数据位（SCL低电平时变化）
        (data & 0x80) ? GPIO_SetBits(CH455_SDA_PORT, CH455_SDA_PIN) 
                       : GPIO_ResetBits(CH455_SDA_PORT, CH455_SDA_PIN);
        data <<= 1;
        Delay_us(2);
        GPIO_SetBits(CH455_SCL_PORT, CH455_SCL_PIN); // SCL上升沿采样数据
        Delay_us(2);
    }
    // 检测ACK（CH455G在第9个时钟拉低SDA）
    GPIO_ResetBits(CH455_SCL_PORT, CH455_SCL_PIN);
		//while(! I2C_CheckAck()){
		//Delay_us(2);
		//}
		
		
		
    GPIO_SetBits(CH455_SDA_PORT, CH455_SDA_PIN); // 释放SDA
    Delay_us(5);
    GPIO_SetBits(CH455_SCL_PORT, CH455_SCL_PIN);
    Delay_us(5);
    GPIO_ResetBits(CH455_SCL_PORT, CH455_SCL_PIN);
}

// 向CH455G发送命令（2字节格式）
void CH455G_WriteCmd(uint8_t cmd, uint8_t data) {
    I2C_Start();
    I2C_WriteByte(0x24 << 1);   // CH455G I²C地址（0x24 + 写方向）
    I2C_WriteByte(cmd);         // 命令字节
    I2C_WriteByte(data);        // 数据字节
    I2C_Stop();
}

// 初始化CH455G（开启显示+最大亮度）
void CH455G_Init(void) {
    I2C_Self_Init();
    // 发送系统命令：开启显示 + 亮度8级
    CH455G_WriteCmd(CH455_SYS_CMD, 0x01 | 0x70); // 0x01=使能显示, 0x70=亮度最大
		Delay_us(5000);	//等待芯片稳定
}

// 显示"8.8."（第1位和第2位显示8，第3位显示小数点）
void Display_88(void) {
	    // 第0位：数字8（带小数点）
//    CH455G_WriteCmd(CH455_DIG0_CMD,0xFF); s
    // 第1位：数字8（带小数点）
//    CH455G_WriteCmd(CH455_DIG1_CMD,0xFF);
//    // 第2位：关闭显示（避免残留）	
//    CH455G_WriteCmd(CH455_DIG2_CMD, 0x00);  

    // 第1位：数字8（段码0x7F）
    CH455G_WriteCmd(CH455_DIG0_CMD, SEG_CODE1[8]);
    // 第2位：数字8（段码0x7F）
    CH455G_WriteCmd(CH455_DIG1_CMD, SEG_CODE1[8]);
    // 第3位：小数点（段码0x80）
    //CH455G_WriteCmd(CH455_DIG2_CMD, SEG_CODE1[10]); 
	

}