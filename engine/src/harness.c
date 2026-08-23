#include "bro.h"
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

/*
** One fork() per case (design/02_ENGINE.md, decision B1).
**
** The reference implementation of these specs ran every case in one process:
** the first segfault in student code ended the run and produced no report at
** all, which is why SPEC_FRONTEND could promise SIGSEGV and TIMEOUT statuses
** its own architecture could not deliver.
**
** Three details the design doc did not spell out, all of which produce bugs
** that only appear under load (plan gap 2):
**   - every blocking call retries on EINTR; SIGCHLD arrives mid-read otherwise
**   - the capture pipes are drained BEFORE waitpid, or a child that fills a
**     64 KB pipe blocks forever while the parent waits for it to exit
**   - a short read of the result record is a malformed result, never a pass
*/

# define CHILD_STACK_SLACK_MS	500

static ssize_t	read_retry(int fd, void *buf, size_t n)
{
	ssize_t	r;

	while (1)
	{
		r = read(fd, buf, n);
		if (r >= 0 || errno != EINTR)
			return (r);
	}
}

static ssize_t	write_retry(int fd, const void *buf, size_t n)
{
	ssize_t	w;

	while (1)
	{
		w = write(fd, buf, n);
		if (w >= 0 || errno != EINTR)
			return (w);
	}
}

static pid_t	waitpid_retry(pid_t pid, int *st)
{
	pid_t	r;

	while (1)
	{
		r = waitpid(pid, st, 0);
		if (r >= 0 || errno != EINTR)
			return (r);
	}
}

static void	apply_limits(const t_limits *lim)
{
	struct rlimit		rl;
	struct itimerval	it;

	rl.rlim_cur = lim->address_space;
	rl.rlim_max = lim->address_space;
	setrlimit(RLIMIT_AS, &rl);
	rl.rlim_cur = 0;
	rl.rlim_max = 0;
	setrlimit(RLIMIT_CORE, &rl);
	memset(&it, 0, sizeof(it));
	it.it_value.tv_sec = lim->timeout_ms / 1000;
	it.it_value.tv_usec = (lim->timeout_ms % 1000) * 1000;
	setitimer(ITIMER_REAL, &it, NULL);
}

static void	init_result(t_result *r, const t_case *c, long fail_after)
{
	memset(r, 0, sizeof(*r));
	r->status = BRO_OK;
	r->kind = EV_NONE;
	r->diverge = -1;
	r->write_extent = -1;
	r->fail_after = fail_after;
	r->alloc.fail_after = fail_after;
	if (c->flags & BRO_UB_CASE)
		r->status = BRO_UB;
}

/*
** Under injection the case's own assertions are meaningless - the function was
** made to fail - so they are discarded and two different properties checked:
** the result must be NULL (returning a partly-built structure after a failed
** malloc is a bug the leak count alone would not catch) and nothing may be
** left live.
*/
static void	check_injected(t_ctx *ctx, t_result *r)
{
	if (r->alloc.live_bytes > 0)
	{
		r->status = BRO_LEAK;
		snprintf(r->msg, sizeof(r->msg),
			"malloc #%ld forced to fail: %zu byte(s) in %zu block(s) "
			"were never freed", r->fail_after - (long)r->alloc.base + 1,
			r->alloc.live_bytes, r->alloc.live_blocks);
		return ;
	}
	if (ctx->returned)
	{
		r->status = BRO_KO;
		snprintf(r->msg, sizeof(r->msg),
			"malloc #%ld forced to fail, but a non-NULL result came back",
			r->fail_after - (long)r->alloc.base + 1);
		return ;
	}
	r->status = BRO_OK;
}

static void	check_leak(t_result *r)
{
	if (r->status != BRO_OK || r->alloc.live_bytes == 0)
		return ;
	r->status = BRO_LEAK;
	snprintf(r->msg, sizeof(r->msg), "%zu byte(s) in %zu block(s) never freed",
		r->alloc.live_bytes, r->alloc.live_blocks);
}

