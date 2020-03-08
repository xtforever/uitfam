#include "safe_write.h"
#include "unistd.h"
#include "stdlib.h"

/* Write "n" bytes to a descriptor. */
ssize_t writen(int fd, const void *vptr, size_t n)
{
    size_t      nleft;
    ssize_t     nwritten;
    const char  *ptr;
    uint timeout = 1000;
    uint tmr;
    if( n == 0 ) return 0;

    ptr = vptr;
    nleft = n;
    tmr = timeout;
    while (nleft > 0) {
        nwritten = write(fd, ptr, nleft);
        if( nwritten > 0) {
            nleft -= nwritten;
            ptr   += nwritten;
            tmr = timeout;
            continue;
        }

        if( (errno != EINTR) && (errno != EAGAIN) && (errno != EWOULDBLOCK) )
            return n-nleft; /* error */

        if( tmr == 0 ) return n-nleft; /* error, timeout */
        tmr--;
        usleep(1000); /* wait 1ms and call write() again */
    }
    return n-nleft;
}

void xwrite(int line, const char *fn, const char *func, int fd, void *b, int n)
{
    int res = writen(fd,b,n);
    if( n!=res ) {
        fprintf(stderr,"%s:%s:%d:write failed. expected %d got %d: %m\n", fn,func,line, n, res );
	exit(1);
    }
}
