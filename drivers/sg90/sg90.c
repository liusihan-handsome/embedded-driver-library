#include "sg90.h"


TIM_HandleTypeDef tim3_handle = {0};
// init函数
void tim3_init(void)
{
    TIM_OC_InitTypeDef pwm_config = {0};
    
    tim3_handle.Instance = TIM3;
    tim3_handle.Init.Prescaler = 7200 - 1;
    tim3_handle.Init.Period = 200 - 1;
    tim3_handle.Init.CounterMode = TIM_COUNTERMODE_UP;
    HAL_TIM_PWM_Init(&tim3_handle);
    
    pwm_config.OCMode = TIM_OCMODE_PWM1;
    pwm_config.Pulse = 100;
    pwm_config.OCPolarity = TIM_OCPOLARITY_HIGH;
    HAL_TIM_PWM_ConfigChannel(&tim3_handle, &pwm_config, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&tim3_handle, TIM_CHANNEL_1);
}


//msp函数
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM3)
    {
        GPIO_InitTypeDef gpio_initstruct;
        //打开时钟
        __HAL_RCC_GPIOA_CLK_ENABLE();                           // 使能GPIOB时钟
        __HAL_RCC_TIM3_CLK_ENABLE();
        
        //调用GPIO初始化函数
        gpio_initstruct.Pin = GPIO_PIN_6;                    // 两个LED对应的引脚
        gpio_initstruct.Mode = GPIO_MODE_AF_PP;             // 推挽输出
        gpio_initstruct.Pull = GPIO_PULLUP;                     // 上拉
        gpio_initstruct.Speed = GPIO_SPEED_FREQ_HIGH;           // 高速
        HAL_GPIO_Init(GPIOA, &gpio_initstruct);
    }
}


//修改CCR值的函数
void tim3_compare_set(uint16_t val)
{
    __HAL_TIM_SET_COMPARE(&tim3_handle, TIM_CHANNEL_1, val);
}

void sg90_init(void)
{
    tim3_init();
}

void sg90_angle_set(uint16_t angle)
{
    uint16_t CCRx = (1.0 / 9.0) * angle + 5.0;
    tim3_compare_set(CCRx);
}
