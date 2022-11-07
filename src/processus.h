#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <stdbool.h>
#include <sys/wait.h> /*waitpid import */ //todo mettre dans .c la struct

/* Struct is a linked list of processes  */

/* Add a process to the list of processes.  */
void add_process(struct process **process_list, char *name, pid_t pid, char **argv);

/* Remove the process to the linked list */
void remove_process(struct process **process_list, pid_t pid);

/* Print the list of processes */
void print_process_list(struct process **process_list);

/* Check if the process is finished.  */
bool is_pid_done(pid_t pid);

/* Update linked list: check if processes are done and remove them */
void update_process_list(struct process **process_list);