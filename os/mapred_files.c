#include "mapred_files.h"

static char* get_shell()
{
    char* shell = getenv("SHELL");
    return shell ? shell : "/bin/sh";
}

static char* last_component(const char* str)
{
    char* bp = strrchr(str, (int)'/');
    return ++bp;
}

int mapred_read_fd(int fd, void* buf, uint32_t size)
{
    int nread = 0;
    char* ptr = buf;
    while (size > 0) {
        nread = read(fd, ptr, size);
        if (nread < 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN) break;
            error(EXIT_FAILURE, errno, "read error");
        }

        if (nread == 0) break;
        ptr += nread;
        size -= nread;
    }
    return ptr - buf;
}

int mapred_write_fd(int fd, void* buf, uint32_t size) 
{
    int nwrite = 0;
    char* ptr = buf;

    while (size > 0) {
        nwrite = write(fd, ptr, size);
        if (nwrite < 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN) break;
            error(EXIT_FAILURE, errno, "write error");
        } 
        if (nwrite == 0) break;
        ptr += nwrite;
        size -= nwrite;
    }
    return ptr - buf;
}

#define check_exp(x, message) \
    do { \
        if ((x) < 0) { \
            error(EXIT_FAILURE, errno, message); \
        } \
    } while (0)

int mapred_set_noblocking(int fd) 
{
    int flags = fcntl(fd, F_GETFD, 0);
    check_exp(flags >= 0);
    return fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
}

int mapred_set_cloexec(int fd) 
{
    int flags = fcntl(fd, F_GETFD, 0);
    check_exp(flags >= 0);
    return fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
}

int mapred_spawn_process(const char* cmd, int* in, int* out)
{
    int in_fds[2], out_fds[2];
    int pid = -1;
    check_exp(pipe(in_fds), "pipe error");
    check_exp(pipe(out_fds), "pipe error");
    check_exp(pid = fork(), "fork error");
    
    if (pid == 0) {
        close(in[0]);
        close(out[1]);
        check_exp(dup2(out[0], STDIN_FILENO));
        close(out[0]);
        char* shell = get_shell();
        execl(shell, last_componect(shell), "-c", cmd, NULL);
        error(EXIT_FAILURE, errno, "execl error");
    }

    close(in[1]);
    close(out[0]);

    *in = in[0];
    *out = out[1];
    return pid;
}
