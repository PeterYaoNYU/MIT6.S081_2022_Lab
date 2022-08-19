
#include "kernel/types.h"
#include "user/user.h"
#include "kernel/stat.h"

#define BUFSIZE 30

int main(int argc, char * argv)
{
  if(argc != 1){
    fprintf(1, "wrong number of inputs, no parameter allowed");
    exit(1);
  }

  int fds1[2];
  int fds2[2];

  char buf1[BUFSIZE];
  char buf2[BUFSIZE];

  char * p ="p";
  char * c = "c";
  
  pipe(fds1);
  pipe(fds2);
  int rc = fork();
  if (rc != 0){
    close(fds1[0]);
    close(fds2[1]);
    write(fds1[1], p, 1);
    sleep(3);
    if (read(fds2[0], buf1, 1)!=1){
      printf("parent read error\n");
      exit(1);
    }
    printf("%d: received pong\n", getpid());
    close(fds1[1]);
    close(fds2[0]);
    exit(0);
  } else if (rc ==0){
    close(fds1[1]);
    close(fds2[0]);

    if (read (fds1[0], buf2, 1)!=1){
      printf("child read error\n");
      exit(1);
    }
    sleep(2);
    printf("%d: received ping\n", getpid());
    write(fds2[1], c, 1);
    close(fds1[0]);
    close(fds2[1]);
    exit(0);
  }
  return 0;
}
