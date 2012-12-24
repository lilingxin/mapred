#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <error.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "util.h"

extern int set_noblocking(int fd);
extern int set_cloexec(int fd);
extern int spawn_process(const char*, int*, int*);
extern void wait_children();

extern int thread_init(int);
extern int thread_add(int, int);
extern int thread_start();
extern int thread_term();

static char cmd[4096];
static int count = 2;
static int pids[1024];
static char file[1024];

static void usage(char* proc)
{
    fprintf(stdout, "%s usage : \n"
            "%s [option] [INPUT] \n"
            "option is :\n"
            " --mapper | -m the command to execute in shell\n"
            " --count  | -c the process num in background\n"
            " --help   | -h \n"
            , proc, proc);
}

int getopts(int argc, char** argv)
{
    char* short_opt = "m:c:h";
    struct option long_opt[] = {
        {"mapper", required_argument, NULL, 'm'},
        {"count", required_argument, NULL, 'c'},
        {"help", no_argument, NULL, 'h'}
    };
    
    int o = -1;
    while ((o = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1) {
        switch (o) {
        case 'm':
            memset(cmd, 0, 4096);
            memcpy(cmd, optarg, strlen(optarg));
            break;
        case 'c':
            count = atoi(optarg);
            break;
        case 'h':
            usage(argv[0]);
            break;
        }
    }
    
    if (optind < argc) {
        int fd = open(argv[optind++], O_RDONLY);
        if (fd < 0 || dup2(fd, STDIN_FILENO) < 0) {
            error(EXIT_FAILURE, errno, "open file error");
        }
        close(fd);
    }
    return 0;
}

void sig_handler(int signo)
{
    if (signo == SIGQUIT || signo == SIGTERM 
        || signo == SIGINT) {
        for (int i = 0; i < count; ++i) {
            kill(pids[i], signo);
        }    
    }
}

int main(int argc, char* argv[])
{
    signal(SIGPIPE, SIG_IGN);
    signal(SIGQUIT, sig_handler);
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    getopts(argc, argv);
    thread_init(count);

    int in = 0, out = 0;
    for (int i = 0; i < count; ++i) {
        int pid = spawn_process(cmd, &in, &out);
        set_cloexec(out);
        set_cloexec(in);
        thread_add(out, in);
        pids[i] = pid;
    }

    thread_start();
    wait_children();
    thread_term();
    return 0;
}
