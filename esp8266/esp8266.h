#ifndef __ESP8266_H__
#define __ESP8266_H__

#include <stdint.h>
#include "stm32f1xx_hal.h"

#define ESP8266_RX_BUF_SIZE         128
#define ESP8266_TX_BUF_SIZE         64

#define ESP8266_EOK                 0
#define ESP8266_ERROR               1
#define ESP8266_ETIMEOUT            2
#define ESP8266_EINVAL              3

#define ESP8266_STA_MODE            1
#define ESP8266_AP_MODE             2
#define ESP8266_STA_AP_MODE         3

#define ESP8266_SINGLE_CONNECTION   0
#define ESP8266_MULTI_CONNECTION    1

#define WIFI_SSID                   "HuaweiAP-1ED0_Guest"
#define WIFI_PWD                    "gcc11111111"

#define TCP_SERVER_IP               "192.168.1.18"
#define TCP_SERVER_PORT             "8080"

void esp8266_init(uint32_t baudrate);
uint16_t esp8266_receive_data(char *recv_data);

#endif


