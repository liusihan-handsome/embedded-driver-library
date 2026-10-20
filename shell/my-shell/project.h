#ifndef __project_h
#define __project_h
#include <stdint.h>
#include "SWM320.h"
#include <stdint.h>
#include <stddef.h>

#define ITEM_NUM(items)  ( sizeof(items) / sizeof(items[0]) )
    extern void cmd_line_work(void);
typedef struct {
    uint8_t *buf;    /* data buffer */
    uint32_t size;   /* buffer size */
    uint32_t head;   /* write index */
    uint32_t tail;   /* read index */
} loopbuf_t;
extern loopbuf_t *buf;
extern void SerialInit(uint32_t Baudrate);
extern void cmd_analysis(uint8_t *buf);
/* 开辟缓冲空间并初始化，malloc 失败返回 NULL */
extern loopbuf_t *loopbuf_create(uint32_t size);

/* 释放缓冲空间和结构体 */
extern void loopbuf_destroy(loopbuf_t *lb);

/* 使用用户提供的静态内存初始化（不 malloc） */
extern void loopbuf_init(loopbuf_t *lb, uint8_t *buf, uint32_t size);

/* 清空缓冲区 */
extern void loopbuf_reset(loopbuf_t *lb);

/* 写入 len 字节，返回实际写入字节数 */
extern uint32_t loopbuf_write(loopbuf_t *lb, const uint8_t *data, uint32_t len);

/* 读出 len 字节并推进读指针，返回实际读出字节数 */
uint32_t loopbuf_read(loopbuf_t *lb, uint8_t *data, uint32_t len);

/* 窥看 len 字节但不推进读指针，返回实际拷贝字节数 */
extern uint32_t loopbuf_peek(loopbuf_t *lb, uint8_t *data, uint32_t len);

/* 已用（有效）字节数 */
extern uint32_t loopbuf_used(loopbuf_t *lb);

/* 剩余空间字节数 */
extern uint32_t loopbuf_free(loopbuf_t *lb);

uint8_t loopbuf_is_empty(loopbuf_t *lb);
uint8_t loopbuf_is_full(loopbuf_t *lb);


#endif