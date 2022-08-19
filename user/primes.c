#include "kernel/types.h"
#include "user/user.h"
#include "kernel/stat.h"

void swap_array(int *left_buf, int *right_buf){
  int temp;
  for (int i =0; i<34 ; ++i){
    temp = right_buf[i];
    right_buf[i]=left_buf[i];
    left_buf[i] = temp;
  }
}

int main(int argc, char * argv [])
{
  int left_buf [34];
  int right_buf[34];
  int temp [34];
  int p1[2];
  pipe(p1);

  for (int i =2; i<36; ++i){
    right_buf[i-2]= i;
  }

  int sieve_num = 2;

  while(1){
    swap_array(left_buf, right_buf);
    int rc = fork();
    if (rc==0){
      memset(right_buf, 0, 34);
      if (read (p1[0], right_buf, sizeof(right_buf))==0){
        break;
      };
      sieve_num = right_buf[0];

    } else if (rc>0){
      
      int j =0;
      memset(temp, 0, 34);
      for (int i =0; i<35; ++i){
        if (left_buf[i]%sieve_num!=0 && left_buf[i] != sieve_num){
          temp[j++]=left_buf[i];
        }
      printf("prime %d\n", sieve_num);
      
      write(p1[1], temp, sizeof(temp));
      memset(left_buf, 0, 34);
      wait(0);
      }
    }
  }

  exit(0);
}
