#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <execinfo.h>
#include <stdio.h>
#include "libtask/task.h"
#include "os/mapred_files.h"
#include "buffer.h"
#include "util.h"

#define STACK_SIZE 32768

struct fd_buf_s {
    int fd;
    mapred_buf_t buf;
};

typedef struct fd_buf_s fd_buf_t;

static fd_buf_t stdin_buf;
static int rdeof = 0;
static int children = 2;

int read_to_buf(mapred_buf_t* buf, int fd)
{
    int capacity = mapred_buf_getcapacity(buf);
    if (capacity <= 0)
        return 0;
    int nread = read(fd, buf->wp, capacity);
    buf->wp += nread;
    if (nread < capacity) return -1;
    return 0;
}

int get_buf_line(mapred_buf_t* buf, void** line, uint32_t* length)
{
    *length = 0;
    int rc = mapred_buf_getline(buf, line, length);
    if (rc < 0) return -1;
    if (rc == 0 && buf->rp == buf->ptr) {
        mapred_buf_setcapacity(buf, buf->capacity * 2);
        return 1;
    }

    if (rc == 0) {
        mapred_buf_readjust(buf);
        return 1;
    }
    return 0; 
}

int mapred_write_line(int fd, char* buf, uint32_t length) 
{
    return 0;
}

void taskreadstdin(void* args)
{
    while (1) {
        int rc = read_to_buf(&(stdin_buf.buf), STDIN_FILENO);
        if (rc < 0) break;
        taskyield();
    }
    rdeof = 1;
}

void taskwritebuf(void* args)
{
    int fd = (int)args;
    char* line = NULL;
    uint32_t length = 0;

    while (1) {
        int rc = get_buf_line(&(stdin_buf.buf), (void**)&line, &length);
        if (rc != 0) {
            if (rdeof) {
                if (rc == 1) write(fd, line, length);
                break;
            }
        }
        else {
            write(fd, line, length);
        }

        taskyield();
    }
}

void mapred_spawn_children(const char* cmd, int num)
{
    int fdin = 0;
    int fdout = 0;
    for (int i = 0; i < num; ++i) {
        mapred_spawn_process(cmd, &fdin, &fdout);        
        mapred_set_cloexec(fdout);
        taskcreate(taskwritebuf, (void*)fdout, STACK_SIZE);
    }        
}

void wait_children(void * args)
{
    int status = -1;
    int pid = -1;
    while ( (pid = waitpid(-1, &status, WNOHANG)) > 0 ) {
        if (WIFSIGNALED(status)) {
            int signo = WTERMSIG(status);
            log("child [%d] exit by signal %d\n", pid, signo);
        }

        if (--children == 0) {
            taskexitall(1);
        }
    }
}

static void btrace(int depth)
{
    void **btrace = (void**)malloc(sizeof(void *) * depth);
    size_t bt_size, i;
    char **bt_strings;

    bt_size = backtrace(btrace, depth);
    bt_strings = backtrace_symbols(btrace, bt_size);
    log("*** backtrace of %d ***\n", (int) getpid());
    for(i=0;i<bt_size;i++) 
    {   
        log("%s\n", bt_strings[i]);
    }    
    free(btrace);
    log("*** end of backtrace ***\n");
}

void sig_handle(int sig)
{
    if (sig == SIGSEGV) {
        btrace(10);
    }

    if (sig == SIGCHLD) {
        wait_children(NULL);
    }
}

void taskmain(int argc, char* argv[])
{
    signal(SIGSEGV, sig_handle);
    signal(SIGCHLD, sig_handle);
    stdin_buf.fd = STDIN_FILENO;    
    mapred_buf_init(&(stdin_buf.buf), 4096);

    const char* cmd = argv[1];
    taskcreate(taskreadstdin, NULL, STACK_SIZE);
    mapred_spawn_children(cmd, 2);
    // taskcreate(wait_children, NULL, STACK_SIZE);
}
