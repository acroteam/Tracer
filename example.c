#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>


#include "set.h"
#include "tracer_final.h"
#include "syscall_handle.h"

#define UPDATE_LOOP 10
#define TOUT 2

enum syscalls {
	SYSCALL_OPEN = 2,
	SYSCALL_OPENAT = 257	
};


int collect_pids(DIR* dir_ptr, Set* set)
{
	int n = 0;
	struct dirent* s_dir_ptr = NULL;
	char* ptr = NULL;

	pid_t my_pid = getpid();
	while (s_dir_ptr = readdir(dir_ptr)) {
		struct pid_inf s_inf;
		s_inf.pid = strtol(s_dir_ptr->d_name, &ptr, 10);
		if (ptr && s_dir_ptr->d_name != ptr && *ptr == '\0' && s_inf.pid != my_pid) {
			s_inf.in_syscall = 0;
			add(set, s_inf);
			n++;
		}
	}
	rewinddir(dir_ptr);
	return n;
}



int check_pid(pid_t pid)
{
	if (pid < 7000 || pid == getpid())
		return 0;
	return 1;
}



int main(int argc, char* argv[], char* envp[])
{
	Set set = init_set(0);
	DIR* dir_ptr = opendir("/proc");
	int m = collect_pids(dir_ptr, &set);


	int i = 0;

	union machine_word word;



	char c, tr;
	for (i = 0; i < set.len; ++i) {
		if (!check_pid(set.arr[i].pid))
			continue;

		printf("Attaching %d\n", set.arr[i].pid);
		if (attach_process(set.arr[i].pid)) {
			perror("Attach: ");
		}
	}



	struct ret_sys_info res;
	int count = 0;
	while(1) {
		count++;
		if (count == UPDATE_LOOP) {
			count = 0;

			Set tmp_set = init_set(0);
			int n = collect_pids(dir_ptr, &tmp_set);
			printf("COLLECT %d\n", n);
			sleep(5);
			for (i = 0; i < tmp_set.len; ++i) {
				if (!check_pid(tmp_set.arr[i].pid) || find_pid(&set, tmp_set.arr[i].pid) != -1)
					continue;

				printf("Attaching %d\n", tmp_set.arr[i].pid);
				if (attach_process(tmp_set.arr[i].pid)) {
					perror("Attach: ");
					//return 0;
				}
				else {
					add(&set, tmp_set.arr[i]);
				}
			}
		}


		res = wait_for_syscall(TOUT, &set);

		switch (res.retval) {
			case SIGNAL:
				printf("%d: SIGNAL\n", res.pid);
				break;
			case TIMEOUT:
			printf("TO\n");
				break;
			case ERROR:
				return 0;
			case DEAD:
				printf("%d: DEAD\n", res.pid);
				del(&set, res.pid);
				break;
			case ALL_DEAD:
				printf("ALL DEAD\n");
				return 0;
			case GROUPSTOP:
				printf("%d: GROUPSTOP\n", res.pid);
				break;
			case FORK:
				if (res.pid == 0) {
					break; // this should maybe restart infide wait_for_syscall()
				}
				printf("FORK %d\n", res.pid);
				struct pid_inf p_inf;
				p_inf.pid = res.pid;
				p_inf.in_syscall = 0;
				add(&set, p_inf);
				break;
			case SYSCALL:


				i = find_pid(&set, res.pid);
				if (i == -1) {
					printf("Unexpected pid: %d\n", res.pid);
					break;
				}

				if (res.registers.orig_rax == -1) {
					res.registers.rax = -EPERM;
					u_set_regs(res.pid, &res.registers);
				}

				if ((res.registers.orig_rax == SYSCALL_OPENAT)) {
					if (openat_handler(set.arr[i].pid, &res.registers, set.arr[i].in_syscall) == -1)
						printf("%d: openat_handler failed\n", set.arr[i].pid);
				} 

				if ((res.registers.orig_rax == SYSCALL_OPEN)) {
					if (open_handler(set.arr[i].pid, &res.registers, set.arr[i].in_syscall) == -1)
						printf("%d: open_handler failed\n", set.arr[i].pid);
				}

				if (set.arr[i].in_syscall == 0) {
					set.arr[i].in_syscall = 1;
					printf("%d: Syscall %lld start\n", res.pid, res.registers.orig_rax);
				}
				else {
					set.arr[i].in_syscall = 0;
					printf("%d: Syscall %lld = %lld\n", res.pid, res.registers.orig_rax, res.registers.rax);
				}

				u_continue_after_syscall(res.pid);
				break;
			default:
				printf("Strange res.retval=%d\n", res.retval);
				return 0;
		}

	}
}