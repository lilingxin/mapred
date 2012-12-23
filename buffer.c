#include <assert.h>
#include <error.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "util.h"

void mapred_buf_init(mapred_buf_t* buf, uint32_t capacity)
{
    safe_malloc(buf->ptr, void, capacity);
    buf->capacity = capacity;
    buf->rp = buf->wp = buf->ptr;
}

void mapred_buf_reset(mapred_buf_t* buf)
{
    buf->rp = buf->ptr;
    buf->wp = buf->ptr;
}

void mapred_buf_free(mapred_buf_t* buf)
{
    safe_free(buf->ptr);
}

void mapred_buf_readjust(mapred_buf_t* buf) 
{
    memmove(buf->ptr, buf->rp, buf->wp - buf->rp);
    buf->wp = buf->ptr + (buf->wp - buf ->rp);
    buf->rp = buf->ptr;
}

int mapred_buf_setcapacity(mapred_buf_t* buf, uint32_t capacity) 
{
    if (capacity < buf->capacity) return 0;
    buf->ptr = realloc(buf->ptr, capacity); 
    if (!buf->ptr) {
        error(EXIT_FAILURE, errno, "realloc failed");
    }
    buf->capacity = capacity;
    return 0;
}

uint32_t mapred_buf_getcapacity(mapred_buf_t* buf) 
{
    return buf->ptr + buf->capacity - buf->wp;
}

int mapred_buf_getline(mapred_buf_t* buf, void** line, uint32_t* length)
{
    *line = buf->rp;
    *length = 0;
    
    if (mapred_buf_size(buf) == 0 && 
        mapred_buf_getcapacity(buf) == 0) {
        mapred_buf_reset(buf);
        return -1;
    } 

    if (mapred_buf_size(buf) == 0) {
        return -1;
    }

    void* bp = memchr(buf->rp, (int)'\n', buf->wp - buf->rp);
    if (bp == buf->wp || bp == NULL) {
        *length = buf->wp - buf->rp;
        return 0;
    }

    *length = ++bp - buf->rp;
    buf->rp = bp;
    return *length;
}

uint32_t mapred_buf_size(mapred_buf_t* buf)
{
    return buf->wp - buf->rp;
}
