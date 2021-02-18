#pragma once

#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/reg.h>

enum retval {
	ERROR,
	SYSCALL,
	DEAD,
	ALL_DEAD,
	SIGNAL,
	GROUPSTOP,
	FORK,
	TIMEOUT
};

struct ret_sys_info {
	pid_t pid;
	struct user_regs_struct registers;
	int retval;
};

int config_attach(pid_t pid);
int attach_process(pid_t pid);
struct ret_sys_info wait_for_syscall();

int u_continue_after_syscall(pid_t pid);

int u_set_regs(pid_t pid, struct user_regs_struct* regs);

int u_get_regs(pid_t pid, struct user_regs_struct* regs);
