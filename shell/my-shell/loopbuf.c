#include "project.h"
#include <stdlib.h>
/* 开辟空间：一次性分配 结构体 + 数据缓冲区 */
loopbuf_t *loopbuf_create(uint32_t size)
{
    loopbuf_t *lb;

    if (size < 2)
        return NULL;

    lb = (loopbuf_t *)malloc(sizeof(loopbuf_t));
    if (lb == NULL)
        return NULL;

    lb->buf = (uint8_t *)malloc(size);
    if (lb->buf == NULL) {
        free(lb);
        return NULL;
    }
    lb->size = size;
    lb->head = 0;
    lb->tail = 0;
    return lb;
}

/* 释放空间 */
void loopbuf_destroy(loopbuf_t *lb)
{
    if (lb == NULL)
        return;
    if (lb->buf != NULL) {
        free(lb->buf);
        lb->buf = NULL;
    }
    free(lb);
}

/* 用户提供静态内存，不 malloc */
void loopbuf_init(loopbuf_t *lb, uint8_t *buf, uint32_t size)
{
    if (lb == NULL || buf == NULL || size < 2)
        return;

    lb->buf = buf;
    lb->size = size;
    lb->head = 0;
    lb->tail = 0;
}

void loopbuf_reset(loopbuf_t *lb)
{
    if (lb == NULL)
        return;
    lb->head = 0;
    lb->tail = 0;
}

uint32_t loopbuf_used(loopbuf_t *lb)
{
    if (lb == NULL)
        return 0;
    return (lb->head + lb->size - lb->tail) % lb->size;
}

uint32_t loopbuf_free(loopbuf_t *lb)
{
    return lb->size - 1 - loopbuf_used(lb);
}

uint8_t loopbuf_is_empty(loopbuf_t *lb)
{
    return loopbuf_used(lb) == 0;
}

uint8_t loopbuf_is_full(loopbuf_t *lb)
{
    return loopbuf_free(lb) == 0;
}

uint32_t loopbuf_write(loopbuf_t *lb, const uint8_t *data, uint32_t len)
{
    uint32_t i;

    if (lb == NULL || data == NULL)
        return 0;

    for (i = 0; i < len; i++) {
        if (loopbuf_is_full(lb))        /* 写满则丢弃剩余数据 */
            break;
        lb->buf[lb->head] = data[i];
        lb->head = (lb->head + 1) % lb->size;
    }
    return i;
}

uint32_t loopbuf_read(loopbuf_t *lb, uint8_t *data, uint32_t len)
{
    uint32_t i;

    if (lb == NULL || data == NULL)
        return 0;

    for (i = 0; i < len; i++) {
        if (loopbuf_is_empty(lb))       /* 读空则返回已读数 */
            break;
        data[i] = lb->buf[lb->tail];
        lb->tail = (lb->tail + 1) % lb->size;
    }
    return i;
}

uint32_t loopbuf_peek(loopbuf_t *lb, uint8_t *data, uint32_t len)
{
    uint32_t i, idx;

    if (lb == NULL || data == NULL)
        return 0;

    idx = lb->tail;
    for (i = 0; i < len; i++) {
        if (i >= loopbuf_used(lb))
            break;
        data[i] = lb->buf[idx];
        idx = (idx + 1) % lb->size;
    }
    return i;
}