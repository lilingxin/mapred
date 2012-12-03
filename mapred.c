#include <unistd.h>
#include "buffer.h"

struct fd_buf_s {
    int fd;
    mapred_buf_t buf;
};

typedef struct fd_buf_s fd_buf_t;

int mapred_read_line(fd_buf_t* fdbuf, void** line, uint32_t* length)
{
     
}
