#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <event.h>
#include <stdbool.h>
#include <pthread.h>
#include "buf.h"
#include "util.h"

enum 
{
    STAT_READ_MORE,
    STAT_WRITE_LINE,
    STAT_LAST_BUF,
    STAT_CLOSE_PIPE
};

typedef struct thread_t
{
    pthread_t id;
    struct event_base* base;
    struct buf_t buf;
    int st;
    int evno;
    struct event ev[];
} WRITE_EV_THREAD;

typedef struct 
{
    pthread_t id;
    struct event_base* base;
    int evno;
    struct fdev_t
    {
        int fd;
        struct event ev;
        struct buf_t buf;
    } evs[];
} READ_EV_THREAD;

static WRITE_EV_THREAD* wet;
static READ_EV_THREAD* ret;

static void update_stat(int stat)
{
    wet->st = stat;
}

void event_handler(int fd, short e, void* args)
{
    int st = wet->st;
    struct buf_t* buf = &wet->buf;
    int rc = -1;
    char* line = NULL;
    size_t len = 0;
    struct event* ev = (struct event*)args;

    switch (st) {
    case STAT_READ_MORE:
        rc = try_read_more(buf, STDIN_FILENO);
        if (rc == E_ERROR) 
            update_stat(STAT_LAST_BUF);
        else
            update_stat(STAT_WRITE_LINE);
        break;

    case STAT_LAST_BUF:
        if (write(fd, buf->cur, buf->bytes) < 0) {
            log("write error, %s\n", strerror(errno));
        } 
        update_stat(STAT_CLOSE_PIPE);
        break;

    case STAT_WRITE_LINE:
        if (get_line(buf, &line, &len) != E_OK) {
            update_stat(STAT_READ_MORE);
            break;
        }

        if (write(fd, line, len) < 0) {
            log("write error, %s\n", strerror(errno));
            update_stat(STAT_CLOSE_PIPE);
        }
        break;

    case STAT_CLOSE_PIPE:
        close(fd);
        event_del(ev);
        break;
    } 
}

void revent_handler(int fd, short e, void* args)
{
    struct fdev_t* fdev = (struct fdev_t*)args;
    struct buf_t* b = &fdev->buf;
    char *line = NULL;
    size_t length = 0;

    if (try_read_more(b, fd) == E_ERROR) {
        write(STDOUT_FILENO, b->cur, b->bytes);
        event_del(&fdev->ev);
        return;
    }

    for ( ;get_line(b, &line, &length) == E_OK; ) {
        write(STDOUT_FILENO, line, length);
    }    
}

int thread_init(int num)
{
    wet = (WRITE_EV_THREAD*)malloc(sizeof(WRITE_EV_THREAD) + num * sizeof(struct event));
    wet->base = event_base_new();
    wet->evno = num;
    wet->st = STAT_READ_MORE;
    init_buf(&wet->buf, 4096);

    ret = (READ_EV_THREAD*)malloc(sizeof(READ_EV_THREAD) + num * sizeof(struct fdev_t));
    ret->base = event_base_new();
    ret->evno = num;
    return 0;
}

int thread_add(int wfd, int rfd) 
{
    static int i = 0;
    if (i >= wet->evno) {
        log("thread add error, out of bouder\n");
        return -1;
    }
    event_set(&wet->ev[i], wfd, EV_WRITE | EV_PERSIST, event_handler, (void*)&wet->ev[i]);
    event_base_set(wet->base, &wet->ev[i]);
    event_add(&wet->ev[i], 0);
    
    struct fdev_t* evs = ret->evs;
    event_set(&evs[i].ev, rfd, EV_READ | EV_PERSIST, revent_handler, (void*)&evs[i]);
    event_base_set(ret->base, &evs[i].ev);
    event_add(&evs[i].ev, 0);
    init_buf(&evs[i].buf, 4096);

    ++i;
    return 0;
}

static void* wthread_proc(void* args) 
{
    WRITE_EV_THREAD* et = (WRITE_EV_THREAD*)args;
    event_base_loop(et->base, 0);
    return NULL;
}

static void* rthread_proc(void* args)
{
    READ_EV_THREAD* et = (READ_EV_THREAD*)args;
    event_base_loop(et->base, 0);
    return NULL;
}

int thread_start()
{
    if (pthread_create(&wet->id, NULL, wthread_proc, (void*)wet) < 0) {
        log("pthread_create error\n");
        return -1;
    }

    if (pthread_create(&ret->id, NULL, rthread_proc, (void*)ret) < 0) {
        log("pthread_create error\n");
        return -1;
    }
    return 0;
}

int thread_term()
{
    event_base_free(wet->base);
    term_buf(&wet->buf);
    free(wet);
    
    void* retval = NULL;
    pthread_join(ret->id, &retval);
    event_base_free(ret->base);
    for (int i = 0; i < ret->evno; ++i) {
        term_buf(&ret->evs[i].buf);
    }
    free(ret);
    return 0;
}
