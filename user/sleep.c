#include "kernel/types.h"
#include "user/user.h"
#include "kernel/types.h"

int main(int argc, char * argv[])
{
  if (argc != 2){
    fprintf(2, "wrong number of inputs!");
  }

  int time;
  time = atoi(argv[1]);
  sleep(time);
  exit(0);
}