static void	run_inline(const t_case *c, t_result *r)
{
	t_ctx	ctx;

	memset(&ctx, 0, sizeof(ctx));
	ctx.out = r;
	ctx.flags = c->flags;
	ctx.cap_fd[0] = -1;
	ctx.cap_fd[1] = -1;
	ctx.cap_saved[0] = -1;
	ctx.cap_saved[1] = -1;
	bro_del_reset();
	if (c->flags & BRO_INJECT)
		bro_alloc_arm(r->fail_after);
	c->run(&ctx);
	bro_alloc_snapshot(&r->alloc);
	bro_alloc_disarm();
	if (r->status == BRO_OK || r->status == BRO_UB)
	{
		bro_guard_check(&ctx.dst, r);
		bro_guard_check(&ctx.src, r);
	}
	if (c->flags & BRO_INJECT)
	{
		if (r->fail_after >= 0)
			check_injected(&ctx, r);
		else
			check_leak(r);
	}
	bro_ctx_reset(&ctx);
}

static void	child(const t_case *c, const t_limits *lim, int result_fd,
		int cap[2], long fail_after)
{
	t_result	r;

	apply_limits(lim);
	dup2(cap[0], STDOUT_FILENO);
	dup2(cap[1], STDERR_FILENO);
	close(cap[0]);
	close(cap[1]);
	init_result(&r, c, fail_after);
	run_inline(c, &r);
	write_retry(result_fd, &r, sizeof(r));
	close(result_fd);
	_exit(0);
}

/*
** Drain every pipe to EOF before reaping. Returns false if the child outlived
** its own alarm by the slack margin, which means the alarm never fired and the
** parent must kill it rather than block.
*/
static bool	drain(int fds[3], char *cap, size_t caplen, size_t *caplen_used,
		unsigned timeout_ms)
{
	struct pollfd	p[3];
	char		buf[4096];
	ssize_t		n;
	int		i;
	int		open_count;

	open_count = 3;
	i = 0;
	while (i < 3)
	{
		p[i].fd = fds[i];
		p[i].events = POLLIN;
		i++;
	}
	while (open_count > 0)
	{
		if (poll(p, 3, (int)timeout_ms) <= 0)
		{
			if (errno == EINTR)
				continue ;
			return (false);
		}
		i = -1;
		while (++i < 3)
		{
			if (p[i].fd < 0 || !(p[i].revents & (POLLIN | POLLHUP)))
				continue ;
			n = read_retry(p[i].fd, buf, sizeof(buf));
			if (n <= 0)
			{
				p[i].fd = -1;
				p[i].events = 0;
				open_count--;
				continue ;
			}
			if (i == 0 && *caplen_used < sizeof(t_result))
			{
				if ((size_t)n > sizeof(t_result) - *caplen_used)
					n = (ssize_t)(sizeof(t_result) - *caplen_used);
				memcpy(cap + *caplen_used, buf, (size_t)n);
				*caplen_used += (size_t)n;
			}
			(void)caplen;
		}
	}
	return (true);
}

/*
** A UB-flagged case that CRASHES is still not scored.
**
** Decision A5 says an undefined input is never scored, and a crash is simply
** what this implementation does with it - ft_striteri on a string literal
** faults on every platform 42 uses, and failing a student for that would be
** scoring them against behaviour the standard does not define. The signal is
** still reported, because what it does is the thing worth knowing.
*/
static void	classify_signal(t_result *r, int sig, unsigned flags)
{
	if (sig == SIGSEGV)
		r->status = BRO_SIGSEGV;
	else if (sig == SIGBUS)
		r->status = BRO_SIGBUS;
	else if (sig == SIGABRT)
		r->status = BRO_SIGABRT;
	else if (sig == SIGALRM || sig == SIGKILL)
		r->status = BRO_TIMEOUT;
	else
		r->status = BRO_KO;
	if (r->status == BRO_TIMEOUT)
		snprintf(r->msg, sizeof(r->msg),
			"did not finish - killed after the time limit");
	else
		snprintf(r->msg, sizeof(r->msg), "crashed with %s",
			strsignal(sig));
	if ((flags & BRO_UB_CASE) && r->status != BRO_TIMEOUT)
	{
		r->status = BRO_UB;
		snprintf(r->msg, sizeof(r->msg),
			"crashed with %s - this input is undefined, so it is not scored",
			strsignal(sig));
	}
}

