#include "wifidev.h"
#include "shell.h"
#include "elog.h"

static int WiFiDevInit(void);
static int WiFiWrite(unsigned char *buf, unsigned int length, uint32_t Timeout);
static int WiFiRead(uint32_t Timeout);

TaskHandle_t esp32_wifi_online_task_handler;


static char mqtt_pub_buf[256];

char * (p_atcmds)[ATCMD_NUM] = {                   \
	                              "AT+RST\r\n",          \
	                              "AT\r\n",      \
	                              "ATE0\r\n",        \
	                              "AT+CWMODE=1\r\n", \
	                              "AT+CWJAP=\"" WIFI_SSID "\",\"" WIFI_PASSWORD "\"\r\n", \
	                              "AT+CIPSNTPCFG=1,8,\"cn.ntp.org.cn\"\r\n", \
	                              "AT+MQTTUSERCFG=0,1,\"NULL\",\"" MQTT_USERNAME "\",\"" MQTT_PASSWORD "\",0,0,\"\"\r\n",  \
	                              "AT+MQTTCLIENTID=0,\"" MQTT_CLIENT_ID "\"\r\n",  \
	                              "AT+MQTTCONN=0,\"" MQTT_IP "\"," MQTT_PORT ",1\r\n"
                               };

void esp32_wifi_online_task_entry(void *p)
{
	WiFiDevInit();
	
	while(1)
	{
	 vTaskDelay(1000);

	 snprintf(mqtt_pub_buf, sizeof(mqtt_pub_buf),
				 "AT+MQTTPUB=0," \
	       MQTT_TOPIC "," \
	       "\"{\\\"params\\\":{\\\"voltage\\\":%.2f\\,\\\"brightness\\\":%d\\,\\\"temprature\\\":%.2f\\,\\\"humidity\\\":%d}}\",1,0\r\n",
				 (float)(Get_Adj_Voltage()/1000.0), (ap3216c_read_als()), (float)(NTC_Get_Temprature()/1000.0),SHT30_Get_Data()/10);
	 
	 log_i("mqtt_str:%s",mqtt_pub_buf);
	 
	 WiFiWrite((uint8_t *)mqtt_pub_buf, strlen(mqtt_pub_buf), 5000);		
	}
	
}

int WiFiDevInit(void)
{
 int i=0;
 int cnt = 0;
	
 log_i("try to connect wifi ....");
	
 for(i=0;i<(ATCMD_NUM-1);)
 {
	log_d("atcmd_send:%s",p_atcmds[i]);  
	 
  WiFiWrite(p_atcmds[i],strlen(p_atcmds[i]), HAL_MAX_DELAY);
	 
  if(WiFiRead(20000)>=0)
	{
   log_d("suc! @ i=%d %d times",i,cnt);
		
	 log_i("atack:%s",at_rev_buf);	
	 
	 i++;
		
	 cnt = 0;
	 memset(at_rev_buf,0,AT_REV_BUFSIZE);
	 at_rev_cnt = 0;
		
	}
	else
	{
	 printf(">");
	 cnt ++;
		
	 if(cnt > 5) 
	 {
		i = 0;
		cnt = 0;
		 
		log_w("atcmd send overtime!");
	 }
	}
	
	vTaskDelay(1000);
 }
 
 WiFiWrite(p_atcmds[i],strlen(p_atcmds[i]), HAL_MAX_DELAY);

 return 0;
}


int WiFiInit(void)
{
	/* 创建信号量 */
	uart3_got_at_ack_sem = xSemaphoreCreateBinary();
	if(NULL != uart3_got_at_ack_sem){
			printf("uart3_got_at_ack_sem creat succeed\r\n");
	}
	/* 创建任务 */
	xTaskCreate((TaskFunction_t )esp32_wifi_online_task_entry,"esp32_wifi_online_task_entry",128,NULL, 2,(TaskHandle_t*  )&esp32_wifi_online_task_handler);
	HAL_UART_Receive_IT(&huart3, (uint8_t *)g_rx_buffer, RXBUFFERSIZE); 
	return 0;
}

int WiFiWrite(unsigned char *buf, unsigned int length, uint32_t Timeout)
{
	memset(at_rev_buf,0,AT_REV_BUFSIZE);
	at_rev_cnt = 0;
	
	return HAL_UART_Transmit(&huart3, buf, length, Timeout);
}

int WiFiRead(uint32_t Timeout)
{
	if(pdFALSE == xSemaphoreTake(uart3_got_at_ack_sem,Timeout)){
		return -1;
	}

	return at_rev_cnt;
}


