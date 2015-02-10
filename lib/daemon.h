#ifndef _DEAMON_H_
#define _DAEMON_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/file.h>

#define DEFAULT_FD 1024

//daemonize process
inline void daemonize() {
    umask(0);

    pid_t pid;
    if ( (pid = fork()) < 0) {
        printf("first fork error %s", strerror(errno));
        exit(1);
    } else if (pid != 0) {
        exit(0);
    }

    setsid();

    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGHUP, &sa, NULL);

    if ( (pid = fork()) < 0) {
        printf("second fork error %s", strerror(errno));
        exit(1);
    } else if (pid != 0){
        exit(0);
    }
}

#define PIDFILE ".daemon.pid"
#define PIDLEN 64

inline bool isrunning() {
    int fd = open(PIDFILE, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG);

    if (flock(fd, LOCK_EX | LOCK_NB) < 0) 
        return true;

    ftruncate(fd, 0);

    char buff[PIDLEN];
    snprintf(buff, sizeof(buff), "%d\n", getpid());

    write(fd, buff, strlen(buff));

    return false;
}

#endif
