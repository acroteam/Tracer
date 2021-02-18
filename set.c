#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#include "set.h"



struct Set init_set(unsigned int len)
{ 
	len = (len == 0) ? DEFAULT_LEN : len;
	struct pid_inf *ptr = malloc(len * sizeof(struct pid_inf));
	struct Set new_set;
	new_set.arr = ptr;
	new_set.max_len = len;
	new_set.len = 0;
	return new_set;
}

void delete_set(struct Set *set)
{
	return free(set->arr);
}

int s_set_len(struct Set *set, unsigned int new_len)
{
	struct pid_inf* new_arr = malloc(sizeof(struct pid_inf) * new_len);
	if (!new_arr)
		return -1;

	memcpy(new_arr, set->arr, sizeof(struct pid_inf) * (set->len + 1));

	struct pid_inf *tmp = set->arr;
	set->arr = new_arr;
	set->max_len = new_len;
	free(tmp);
	return 0;
}

int find_pid(struct Set *set, pid_t pid)
{
	int i = 0;
	for (i = 0; i < set->len; ++i) {
		if (set->arr[i].pid == pid)
			return i;
	}
	return -1;
}

int add(struct Set *set, struct pid_inf item) 
{
	if (find_pid(set, item.pid) != -1)
		return 0;

	if (set->len == set->max_len - 1) {
		if (s_set_len(set, set->max_len * 2))
			return -1;
	}
	set->arr[set->len] = item;
	set->len++;

	return 0;
}

int del(struct Set *set, pid_t pid)
{
	int i = find_pid(set, pid);

	if (i == -1)
		return 0;
	set->arr[i] = set->arr[set->len-1];
	set->len--;

	if (set->len * 3 < set->max_len) {
		if (s_set_len(set, set->max_len / 2)) {
			return -1;
		}
	}
	return 0;
}