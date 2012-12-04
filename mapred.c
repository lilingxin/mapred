#include <unistd.h>
#include <libtask/task.h>
#include "os/mapred_files.h"
#include "buffer.h"

#define STACK_SIZE 32768

struct fd_buf_s {
    int fd;
    mapred_buf_t buf;
};

typedef struct fd_buf_s fd_buf_t;

static fd_buf_t stdin_buf;

int mapred_read_line(fd_buf_t* fdbuf, void** line, uint32_t* length)
{
    mapred_buf_t* buf = &(fdbuf->buf);
    if (mapred_buf_size(buf) == 0) {
        int nread = 0;
        int capactiy = mapred_buf_getcapacity(buf);
        if ( (nread = fdread(STDIN_FILENO, 
                             buf->ptr + buf->pos, 
                             capacity)) <= 0 ) {
            return -1;
        }
        buf->size = buf->pos + nread;
    }
    
    char* _line = NULL;
    uint32_t _length = 0;
    mapred_buf_getline(buf, &_line, &_length);
    if (mapred_buf_size(buf) == 0 && mapred_buf_capacity(buf) == 0) {
        mapred_buf_setcapacity(buf->capacity * 2);
        return 0;
    }
    
    if (mapred_buf_size(buf) == 0) {
        mapred_buf_readjust(buf);
        return 0;
    }
    
    *line = _line;
    *length = _length;
    return 0;     
}

int mapred_write_line(int fd, char* buf, uint32_t length) 
{
    return 0;
}

void taskprocess(void* args)
{
    int fd = (int)args;

    while (1) {
        char* line = NULL;
        uint32_t length = 0;

        int rc = mapred_read_line(&stdin_buf, &line, &length);
        if (rc == -1) break;
        if (line == NULL) {
            taskyield();
            continue;
        }
        
        fdwrite(fd, line, length);
    }
}

void mapred_spawn_children(const char* cmd, int num)
{
    int fdin = 0, fdout = 0;
    for (int i = 0; i < num; ++i) {
        mapred_spawn_process(cmd, &fdin, &fdout);        
        mapred_set_cloexec(fdin);
        mapred_set_cloexcc(fdout);
        taskcreate(taskprocess, (void*)fdout, STACK_SIZE);
    }        
}

int taskmain(int argc, char** argv)
{
    stdin_buf.fd = STDIN_FILENO;    
}
