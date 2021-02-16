#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ptrace.h>


#include "syscall_handle.h"


/*
tracee_ptr is tracees char pointer, functions get_strlen_arg() and get_str() help to work with this pointer

*/

int open_cb(char* path, int flags) {
	return 0;
}

int get_strlen_arg(pid_t pid, unsigned long long int tracee_ptr)  
{
	int n = 0;
	char* ptr = NULL;
	union machine_word word;
	if (tracee_ptr == 0) 
		return -1;
	while(1) {
		if (word.num = ptrace(PTRACE_PEEKDATA, pid, tracee_ptr + n, NULL) == -1)
			return -1;
		if (ptr = memchr(word.buf, '\0', sizeof(size_t))) {
			n += ptr - word.buf; 
			break;
		}
		n += sizeof(size_t);
	}
	return n;
}

int get_str(pid_t pid, unsigned long long int tracee_ptr, char* str)
{
	int n = 0;
	char* ptr = NULL;
	union machine_word word;
	if (tracee_ptr == 0) 
		return -1;
	while(1) {
		if (word.num = ptrace(PTRACE_PEEKDATA, pid, tracee_ptr + n, NULL) == -1)
			return -1;
		if (ptr = memchr(word.buf, '\0', sizeof(size_t))) {
			memcpy(str, word.buf, ptr - word.buf + 1); 
			break;
		}
		n += sizeof(size_t);
		memcpy(str, word.buf, sizeof(size_t));
	}
	return 0;
}


int open_handler(pid_t pid, struct user_regs_struct* regs, int in_syscall) 
{
	int n = get_strlen_arg(pid, regs->rdi);
	if (n < 0)
		return 0; 

	char* path = malloc(n + 1);
	if (get_str(pid, regs->rdi, path) < 0)
		return 0;

	int flags = regs->rsi;

	if (!open_cb(path, flags)) {
		if (in_syscall == 0)
			regs->orig_rax = -1; //entry
	}
	ptrace(PTRACE_SETREGS, pid, NULL, regs);

	free(path);
	return 0;
}

int openat_handler(pid_t pid, struct user_regs_struct* regs, int in_syscall) 
{
	int n = get_strlen_arg(pid, regs->rsi);
	if (n < 0)
		return 0; 

	char* path = malloc(n + 1);
	if (get_str(pid, regs->rsi, path) < 0)
		return 0;

	//printf("PATH: %s\n", path);
	int flags = regs->rdx;

	if (!open_cb(path, flags)) {
		if (in_syscall == 0)
			regs->orig_rax = -1; //entry
	}

	ptrace(PTRACE_SETREGS, pid, NULL, regs);

	free(path);
	return 0;
}