#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char * argv[])
{
	if (argc <2){
		fprintf(2, "the command has at least two args\n");
		exit(1);
	}

	char buf[1024];
	char * beg = buf;
	int rc;
	char * child_argv[MAXARG];
	int pos_arg = 0;
 	for (int i = 1; i <= argc; ++i){
		strcpy(child_argv[i-1], argv[i]);
		pos_arg++;
	}
	int ori_pos_arg = pos_arg;



	while (read(0, beg++, 1) != 0){
		if (*(--beg) == '\n'){
			rc = fork();
			if (rc == 0){
				// child proc running
				child_argv[pos_arg]=0;
				exec(child_argv[0], child_argv);
			}else if (rc > 0){
				wait((int *)0);
				for (int i = ori_pos_arg; i<MAXARG; ++i){
					child_argv[i]= "0";
				}
			}
		else if (*(--beg) == ' '){
			*(--beg) = '\0';
			strcpy(child_argv[pos_arg++], buf);
			memset(buf, 0, 1024);
			beg = buf;
		}
		}
	}
	return 0;
}
