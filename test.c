#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "buffer.h"

int main(void)
{
    int nread = 0;
    char* line = NULL;
    uint32_t length = 0;
    char buffer[4096] = {0};

    int stop = 0;
    mapred_buf_t buf = {NULL, 0, 0, 0};

    mapred_buf_init(&buf, 4096);
    while (!stop) {
        int capacity = mapred_buf_getcapacity(&buf);
        nread = read(STDIN_FILENO, buf.ptr + buf.pos, capacity);
        stop = nread >= capacity ? 0 : 1;
        buf.size = buf.pos + nread;
        
        int stop_buf = 0;
        while (!stop_buf) {
            mapred_buf_getline(&buf, (void**)&line, &length);
            stop_buf = mapred_buf_size(&buf) ? 0 : 1;
            if (!stop_buf && !stop) {
                memmove(buf.ptr, buf.ptr + buf.pos, buf.size - buf.pos);
                buf.pos = 0;
                buf.size = buf.size - buf.pos;
                break;
            }
            memcpy(buffer, line, length);
            buffer[length] = '\0';
            printf("%s", buffer);
        }
    }

    return 0;
}
