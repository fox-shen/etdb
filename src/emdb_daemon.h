#ifndef H_EMDB_DAEMON_H
#define H_EMDB_DAEMON_H

extern pid_t emdb_pid;

int
emdb_daemon()
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

    emdb_pid = getpid();

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
