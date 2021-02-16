#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#include "tracer_final.h"
#include "syscall_handle.h"
enum syscalls {
	SYSCALL_OPEN = 2,
	SYSCALL_OPENAT = 257	
};


int main(int argc, char* argv[], char* envp[])
{
	pid_t pid[10];
	int in_syscalls[10];
	int i = 0;
	int n = 1;

	union machine_word word;

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
						if (res.registers.orig_rax == -1) {
							res.registers.rax = -EPERM;
							u_set_regs(res.pid, &res.registers);
						}							
						if ((res.registers.orig_rax == SYSCALL_OPENAT)) {
							if (openat_handler(pid[i], &res.registers, in_syscalls[i]) == -1)
								printf("%d: openat_handler failed\n", pid[i]);
							/*word.num = ptrace(PTRACE_PEEKDATA, res.pid, res.registers.rsi, NULL);

							write(1, word.buf, 8);
							printf("\n");
							res.registers.orig_rax = -1;
							u_set_regs(res.pid, &res.registers);*/
						} 

						if ((res.registers.orig_rax == SYSCALL_OPEN)) {
							if (open_handler(pid[i], &res.registers, in_syscalls[i]) == -1)
								printf("%d: open_handler failed\n", pid[i]);
						}


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

				u_continue_after_syscall(res.pid);
				break;
			default:
				printf("Strange res.retval=%d\n", res.retval);
				return 0;
		}

	}
}