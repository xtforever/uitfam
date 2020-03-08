#ifndef SAFE_WRITE_H
#define SAFE_WRITE_H

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

void xwrite(int line, const char *fn, const char *func, int fd, void *b, int n);
#define safe_write(a,b,c) xwrite(__LINE__,__FILE__,__func__,(a),(b),(c))

#endif
