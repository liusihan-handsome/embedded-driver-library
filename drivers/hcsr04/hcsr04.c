#include "hcsr04.h"
#include "delay.h"

TIM_HandleTypeDef tim2_handle = {0};

//定时器初始化函数
void tim2_init(void)
{
    tim2_handle.Instance = TIM2;
    tim2_handle.Init.Prescaler = 72 - 1;
    tim2_handle.Init.Period = 65536 - 1;
    tim2_handle.Init.CounterMode = TIM_COUNTERMODE_UP;
    tim2_handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&tim2_handle);
}

//msp函数
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM2)
    {
        __HAL_RCC_TIM2_CLK_ENABLE();
    }
}

void tim2_start(void)
{
    HAL_TIM_Base_Start(&tim2_handle);
}

void tim2_stop(void)
{
    HAL_TIM_Base_Stop(&tim2_handle);
}

uint16_t tim2_get_cnt(void)
{
    return __HAL_TIM_GetCounter(&tim2_handle);
}

void tim2_set_cnt(uint16_t val)
{
    __HAL_TIM_SetCounter(&tim2_handle, val);
}

void hcsr04_gpio_init(void)
{
    GPIO_InitTypeDef gpio_initstruct;
    //打开时钟
    TRIG_GPIO_CLK_ENABLE();
    ECHO_GPIO_CLK_ENABLE();
    
    //初始化Trig引脚
    gpio_initstruct.Pin = TRIG_PIN;
    gpio_initstruct.Mode = GPIO_MODE_OUTPUT_PP;             
    gpio_initstruct.Pull = GPIO_NOPULL;                     
    gpio_initstruct.Speed = GPIO_SPEED_FREQ_HIGH;           
    HAL_GPIO_Init(TRIG_PORT, &gpio_initstruct);
    
    gpio_initstruct.Pin = ECHO_PIN;
    gpio_initstruct.Mode = GPIO_MODE_INPUT;             
    HAL_GPIO_Init(ECHO_PORT, &gpio_initstruct);
}

void hcsr04_init(void)
{
    tim2_init();
    hcsr04_gpio_init();
}

float hcsr04_get_length(void)
{
    uint16_t total_time = 0;
    float distance = 0;

    //1. Trig，给Trig端口至少10us的高电平
    TRIG_HIGH();
    delay_us(15);
    TRIG_LOW();

    //2. Echo引脚，由低电平跳转到高电平，表示开始发送波
        //波发出去的那一下，开始启动定时器
    while(ECHO_STATUS() == GPIO_PIN_RESET);
    tim2_start();
    tim2_set_cnt(0);

    //3. Echo，由高电平跳转回低电平，表示波回来了
        //波回来的那一下，我们开始停止定时器，计算出中间经过多少时间
    while(ECHO_STATUS() == GPIO_PIN_SET);
    tim2_stop();

    //4. 计算出中间经过多少时间
    total_time = tim2_get_cnt();

    //5. 距离 = 速度（343m/s） * 时间 / 2
    distance = total_time * 0.01715;
    return distance;
}
