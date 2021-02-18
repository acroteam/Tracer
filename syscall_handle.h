#pragma once

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/user.h>


union machine_word {
	size_t num;
	char buf[sizeof(size_t)];
};



int get_strlen_arg(pid_t pid, unsigned long long int tracee_ptr);
int get_str(pid_t pid, unsigned long long int tracee_ptr, char* str);
int open_handler(pid_t pid, struct user_regs_struct* regs, int in_syscall);
int openat_handler(pid_t pid, struct user_regs_struct* regs, int in_syscall);