#include "tester.h"

static int s_pipe_write_fd = -1;

static void crash_handler(int sig)
{
	if (s_pipe_write_fd >= 0)
	{
		if (g_func_not_found)
			g_current_ipc.status = TEST_NOT_FOUND;
		else
		{
			if (sig == SIGSEGV)
				g_current_ipc.status = TEST_SIGSEGV;
			else if (sig == SIGBUS)
				g_current_ipc.status = TEST_SIGBUS;
			else if (sig == SIGFPE)
				g_current_ipc.status = TEST_SIGFPE;
			else
				g_current_ipc.status = TEST_SIGABRT;
			g_current_ipc.signal_num = sig;
		}
		write(s_pipe_write_fd, &g_current_ipc, sizeof(t_test_ipc));
		close(s_pipe_write_fd);
	}
	_exit(128 + sig);
}

void	run_isolated_test(void (*test_fn)(void), t_test_case *tc)
{
	int pipe_fd[2];
	if (pipe(pipe_fd) == -1)
	{
		tc->status = TEST_KO;
		return ;
	}

	pid_t pid = fork();
	if (pid < 0)
	{
		close(pipe_fd[0]);
		close(pipe_fd[1]);
		tc->status = TEST_KO;
		return ;
	}

	if (pid == 0)
	{
		/* Child process */
		close(pipe_fd[0]); /* Close read end */

		/* Set 2 second timeout guard against infinite loops */
		alarm(2);

		s_pipe_write_fd = pipe_fd[1];
		signal(SIGSEGV, crash_handler);
		signal(SIGBUS, crash_handler);
		signal(SIGABRT, crash_handler);
		signal(SIGFPE, crash_handler);

		g_func_not_found = false;
		memset(&g_current_ipc, 0, sizeof(t_test_ipc));
		test_fn();

		if (g_func_not_found)
			g_current_ipc.status = TEST_NOT_FOUND;

		/* Write IPC packet back to parent */
		write(pipe_fd[1], &g_current_ipc, sizeof(t_test_ipc));
		close(pipe_fd[1]);
		exit(0);
	}

	/* Parent process */
	close(pipe_fd[1]); /* Close write end */

	int status;
	waitpid(pid, &status, 0);

	t_test_ipc ipc = {0};
	if (read(pipe_fd[0], &ipc, sizeof(t_test_ipc)) > 0)
	{
		tc->test_num = ipc.test_num;
		tc->status = ipc.status;
		tc->signal_num = ipc.signal_num;
		if (ipc.desc[0]) tc->desc = strdup(ipc.desc);
		if (ipc.input[0]) tc->input = strdup(ipc.input);
		if (ipc.expected[0]) tc->expected = strdup(ipc.expected);
		if (ipc.actual[0]) tc->actual = strdup(ipc.actual);
		if (ipc.hint[0]) tc->hint = strdup(ipc.hint);
		if (ipc.ref[0]) tc->ref = strdup(ipc.ref);
	}
	else if (WIFSIGNALED(status))
	{
		int sig = WTERMSIG(status);
		tc->signal_num = sig;
		if (sig == SIGALRM)
			tc->status = TEST_TIMEOUT;
		else if (sig == SIGSEGV)
			tc->status = TEST_SIGSEGV;
		else if (sig == SIGBUS)
			tc->status = TEST_SIGBUS;
		else if (sig == SIGFPE)
			tc->status = TEST_SIGFPE;
		else
			tc->status = TEST_SIGABRT;
	}

	close(pipe_fd[0]);
}

void	run_suite_tests(t_func_suite *s, void (**tests)(void))
{
	g_current_func_under_test = s->func_name;
	print_suite_header(s->func_name, s->part);
	for (int i = 0; i < s->test_count; i++)
	{
		run_isolated_test(tests[i], &s->cases[i]);
		if (s->cases[i].status == TEST_NOT_FOUND)
		{
			s->not_found = true;
			s->fail_count = s->test_count;
			print_not_found(s->func_name);
			return ;
		}
		if (s->cases[i].status == TEST_OK)
			s->pass_count++;
		else
			s->fail_count++;
		print_case_result(&s->cases[i]);
	}
	print_suite_summary(s);
}
