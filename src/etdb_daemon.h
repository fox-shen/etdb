#ifndef H_ETDB_DAEMON_H
#define H_ETDB_DAEMON_H

extern pid_t etdb_pid;

int
etdb_daemon()
{
    int  fd;

    switch (fork()) {
    case -1:
        return -1;

    case 0:
        break;

    default:
        exit(0);
    }

    etdb_pid = getpid();

    if (setsid() == -1) {
        return -1;
    }

    umask(0);

    fd = open("/dev/null", O_RDWR);
    if (fd == -1) {
        return -1;
    }

    if (dup2(fd, STDIN_FILENO) == -1) {
        return -1;
    }

    if (dup2(fd, STDOUT_FILENO) == -1) {
        return -1;
    }

    if(dup2(fd, STDERR_FILENO) == -1){
        return -1;
    }

    return 0;
}

#endif
