#define _XOPEN_SOURCE
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <math.h>
#include <error.h>
#include <poll.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "child_ru_print.h"

void printArgs(char **argv,char **env) {
  for(int i=0;argv[i];i++)
    printf("argv[%d]=%s\n",i,argv[i]);

  for(int i=0;env[i];i++)
    printf("env[%d]=%s\n",i,env[i]);
}

void set_nonbuf(int fd, short int f) {
  static struct termios TermAttrs;

  struct termios t;

  if( tcgetattr(fd,&t) < 0 )
    errExit("tcgetattr");

  if( !TermAttrs.c_lflag)
    memcpy(&TermAttrs,&t,sizeof(struct termios));

  if( f ) {
    t.c_lflag &= ~ICANON;
    t.c_lflag &= ~ECHO;
    if( tcsetattr(fd,TCSANOW,&t) < 0 )
      errExit("tcsetattr");
  } else
    tcsetattr(fd,TCSANOW,&TermAttrs);
}

int main(int argc, char *argv[],char *env[]) {
  int master_pty ;
  char *pts_name = NULL;
  char *col = getenv("COLUMNS");
  struct FieldBuffer **fBuf;
  struct timeval start,end,diff;
  int width ;

  pid_t child_id ;

  gettimeofday(&start,NULL);
  if( col ) 
    width = round(strtol(col,NULL,10)*.9);
  else 
    width = round(DEFAULT_WIDTH*.9);

  //printf("width is %d,%s\n",width,col);
  master_pty = posix_openpt(O_RDWR|O_NOCTTY);
  if( master_pty < 0 ) 
    errExit("posix_openpt");

  if( !(pts_name = ptsname(master_pty)) )
    errExit("ptsname");

  if( grantpt(master_pty) )
    errExit("grantpt");

  if( unlockpt(master_pty) )
    errExit("unlockpt");

  child_id = fork();

  if( child_id > 0 ) {
    const int max_poll_fds = 2;
    const int buf_size = 1;
    struct pollfd poll_fds[max_poll_fds] ;
    size_t readSize;
    char buf[buf_size];
    int status;

    memset(poll_fds,'\0',sizeof(poll_fds));

    poll_fds[0].fd      = master_pty;
    poll_fds[0].events  = POLLIN;

    poll_fds[1].fd      = STDIN_FILENO;
    poll_fds[1].events  = POLLIN;

    set_nonbuf(STDIN_FILENO,1);
    while(1) {
      if( poll(poll_fds,max_poll_fds,-1) < -1)
        errExit("poll");

      if( poll_fds[0].revents & POLLIN ) {
        readSize = read(poll_fds[0].fd,buf,buf_size);
        if( readSize == buf_size ) {
          //SetUpStatsBuffer(&sBuf,buf[0]);
          write(STDOUT_FILENO,buf,buf_size);
        } else if(readSize == 0) { // eof
          break;
        } else  {
          errExit("read");
        }
      } else if( poll_fds[1].revents & POLLIN) {
        readSize = read(STDIN_FILENO,buf,buf_size);
        if( readSize == buf_size ) {
          write(master_pty,buf,buf_size);
        } else if(readSize == 0) { // eof
          break;
        } else  {
          errExit("read");
        }
      } else if( poll_fds[0].revents & POLLHUP) { //child closed its end
        wait(&status);
        gettimeofday(&end,NULL);
        timersub(&end,&start,&diff);
        fBuf = AllocateFieldBuffers(RES_COUNT);
        FillResourceUsage(fBuf,&diff);
        AlignFieldMaxWidthForGivenSize(fBuf,width);
        PrintFieldBuffers(fBuf);
        FreeFieldBuffers(fBuf);
        break;
      } else if( poll_fds[1].revents & POLLHUP) { 
        errExit("got POLLHUP on stdin!");
      } else if( poll_fds[0].revents & POLLNVAL) {
        errExit("got POLLNVAL on master_pty!");
      } else if( poll_fds[1].revents & POLLNVAL) {
        errExit("got POLLNVAL on stdin!");
      }
    }
  } else if( child_id == 0 ) {
    int slave_fd;

    slave_fd = open(pts_name,O_RDWR);
    if(slave_fd < 0)
      errExit("open");

    close(master_pty);
    for(short int i=0;i<3;i++) {
      close(i);
      dup2(slave_fd,i);
    }

    execvp(argv[1],&(argv[1]));
    errExit("execvp");
  } else {
    errExit("fork");
  }
  set_nonbuf(STDIN_FILENO,0);
}
