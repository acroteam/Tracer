#pragma once
#include <sys/types.h>
#define DEFAULT_LEN 50

struct pid_inf
{
	int in_syscall;
	pid_t pid;
};


struct Set
{
	struct pid_inf *arr;
	unsigned int max_len;
	unsigned int len;
};

typedef struct Set Set;



struct Set init_set(unsigned int len);
void delete_set(struct Set *set);

int s_set_len(struct Set *set, unsigned int new_len);
int find_pid(struct Set *set, pid_t pid);
int add(struct Set *set, struct pid_inf item);
int del(struct Set *set, pid_t pid);