static int	reap(pid_t pid, bool drained, t_result *r, size_t got,
		unsigned flags)
{
	int	st;

	if (!drained)
		kill(pid, SIGKILL);
	if (waitpid_retry(pid, &st) < 0)
		return (-1);
	if (WIFSIGNALED(st))
	{
		memset(r, 0, sizeof(*r));
		r->diverge = -1;
		r->write_extent = -1;
		classify_signal(r, WTERMSIG(st), flags);
		return (0);
	}
	if (got != sizeof(t_result))
	{
		memset(r, 0, sizeof(*r));
		r->diverge = -1;
		r->write_extent = -1;
		r->status = BRO_KO;
		snprintf(r->msg, sizeof(r->msg),
			"the case exited without reporting a result");
	}
	return (0);
}

static int	run_once(const t_suite *s, const t_case *c, const t_limits *lim,
		long fail_after, t_result *out)
{
	int	res[2];
	int	so[2];
	int	se[2];
	pid_t	pid;
	size_t	got;
	bool	ok;
	int	fds[3];

	(void)s;
	if (!lim->fork)
	{
		init_result(out, c, fail_after);
		run_inline(c, out);
		return (0);
	}
	if (pipe(res) < 0 || pipe(so) < 0 || pipe(se) < 0)
		return (-1);
	pid = fork();
	if (pid < 0)
		return (-1);
	if (pid == 0)
	{
		close(res[0]);
		close(so[0]);
		close(se[0]);
		child(c, lim, res[1], (int [2]){so[1], se[1]}, fail_after);
	}
	close(res[1]);
	close(so[1]);
	close(se[1]);
	got = 0;
	fds[0] = res[0];
	fds[1] = so[0];
	fds[2] = se[0];
	ok = drain(fds, (char *)out, sizeof(*out), &got,
			lim->timeout_ms + CHILD_STACK_SLACK_MS);
	close(res[0]);
	close(so[0]);
	close(se[0]);
	return (reap(pid, ok, out, got, c->flags));
}

/*
** libft-01-cases.md's allocation-failure section asks for malloc_fail_after(k) on ten
** functions. Rather than hand-writing ten injection tests, the sweep derives
** them: run once to learn how many allocations the case makes, then re-run it
** once per allocation index with malloc failing exactly there - each in a
** fresh child, so nothing carries over.
**
** Every k is reported individually, which localises the bug: "you handle the
** first two failures and not the third" is a far better clue than "you leak".
*/
static int	inject_sweep(const t_suite *s, const t_case *c,
		const t_limits *lim, t_result *out)
{
	t_result	run;
	size_t	n;
	size_t	k;

	n = out->alloc.calls - out->alloc.base;
	if (n > BRO_INJECT_MAX)
		n = BRO_INJECT_MAX;
	out->rollback_n = n;
	k = 0;
	while (k < n)
	{
		/* absolute index, so the fixture's own allocations (1..base) are
		** never the ones made to fail */
		if (run_once(s, c, lim, (long)(out->alloc.base + k), &run) < 0)
			return (-1);
		out->rollback[k] = (run.status == BRO_OK);
		if (!out->rollback[k] && out->status == BRO_OK)
		{
			out->status = run.status;
			memcpy(out->msg, run.msg, sizeof(out->msg));
			out->alloc = run.alloc;
			out->fail_after = (long)k;
		}
		k++;
	}
	return (0);
}

int	bro_run_case(const t_suite *s, const t_case *c, const t_limits *lim,
		t_result *out)
{
	if (run_once(s, c, lim, -1, out) < 0)
		return (-1);
	if (!(c->flags & BRO_INJECT) || !bro_alloc_available())
		return (0);
	if (out->status != BRO_OK
		|| out->alloc.calls == out->alloc.base)
		return (0);
	return (inject_sweep(s, c, lim, out));
}
