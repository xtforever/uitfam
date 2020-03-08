#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>

int  main(int argc, char **argv)
{
  char *devicename = "/dev/ttyUSB0";
  int fd,i;

  if( argc < 2 ) {
    fprintf(stderr,"%s devicename\ndrops dtr for 1 second\n", argv[0]);
    exit(255); 
  }


  if( argc >= 2 ) 
    devicename = argv[1];
  
  printf("Using device %s\n", devicename );

  fd = open(devicename, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if( fd<0 ) {
    perror(devicename);
    exit(errno);
  }

  i=0;
  i |= TIOCM_DTR; // DTR 

  ioctl( fd, TIOCMBIC, &i );
  usleep(1000);
  ioctl( fd, TIOCMBIS, &i );
  
  close(fd);
  return 0;
}
