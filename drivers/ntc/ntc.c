#include "ntc.h"
#include "stdint.h"
#include "stm32f1xx_hal.h"
#include "shell.h"
#include "math.h"

#define LOG_TAG "NTC"

#include "elog.h"

ADC_HandleTypeDef hadc2;

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */

  /** Common config
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_13;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}

int NTC_Init(void)
{
 MX_ADC2_Init();
	
 return 0;
}

uint32_t Get_ADC_Value(void)
{
 HAL_ADC_Start(&hadc2);
 HAL_ADC_PollForConversion(&hadc2,100);
	
 return HAL_ADC_GetValue(&hadc2);
}

int NTC_Get_Temprature(void)
{
 int adc_value;
 float vol;
 float res;
 float Tc;
	
 adc_value = Get_ADC_Value();
 vol = (adc_value*3.3)/4096;
	
 log_d("voltage is %f",vol);
	
 res = (10000*vol)/(3.3-vol);
	
 log_d("res is %f",res);
	
 Tc = (3950.0/(log(res/10000)+(3950.0/298.15)))-273.15;
	
 return (int)(Tc*1000);
}

void gav(int argc,char **argv)
{
 log_d("gav command is running...");
	
 log_d("adc value is %d",Get_ADC_Value());
}
ZNS_CMD_EXPORT(gav,get adc value)


void ngt(int argc,char **argv)
{
 log_d("ngt command is running ....");
 log_d("ntc temprature is %f",((float)NTC_Get_Temprature())/1000.0);
}
ZNS_CMD_EXPORT(ngt,ntc get temprature)
