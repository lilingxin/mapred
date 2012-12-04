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
}

void mapred_buf_free(mapred_buf_t* buf)
{
    safe_free(buf->ptr);
}

void mapred_buf_readjust(mapred_buf_t* buf) 
{
    memmove(buf->ptr, buf->ptr + buf->pos, buf->size - buf->pos);
    buf->pos = 0;
    buf->size -= buf->pos;
}

int mapred_buf_setcapacity(mapred_buf_t* buf, uint32_t capacity) 
{
    if (capacity < buf->capacity) return 0;
    buf->ptr = realloc(buf->ptr, capacity); 
    if (!buf->ptr) {
        error(EXIT_FAILURE, errno, "realloc failed");
    }
    return 0;
}

uint32_t mapred_buf_getcapacity(mapred_buf_t* buf) 
{
    return buf->capacity - buf->size;
}

int mapred_buf_getline(mapred_buf_t* buf, void** line, uint32_t* length)
{
    void* ptr = buf->ptr + buf->pos;
    *line = ptr;

    void* bp = memchr(ptr, (int)'\n', buf->size - buf->pos);
    if (bp == buf->ptr + buf->size || bp == NULL) {
        *length = buf->size - buf->pos;
        buf->pos = buf->size;
        return *length;
    }

    *length = ++bp - ptr;
    buf->pos = bp - buf->ptr;
    return *length;
}

uint32_t mapred_buf_size(mapred_buf_t* buf)
{
    return buf->size - buf->pos;
}
