#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "safe_write.h"

#define BAUD_RATE B500000
#define MODEMDEVICE "/dev/ttyACM0"
#define _POSIX_SOURCE 1         //POSIX compliant source

#define PREFIX_B(x) B ## x
#define TO_BAUD(x) PREFIX_B(x), x

long lst_baud[] = {
  B50, 50,
  B75, 75,
  B110, 110,
  B134, 134,
  B150, 150,
  B200, 200,
  B300, 300,
  B600, 600,
  B1200, 1200,
  B1800, 1800,
  B2400, 2400,
  B4800, 4800,
  B9600,9600,
  B19200,19200,
  B38400,38400,
  B57600,57600,
  TO_BAUD(115200),
  TO_BAUD(230400),
  TO_BAUD(460800),
  TO_BAUD(500000),
  TO_BAUD(576000),
  TO_BAUD(921600),
  TO_BAUD(1000000),
  TO_BAUD(1152000),
  TO_BAUD(1500000),
  TO_BAUD(2000000),
  TO_BAUD(2500000),
  TO_BAUD(3000000),
  TO_BAUD(3500000),
  TO_BAUD(4000000)

};




void signal_handler_IO (int status);    //definition of signal handler


volatile int keep_going;

void catch_usr1 (int sig)
{
  keep_going = 0;
  signal (sig, catch_usr1);
}

void catch_usr2 (int sig)
{
  keep_going = 1;
  signal (sig, catch_usr2);
}



int CRLF = 1;


static int tty, fd;
struct termios oldtio, oldkey;

int open_tty()
{
  struct termios newkey;

  tty = open("/dev/tty", O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (tty < 0)
    {
      perror("/dev/tty");
      exit(-1);
    }

  // set the user console port up
  // save current port settings
  // so commands are interpreted right for this program
  // set new port settings for non-canonical input processing
  // must be NOCTTY
  tcgetattr(tty,&oldkey);

  memcpy( (void*)&newkey, (void*)&oldkey, sizeof newkey );
  newkey.c_lflag &= ~(ICANON | ECHO);
  newkey.c_cc[VMIN] = 1;
  newkey.c_cc[VTIME] = 0;

  tcflush(tty, TCIFLUSH);
  tcsetattr(tty,TCSANOW,&newkey);
  return tty;
}

void close_tty()
{
  tcflush(tty, TCIFLUSH);
  tcsetattr(tty,TCSANOW,&oldkey);
  close(tty);
}


// Parity as follows:
// 00 = NONE, 01 = Odd, 02 = Even, 03 = Mark, 04 = Space
int open_ser( long baud, char *devicename )
{
   long BAUD = baud;
   long DATABITS=CS8;
   long STOPBITS=0;
   long PARITYON=0;
   long PARITY=0;
   struct termios newtio;

  fd = open(devicename, O_RDWR  | O_NONBLOCK);
  if (fd < 0)
    {
      perror(devicename);
      exit(-1);
    }
  tcgetattr(fd,&oldtio);
  sleep(1);    // wait one second before configuring this port

  memcpy( (void*)&newtio, (void*)&oldtio, sizeof newtio );

  newtio.c_cflag = 0
     | BAUD             // pre-defined values e.g. B19200
     | DATABITS         // CS8, CS7
     | STOPBITS         // 0 or 1
     | PARITYON         // 0 or 1
     | PARITY           // ??
     | CLOCAL           // ?? #define
     | CREAD;           // ?? #define

  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;
  newtio.c_lflag = 0;       // ICANON;
  newtio.c_cc[VMIN]=1;
  newtio.c_cc[VTIME]=0;

  tcflush(fd, TCIFLUSH);
  tcsetattr(fd,TCSANOW,&newtio);
  return fd;
}

void close_ser()
{
  tcflush(fd, TCIFLUSH);
  tcsetattr(fd,TCSANOW,&oldtio);
  close(fd);
}

void drop_dtr(void)
{
  int i=0;
  i |= TIOCM_DTR | TIOCM_RTS; // DTR
  ioctl( fd, TIOCMBIC, &i );
  usleep(1000);
  ioctl( fd, TIOCMBIS, &i );
  puts("\nRESET");
}


int HEXMODE = 0;

void to_screen(unsigned char ch)
{
  char buf[10];
  if( HEXMODE ) {
    sprintf(buf,"%02X ", ch );
    safe_write( STDOUT_FILENO, buf, 3 );
  }
  else {
    if( ch != '\r' )
      safe_write( STDOUT_FILENO, &ch, 1 );
  }
}


int xgetc( int fd, int timeout_ms )
{
  fd_set rfds;
  struct timeval tv;
  int retval;
  char buf[1];

  FD_ZERO(&rfds);
  FD_SET(fd, &rfds);
  /* Warte auf daten */
  tv.tv_sec = 0;
  tv.tv_usec = timeout_ms*1000;
  retval = select(fd+1, &rfds, NULL, NULL, &tv);
  /* Verlaß Dich jetzt bloß nicht auf den Wert von tv! */

  if (retval==1 && FD_ISSET(fd,&rfds) ) {
    if( read( fd, buf, 1 ) == 1 ) return *buf;
  }

  return -1;
}


int main(int argc, char **argv )
{
  int res;
  char ch;
  long b;

  char *devicename;
  long baud;

  baud = BAUD_RATE;
  devicename = MODEMDEVICE;

  if( argc >= 2 ) {
    devicename=argv[1];
  }

  if( argc >= 3 ) {
    b = atol(argv[2]);
    for( res=0; res < sizeof(lst_baud)/sizeof(*lst_baud); res+=2 )
      if( lst_baud[res+1] == b ) {
	baud = lst_baud[res];
      }
  }

  printf("DEV: %s - Baud: %ld\n", devicename, baud );
  printf("Hexmode: Ctrl-H\n"
         "Drop-Dtr: Ctrl-D\n" );

 restart:
  keep_going=1;
  signal (SIGUSR1, catch_usr1);
  signal (SIGUSR2, catch_usr2);

  open_ser( baud, devicename );
  open_tty();
  puts("Running, stop with USR1");

  int ret,max;
  fd_set fds;
  if( tty > fd ) max=tty+1; else max=fd+1;

  while( keep_going ) {
    FD_ZERO( &fds );
    FD_SET(tty,&fds);
    FD_SET(fd,&fds);
    ret = select( max, &fds, NULL, NULL, NULL );
    if (ret == -1 && errno == EINTR)
      continue;
    if (ret < 0) {
      perror ("select()");
      exit (1);
    }


    if( FD_ISSET(tty,&fds) ) {
      res = read (tty, &ch, 1);
      if( res == 1 ) {
	if( ch == 0x1b ) break; // leave this while loop

        if( ch == 8 ) { HEXMODE ^= 1; goto skip_write; }
        if( ch == 4 ) { drop_dtr(); goto skip_write; }
	if( CRLF && ch == '\n' ) {
	  ch= 13;
          safe_write( fd, &ch, 1 ); to_screen(ch);
	  ch = 10;
	}
	safe_write( fd, &ch, 1 );
        to_screen(ch);
      }
    }


  skip_write:

    if( FD_ISSET(fd,&fds)) {
      if( read(fd,&ch,1) == 1 ) to_screen(ch);
    }
  }

  close_tty();
  close_ser();

  if( keep_going == 0 ) {
    puts("Stopped, waiting for USR2");
    while( keep_going == 0 ) select( 0,NULL,NULL,NULL,NULL );
    goto restart;
  }

  return EXIT_SUCCESS;
}
