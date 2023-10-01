
#include "sw_i2c.h"

// #include "stm32f1xx_hal.h"
#include "stm32f4xx_hal.h"

// 硬件引脚配置
#define SW_I2C1_SCL_PORT    GPIOB
#define SW_I2C1_SDA_PORT    GPIOB
#define SW_I2C1_SCL_PIN     GPIO_PIN_6
#define SW_I2C1_SDA_PIN     GPIO_PIN_7

// M3内核DWT32位计数器地址
#define  DWT_CYCCNT  *(volatile unsigned int *)0xE0001004


//引脚置位
static void GPIO_SetBits(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
}

//引脚复位
static void GPIO_ResetBits(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
}

//读引脚状态
static uint8_t GPIO_ReadInputDataBit(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	return (uint8_t)HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);
}

//SDA引脚切换输入模式
static void sda_in_mode(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;

    GPIO_InitStruct.Pin = SW_I2C1_SDA_PIN;
    HAL_GPIO_Init(SW_I2C1_SDA_PORT, &GPIO_InitStruct);
}

//SDA引脚切换输出模式
static void sda_out_mode(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;

    GPIO_InitStruct.Pin = SW_I2C1_SDA_PIN;
    HAL_GPIO_Init(SW_I2C1_SDA_PORT, &GPIO_InitStruct);
}

//SCL引脚切换输入模式
static void scl_in_mode(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;

    GPIO_InitStruct.Pin = SW_I2C1_SCL_PIN;
    HAL_GPIO_Init(SW_I2C1_SCL_PORT, &GPIO_InitStruct);
}

//SCL引脚切换输出模式
static void scl_out_mode(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;

    GPIO_InitStruct.Pin = SW_I2C1_SCL_PIN;
    HAL_GPIO_Init(SW_I2C1_SCL_PORT, &GPIO_InitStruct);
}

static int sw_i2c_port_initial(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    // i2c_sw SCL
    GPIO_InitStruct.Pin = SW_I2C1_SCL_PIN;
    HAL_GPIO_Init(SW_I2C1_SCL_PORT, &GPIO_InitStruct);
    // i2c_sw SDA
    GPIO_InitStruct.Pin = SW_I2C1_SDA_PIN;
    HAL_GPIO_Init(SW_I2C1_SDA_PORT, &GPIO_InitStruct);
    return 0;
}

// stm32f103最大超时时间 60s = 2^32 / 72000000
void bsp_DelayUS(uint32_t _ulDelayTime)
{
    uint32_t tCnt, tDelayCnt;
    uint32_t tStart;

    tStart = DWT_CYCCNT;                                     /* 刚进入时的计数器值 */
    tCnt = 0;
    tDelayCnt = _ulDelayTime * (SystemCoreClock / 1000000);	 /* 需要的节拍数 */

    while(tCnt < tDelayCnt)
    {
        tCnt = DWT_CYCCNT - tStart; /* 求减过程中，如果发生第一次32位计数器重新计数，依然可以正确计算 */
    }
}

static void sw_i2c_port_delay_us(uint32_t us)
{
	bsp_DelayUs(us);
}

static int sw_i2c_port_io_ctl(hal_io_opt_e opt, void *param)
{
    int ret = -1;
    switch (opt)
    {
    case HAL_IO_OPT_SET_SDA_HIGH:
        GPIO_SetBits(SW_I2C1_SDA_PORT, SW_I2C1_SDA_PIN);
        break;
    case HAL_IO_OPT_SET_SDA_LOW:
        GPIO_ResetBits(SW_I2C1_SDA_PORT, SW_I2C1_SDA_PIN);
        break;
    case HAL_IO_OPT_GET_SDA_LEVEL:
        ret = GPIO_ReadInputDataBit(SW_I2C1_SDA_PORT, SW_I2C1_SDA_PIN);
        break;
    case HAL_IO_OPT_SET_SDA_INPUT:
        sda_in_mode();
        break;
    case HAL_IO_OPT_SET_SDA_OUTPUT:
        sda_out_mode();
        break;
    case HAL_IO_OPT_SET_SCL_HIGH:
        GPIO_SetBits(SW_I2C1_SCL_PORT, SW_I2C1_SCL_PIN);
        break;
    case HAL_IO_OPT_SET_SCL_LOW:
        GPIO_ResetBits(SW_I2C1_SCL_PORT, SW_I2C1_SCL_PIN);
        break;
    case HAL_IO_OPT_GET_SCL_LEVEL:
        ret = GPIO_ReadInputDataBit(SW_I2C1_SCL_PORT, SW_I2C1_SCL_PIN);
        break;
    case HAL_IO_OPT_SET_SCL_INPUT:
        scl_in_mode();
        break;
    case HAL_IO_OPT_SET_SCL_OUTPUT:
        scl_out_mode();
        break;
    default:
        break;
    }
    return ret;
}


sw_i2c_t sw_i2c_stm32_f4 = {
    .hal_init = sw_i2c_port_initial,
    .hal_io_ctl = sw_i2c_port_io_ctl,
    .hal_delay_us = sw_i2c_port_delay_us,
    };
