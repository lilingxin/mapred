#include <unistd.h>
#include <stdio.h>
#include <error.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "buf.h"
#include "util.h"

int init_buf(struct buf_t* b, int alloc)
{
    b->capacity = alloc;
    b->bytes = 0;
    b->ptr = (char*) malloc(b->capacity);
    if (!b->ptr) {
        return E_ERROR;
    }
    b->cur = b->ptr;
    return E_OK;
}

int term_buf(struct buf_t* b)
{
    if (b->ptr) {
        free(b->ptr);
    }
    b->ptr = b->cur = NULL;
    b->capacity = b->bytes = 0;
    return E_OK;
}

int try_read_more(struct buf_t* b, int fd)
{
    if (b->cur != b->ptr) {
        if (b->bytes != 0) {
            memmove(b->ptr, b->cur, b->bytes);
        }
        b->cur = b->ptr;
    }

    if (b->bytes >= b->capacity) {
        char* new_ptr = (char*)realloc(b->ptr, b->capacity * 2);
        if (!new_ptr) {
            error(EXIT_FAILURE, errno, "not enough memory, realloc buf");
        }
        b->cur = b->ptr = new_ptr;
        b->capacity *= 2;
    }

    int avail = b->capacity - b->bytes;
    int nread = read(fd, b->cur + b->bytes, avail);
    if (nread > 0) {
        b->bytes += nread;
    }
    
    if (nread == 0) return E_ERROR;
    if (nread < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return E_OK;
        }
        
        log("read error, %s\n", strerror(errno));
        return E_ERROR;
    }

    return E_OK;
}

int get_line(struct buf_t* b, char** line, size_t* len)
{
    char* bp = (char*)memchr(b->cur, '\n', b->bytes);
    if (bp == NULL) {
        return E_NEED_MORE;
    }
    
    *line = b->cur;
    *len = ++bp - b->cur;
    b->cur = bp;
    b->bytes -= *len;
    return E_OK;
}
