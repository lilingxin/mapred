#ifndef _MAPRED_H
#define _MAPRED_H

#include <stdint.h>

struct mapred_buf_s {
    void* ptr;
    uint32_t pos;
    uint32_t size;
    uint32_t capacity;
};

typedef struct mapred_buf_s mapred_buf_t;

void mapred_buf_init(mapred_buf_t* buf, uint32_t capacity);
void mapred_buf_free(mapred_buf_t* buf);
void mapred_buf_readjust(mapred_buf_t* buf);
int mapred_buf_setcapacity(mapred_buf_t* buf, uint32_t capacity);
uint32_t mapred_buf_getcapacity(mapred_buf_t* buf);
int mapred_buf_getline(mapred_buf_t* buf, void** line, uint32_t* length);
uint32_t mapred_buf_size(mapred_buf_t* buf);

#endif /* _MAPRED_H */
