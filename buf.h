#ifndef _BUF_H
#define _BUF_H

enum 
{
    E_OK = 0,
    E_ERROR,
    E_NEED_MORE
};

struct buf_t
{
    char* ptr;
    char* cur;
    size_t bytes;
    size_t capacity;
};

int init_buf(struct buf_t* buf, int alloc);
int try_read_more(struct buf_t* buf, int fd);
int get_line(struct buf_t* buf, char** line, size_t* len);
int term_buf(struct buf_t* buf);

#endif /* _BUF_H */
