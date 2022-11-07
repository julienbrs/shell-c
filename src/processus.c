
#include "processus.h"

/* Add a process to the list of processes.  */
void add_process(struct process **process_list, char *name, pid_t pid, char **argv)
{
  struct process *new_process = malloc(sizeof(struct process));
  new_process->name = name;
  new_process->pid = pid;
  new_process->argv = argv;
  new_process->suspended = false;
  new_process->next = *process_list;
  *process_list = new_process;
}

/* Remove the process to the linked list */
void remove_process(struct process **process_list, pid_t pid)
{
  struct process *current = *process_list;
  struct process *previous = NULL;
  while (current != NULL)
  {
    if (current->pid == pid)
    { // we found the pid
      if (previous == NULL)
      {
        *process_list = current->next;
      }
      else
      {
        previous->next = current->next;
      }
      free(current);
      return;
    }
    previous = current;
    current = current->next;
  }
}

/* Print the list of processes */
void print_process_list(struct process **process_list)
{
  printf("in print_process fucntion");
  struct process *current = *process_list;
  while (current != NULL)
  {
    printf("name: %s, pid: %d, argv: %s, suspended: %d)\n", current->name, current->pid, current->argv[0], current->suspended);
    current = current->next;
  }
}

/* Check if the process is finished.  */
bool is_pid_done(pid_t pid)
{
  int status; // if status not NULL, waitpid() will store status information of the process here
  while (true)
  {
    pid_t result = waitpid(pid, &status, WNOHANG);
    if (result == -1 && errno == EINTR)
    { // error, signal intercepted, we just try again
      continue;
    }
    else if (result == -1 && errno == ECHILD)
    { // error, invalid pid requested or already stated done
      fprintf(stderr, "Value of errno: %d\n", errno);
      fprintf(stderr, "Pid %d invalid . Process already stated done earlier or unrelated pid", pid);
    }
    else if (result == 0)
    { // process is still running atm
      return false;
    }
    else if (result == pid)
    {
      return true;
    }
  }
}

/* Update linked list: check if processes are done and remove them */
void update_process_list(struct process **process_list)
{
  struct process *current = *process_list;
  while (current != NULL)
  {
    if (is_pid_done(current->pid))
    {
      remove_process(process_list, current->pid);
    }
    current = current->next;
  }
}
