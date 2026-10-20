#include "project.h"

typedef struct{
    uint8_t head;
    uint8_t cmd;
    uint8_t ID;
    uint8_t rcv;
    uint16_t card;
    uint16_t egg;
    uint8_t rvc[2];
    uint8_t check;
    uint8_t tail;
    
}AJ1;
typedef struct{
    uint8_t head;
    uint8_t cmd;
    uint8_t Dat[4];
    uint8_t rec[4];
    uint8_t check;
    uint8_t tail;
}AJ2;
void cmd_analysis(uint8_t *buf)
{
   
    if(buf[1]==0xA1)
    {
         AJ1* rec=(AJ1*)buf;
        printf("[%d]out card[%d]\r\n",rec->ID,rec->card);
        printf("[%d]out egg[%d]\r\n",rec->ID,rec->egg);
    }
    if(buf[1]==0xA2)
    {
        printf("led change!\r\n");
    }
}