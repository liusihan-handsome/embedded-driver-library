#ifndef _WIFIDEV_H_
#define _WIFIDEV_H_

#include "cmsis_os.h"

int WiFiInit(void);

#define WIFI_SSID "myhome3"
#define WIFI_PASSWORD  "llllll123"

#define MQTT_USERNAME  "IOT_DEVICE1&k1kbctZHSSg"
#define MQTT_PASSWORD  "8b2c440f327926987a336ab3896a8c58316f549549e9bf4e5ee637f131e2a3f4"

#define MQTT_CLIENT_ID "k1kbctZHSSg.IOT_DEVICE1|securemode=2\\,signmethod=hmacsha256\\,timestamp=1747655101592|"
#define MQTT_IP 	   "iot-06z00cqqz5twa66.mqtt.iothub.aliyuncs.com"
#define MQTT_PORT	   "1883"

#define MQTT_TOPIC   "\"/sys/k1kbctZHSSg/IOT_DEVICE1/thing/event/property/post\""
                     
                     

#define ATCMD_NUM (9)

#endif /* _WIFIDEV_H_ */
