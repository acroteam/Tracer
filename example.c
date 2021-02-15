#include <stdio.h>
#include <sys/types.h>
#include "tracer_final.h"

int main(int argc, char* argv[], char* envp[])
{
	pid_t pid[10];
	int in_syscalls[10];
	int i = 0;
	int n = 1;

	for (i = 0; i < n; ++i) {
		scanf("%d", pid + i);

		if (attach_process(pid[i])) {
			printf("attach fails\n");
			return 0;
		}

		in_syscalls[i] = 0;
	}



	struct ret_sys_info res;
	while(1) {
		res = wait_for_syscall(2);

		switch (res.retval) {
			case SIGNAL:
				printf("SIGNAL\n");
				break;
			case TIMEOUT:
				break;
			case ERROR:
				return 0;
			case DEAD:
				printf("PID: %d is DEAD\n", res.pid);
				break;
			case ALL_DEAD:
				printf("ALL DEAD\n");
				return 0;
			case GROUPSTOP:
				printf("GS\n");
				break;
			case FORK:
				if (res.pid == 0) {
					break; // this should maybe restart infide wait_for_syscall()
				}
				printf("FORK %d\n", res.pid);
				pid[n] = res.pid;
				in_syscalls[n] = 0;
				n++;
				break;
			case SYSCALL:
				for (i = 0; i < n; ++i) {
					if (res.pid == pid[i]) {
						if (in_syscalls[i] == 0) {
							in_syscalls[i] = 1;
							printf("%d: Syscall %lld start\n", res.pid, res.registers.orig_rax);
						}
						else {
							in_syscalls[i] = 0;
							printf("%d: Syscall %lld = %lld\n", res.pid, res.registers.orig_rax, res.registers.rax);
						}
						break;
					}
				}
				ptrace(PTRACE_SYSCALL, res.pid, 0, 0);
				break;
			default:
				printf("Strange res.retval=%d\n", res.retval);
				return 0;
		}

	}
}