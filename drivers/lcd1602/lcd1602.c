#include "lcd1602.h"
#include "delay.h"

// RS���Ŷ���
#define RS_GPIO_Port GPIOB
#define RS_GPIO_Pin GPIO_PIN_1
#define RS_HIGH HAL_GPIO_WritePin(RS_GPIO_Port, RS_GPIO_Pin, GPIO_PIN_SET);
#define RS_LOW HAL_GPIO_WritePin(RS_GPIO_Port, RS_GPIO_Pin, GPIO_PIN_RESET);

// RW���Ŷ���
#define RW_GPIO_Port GPIOB
#define RW_GPIO_Pin GPIO_PIN_2
#define RW_HIGH HAL_GPIO_WritePin(RW_GPIO_Port, RW_GPIO_Pin, GPIO_PIN_SET);
#define RW_LOW HAL_GPIO_WritePin(RW_GPIO_Port, RW_GPIO_Pin, GPIO_PIN_RESET);

// EN���Ŷ���
#define EN_GPIO_Port GPIOB
#define EN_GPIO_Pin GPIO_PIN_10
#define EN_HIGH HAL_GPIO_WritePin(EN_GPIO_Port, EN_GPIO_Pin, GPIO_PIN_SET);
#define EN_LOW HAL_GPIO_WritePin(EN_GPIO_Port, EN_GPIO_Pin, GPIO_PIN_RESET);

void lcd1602_init(void)
{
    // ��ʼ��GPIO
    lcd1602_gpio_init();
    // �ϵ��ʼ��
    lcd1602_start();
}

void lcd1602_gpio_init(void)
{
    GPIO_InitTypeDef gpio_initstruct;
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    gpio_initstruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7; // ����LED��Ӧ������
    gpio_initstruct.Mode = GPIO_MODE_OUTPUT_PP;                                                                                  // �������
    gpio_initstruct.Pull = GPIO_PULLUP;                                                                                          // ����
    gpio_initstruct.Speed = GPIO_SPEED_FREQ_HIGH;                                                                                // ����
    HAL_GPIO_Init(GPIOA, &gpio_initstruct);

    gpio_initstruct.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_10; // ����LED��Ӧ������
    gpio_initstruct.Mode = GPIO_MODE_OUTPUT_PP;                  // �������
    gpio_initstruct.Pull = GPIO_PULLUP;                          // ����
    gpio_initstruct.Speed = GPIO_SPEED_FREQ_HIGH;                // ����
    HAL_GPIO_Init(GPIOB, &gpio_initstruct);
}

void lcd1602_start(void)
{
    // ��1�� ��ʱ 15ms
    delay_ms(15);
    // ��2�� дָ�� 38H(�����æ�ź�)
    lcd1602_write_cmd(0x38);
    // ��3�� ��ʱ 5ms
    delay_ms(5);
    // ��4�� �Ժ�ÿ��дָ���/д���ݲ�������Ҫ���æ�ź�
    // ��5�� дָ�� 38H����ʾģʽ����
    lcd1602_write_cmd(0x38);
    // ��6�� дָ�� 08H����ʾ�ر�
    lcd1602_write_cmd(0x08);
    // ��7�� дָ�� 01H����ʾ����
    lcd1602_write_cmd(0x01);
    // ��8�� дָ�� 06H����ʾ����ƶ�����
    lcd1602_write_cmd(0x06);
    // ��9�� дָ�� 0CH����ʾ�����������
    lcd1602_write_cmd(0x0C);
}

void lcd1602_write_cmd(char cmd)
{
    RS_LOW;
    RW_LOW;
    EN_LOW;
    GPIOA->ODR = cmd;
    delay_ms(5);
    EN_HIGH;
    delay_ms(5);
    EN_LOW;
}

void lcd1602_write_data(char dataShow)
{
    RS_HIGH;
    RW_LOW;
    EN_LOW;
    GPIOA->ODR = dataShow;
    delay_ms(5);
    EN_HIGH;
    delay_ms(5);
    EN_LOW;
}

void lcd1602_show_char()
{
    // ��������ʾ��
    lcd1602_write_cmd(0x80 + 0x02);
    // ��ʾʲô��
    lcd1602_write_data('L');
}

void lcd1602_show_line(char row, char col, char *string)
{
    switch (row)
    {
    case 1:
        lcd1602_write_cmd(0x80 + col);
        while (*string)
        {
            lcd1602_write_data(*string);
            string++;
        }
        break;

    case 2:
        lcd1602_write_cmd(0x80 + 0x40 + col);
        while (*string)
        {
            lcd1602_write_data(*string);
            string++;
        }
        break;
    }
}