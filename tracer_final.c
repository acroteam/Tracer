#include <sys/ptrace.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>

#include "tracer_final.h"
#include "set.h"

void alrm_handler(int signm) 
{}

int config_attach(pid_t pid)
{
	if (ptrace(PTRACE_INTERRUPT, pid, NULL, NULL) == -1) {
		return -1;
	}
	int status;
	int res = 0;
	while (1) {
		res = waitpid(pid, &status, __WALL);
		if (res == -1 && errno == EINTR) 
			continue;
		else if (res == -1) {
			errno = ESRCH;
			return -1; // process was attached but died before wait_for_syscall()
		}

		if (WSTOPSIG(status) == SIGTRAP | PTRACE_EVENT_STOP << 8) {
			if (ptrace(PTRACE_SETOPTIONS, pid, NULL, PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEFORK) == -1 || ptrace(PTRACE_SYSCALL, pid, NULL, 0) == -1)
				return -1;
			break;
		}

		ptrace(PTRACE_CONT, pid, 0, WSTOPSIG(status)); // inject all signals (which was concurrently sent) untill catching PTRACE_EVENT_STOP
	}
	return 0;
}



int attach_process(pid_t pid) 
{
	if (ptrace(PTRACE_SEIZE, pid, 0, 0) == -1) {
		return -1;
	}
	
	return config_attach(pid);
}



struct ret_sys_info wait_for_syscall(int sec, Set *pid_set)
{
	static int x = 0;
	struct ret_sys_info ret;

	struct sigaction act, old_act;
	act.sa_handler = alrm_handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGALRM, &act, &old_act) == -1) {
		ret.pid = 0;
		ret.retval = ERROR;
		goto exit;
	}



	struct itimerval s_it;	
	s_it.it_interval.tv_sec = sec;
	s_it.it_interval.tv_usec = 0;
	s_it.it_value.tv_sec = sec;
	s_it.it_value.tv_usec = 0;

	struct user_regs_struct regs;

	int status = 0;
	setitimer(ITIMER_REAL, &s_it, NULL); 
	pid_t pid = waitpid(-1, &status, __WALL);

	if (pid == -1 && errno == EINTR) {
		ret.retval = TIMEOUT;
		goto exit;
	}

	if (pid == -1 && errno == ECHILD) {
		ret.retval = ALL_DEAD;
		goto exit;
	}

	if (pid == -1) {
		ret.retval = ERROR;
		goto exit;
	}


	if (WIFEXITED(status)){ // tracee unexpectively died
		ret.pid = pid;
		ret.retval = DEAD;
		goto exit;
	}

	if (!WIFSTOPPED(status)) {
		ret.retval = ERROR;
		return ret;
	}




	if (WSTOPSIG(status) == (SIGTRAP | 0x80)) { // We are sure this is a syscall

		if (ptrace(PTRACE_GETREGS, pid, NULL, &regs) == -1){
			ret.retval = ERROR;
		}
		else 
			ret.retval = SYSCALL;

		ret.pid = pid;
		ret.registers = regs;

		goto exit;
	}
	if (status >> 8 == SIGTRAP | (PTRACE_EVENT_FORK << 8)) {
		ret.retval = FORK;
		if (ptrace(PTRACE_GETEVENTMSG, pid, NULL, &ret.pid) == -1) {
			ret.retval = ERROR;
			goto exit;
		};
		//printf("pid=%d ret.pid=%d\n", pid, ret.pid);
		if (find_pid(pid_set, pid) == -1) {
			ret.pid = 0;
			if (ptrace(PTRACE_SETOPTIONS, pid, NULL, PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEFORK) == -1 || ptrace(PTRACE_SYSCALL, pid, NULL, 0) == -1) {
				ret.pid = ERROR;
			}
			
			goto exit;
		}
		if (ptrace(PTRACE_SYSCALL, pid, NULL, 0) == -1) {
			ret.pid = ERROR;
		}
		goto exit;
	}

	if (status >> 8 == PTRACE_EVENT_STOP << 8) {
		ret.retval = GROUPSTOP;
		ret.pid = pid;
		if (WSTOPSIG(status) == SIGSTOP || WSTOPSIG(status) == SIGTSTP || WSTOPSIG(status) == SIGTTIN || WSTOPSIG(status) == SIGTTOU) {
			if (ptrace(PTRACE_LISTEN, pid, NULL, WSTOPSIG(status)) == -1) {
				ret.retval = ERROR;
				goto exit;
			}
				
		}
		else {
			if (ptrace(PTRACE_CONT, pid, NULL, WSTOPSIG(status)) == -1) {
				ret.retval = ERROR;
				goto exit;	
			}
		}
		goto exit;
	}

	// it is a signal

	siginfo_t sig_info;
	ptrace(PTRACE_GETSIGINFO, pid, NULL, &sig_info);
	ptrace(PTRACE_SYSCALL, pid, 0, sig_info.si_signo);
	ret.pid = pid;
	ret.retval = SIGNAL;

exit: 
	s_it.it_value.tv_sec = 0;
	s_it.it_value.tv_usec = 0;	
	setitimer(ITIMER_REAL, &s_it, NULL);

	sigaction(SIGALRM, &old_act, NULL); 

	return ret;
}

int u_continue_after_syscall(pid_t pid) {
	return ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
}

int u_set_regs(pid_t pid, struct user_regs_struct* regs) {
	return ptrace(PTRACE_SETREGS, pid, NULL, regs);
}

int u_get_regs(pid_t pid, struct user_regs_struct* regs) {
	return ptrace(PTRACE_GETREGS, pid, NULL, regs);
}
