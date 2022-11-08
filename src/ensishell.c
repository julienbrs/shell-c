/*****************************************************
 * Copyright Grégory Mounié 2008-2015                *
 *           Simon Nieuviarts 2002-2009              *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "variante.h"
#include "readcmd.h"
#include "processus.h"

#ifndef VARIANTE
#error "Variante non défini !!"
#endif

/* Guile (1.8 and 2.0) is auto-detected by cmake */
/* To disable Scheme interpreter (Guile support), comment the
 * following lines.  You may also have to comment related pkg-config
 * lines in CMakeLists.txt.
 */

#if USE_GUILE == 1
#include <libguile.h>

int question6_executer(char *line)
{
	/* Question 6: Insert your code to execute the command line
	 * identically to the standard execution scheme:
	 * parsecmd, then fork+execvp, for a single command.
	 * pipe and i/o redirection are not required.
	 */
	printf("Not implemented yet: can not execute %s\n", line);

	/* Remove this line when using parsecmd as it will free it */
	free(line);

	return 0;
}

SCM executer_wrapper(SCM x)
{
	return scm_from_int(question6_executer(scm_to_locale_stringn(x, 0)));
}
#endif

void exec_cmd(struct cmdline *l, struct process **process_list)
{
	pid_t pid = fork();
	switch (pid)
	{
	case -1:
		perror("fork:");
		break;
	case 0:

		if (strcmp(l->seq[0][0], "jobs") == 0)
		{
			print_process_list(process_list);
			exit(0);
		}
		else
		{
			execvp(l->seq[0][0], l->seq[0]);
		}
		break;
	default:
		update_process_list(process_list);
		if (l->bg)
		{
			add_process(process_list, l->seq[0][0], pid);
			waitpid(pid, NULL, WNOHANG);
		}
		else
		{
			waitpid(pid, NULL, 0);
		}
		break;
	}
}

void exec_pipe(struct cmdline *l) {
	char **arg1 = l->seq[0];
	char **arg2 = l->seq[1];
	int tuyau[2];
	pipe(tuyau);
	pid_t pid_1 = fork();
	if (pid_1 == 0)
	{ // si on est dans le fils
		pid_t pid_2 = fork();
		if (pid_2 == 0)
		{ // si on est dans le fils
			dup2(tuyau[0], 0);
			close(tuyau[1]);
			close(tuyau[0]);
			execvp(arg2[0], arg2);
		}
		else
		{
			dup2(tuyau[1], 1); // ecriture de stdout dans le tuyau
			close(tuyau[0]);
			close(tuyau[1]);
			execvp(arg1[0], arg1); //
		}
	}
	else
	{
		waitpid(pid_1, NULL, 0);
	}
}

void terminate(char *line)
{
#if USE_GNU_READLINE == 1
	/* rl_clear_history() does not exist yet in centOS 6 */
	clear_history();
#endif
	if (line)
		free(line);
	printf("exit\n");
	exit(0);
}

int main()
{
	printf("Variante %d: %s\n", VARIANTE, VARIANTE_STRING);
	/*create list of process*/
	struct process **process_list = malloc(sizeof(struct process *));
	*process_list = NULL;

#if USE_GUILE == 1
	scm_init_guile();
	/* register "executer" function in scheme */
	scm_c_define_gsubr("executer", 1, 0, 0, executer_wrapper);
#endif

	while (1)
	{
		struct cmdline *l;
		char *line = 0;
		// int i, j;
		char *prompt = "ensishell>";

		/* Readline use some internal memory structure that
		   can not be cleaned at the end of the program. Thus
		   one memory leak per command seems unavoidable yet */
		line = readline(prompt);
		if (line == 0 || !strncmp(line, "exit", 4))
		{
			terminate(line);
		}

#if USE_GNU_READLINE == 1
		add_history(line);
#endif

#if USE_GUILE == 1
		/* The line is a scheme command */
		if (line[0] == '(')
		{
			char catchligne[strlen(line) + 256];
			sprintf(catchligne, "(catch #t (lambda () %s) (lambda (key . parameters) (display \"mauvaise expression/bug en scheme\n\")))", line);
			scm_eval_string(scm_from_locale_string(catchligne));
			free(line);
			continue;
		}
#endif

		/* parsecmd free line and set it up to 0 */
		l = parsecmd(&line);

		/* If input stream closed, normal termination */
		if (!l)
		{

			terminate(0);
		}

		if (l->err)
		{
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}
		int fd_in = -1;
		int fd_out = -1;
		if (l->in) {
			//// ouvrir un descripteur vers l'entree-sortie
			int fd_in = open(l->in , O_RDONLY);
			if (fd_in == -1) { perror("open: " ); exit(EXIT_FAILURE);}
			//// fermer le descripteur standard et dupliquer
			//// le descripteur ouvert dans le descripteur standard
			dup2(fd_in, STDIN_FILENO); // STDIN_FILENO == 0
			//// fermer le descripteur ouvert en double
			//close(fd);
		}
		if (l->out) {
			// ouvrir un descripteur vers l'entree-sortie
			fd_out = open(l->out , O_RDWR);
			if (fd_out == -1) { perror("open: " ); exit(EXIT_FAILURE);}
			// fermer le descripteur standard et dupliquer
			// le descripteur ouvert dans le descripteur standard
			dup2(fd_out, STDOUT_FILENO); // STDOUT_FILENO == 0
			// fermer le descripteur ouvert en double
		}
		if (l->bg)
			printf("background (&)\n");

		if (l->seq[1] != NULL)
		{
			exec_pipe(l);
		}
		else if (l->seq[0] != NULL)
		{
			exec_cmd(l, process_list);
		}
		if (l->out) close(fd_out);
		if (l->in) close(fd_in);
	}
}
