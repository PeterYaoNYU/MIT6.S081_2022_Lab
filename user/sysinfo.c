#include "kernel/types.h"
#include "kernel/riscv.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

int main(int argc, char * argv)
{
    if (argc != 1){
        fprintf(2, "sysinfo: the para number must be 1\n");
        exit(1);
    }

    struct sysinfo p;
    sysinfo(&p);
    printf("Free memory(bytes) :%d, # Running processes: %d\n", p.freemem, p.nproc);
    return 0;

    
}