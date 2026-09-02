#include "bro.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
** bro_micro - argv, filters, run loop. Emits NDJSON on stdout and nothing
** else, so `bro --json | jq` works with no filtering.
*/

typedef struct s_opts
{
	char		*only_fn;
	int		only_id;
	const char	*skip;
	t_limits	lim;
}	t_opts;

static void	usage(void)
{
	fprintf(stderr,
		"usage: bro_micro [--only fn[:id]] [--no-fork] [--timeout MS]\n");
}

static int	parse_only(t_opts *o, const char *arg)
{
	char	*colon;
	char	*buf;

	buf = strdup(arg);
	if (!buf)
		return (-1);
	colon = strchr(buf, ':');
	if (colon)
	{
		*colon = '\0';
		o->only_id = atoi(colon + 1);
	}
	o->only_fn = buf;
	return (0);
}

static int	parse_args(int ac, char **av, t_opts *o)
{
	int	i;

	i = 0;
	while (++i < ac)
	{
		if (!strcmp(av[i], "--only") && i + 1 < ac)
		{
			if (parse_only(o, av[++i]) < 0)
				return (-1);
		}
		else if (!strcmp(av[i], "--skip") && i + 1 < ac)
			o->skip = av[++i];
		else if (!strcmp(av[i], "--no-fork"))
			o->lim.fork = false;
		else if (!strcmp(av[i], "--timeout") && i + 1 < ac)
			o->lim.timeout_ms = (unsigned)atoi(av[++i]);
		else
			return (usage(), -1);
	}
	return (0);
}

static bool	selected(const t_opts *o, const t_suite *s, const t_case *c);

/*
** A function that is not in the student's archive was stubbed only so the link
** would succeed - the stub is never CALLED. Running it would produce whatever
** the stub returns, which is exactly the collision the reference build's -999
** sentinel had: ft_atoi("-999") legitimately returns -999.
*/
static bool	is_skipped(const t_opts *o, const t_suite *s)
{
	const char	*p;
	size_t		len;

	if (!o->skip)
		return (false);
	len = strlen(s->fn);
	p = o->skip;
	while ((p = strstr(p, s->fn)))
	{
		if ((p == o->skip || p[-1] == ',')
			&& (p[len] == '\0' || p[len] == ','))
			return (true);
		p += len;
	}
	return (false);
}

static void	emit_missing(const t_suite *s, const t_opts *o)
{
	t_result	r;
	size_t		i;

	i = 0;
	while (i < s->count)
	{
		if (selected(o, s, &s->cases[i]))
		{
			memset(&r, 0, sizeof(r));
			r.status = BRO_MISSING;
			r.diverge = -1;
			r.write_extent = -1;
			snprintf(r.msg, sizeof(r.msg), "%s is not in the archive yet",
				s->fn);
			bro_emit(s, &s->cases[i], &r, 0.0);
		}
		i++;
	}
}

static bool	selected(const t_opts *o, const t_suite *s, const t_case *c)
{
	if (o->only_fn && strcmp(o->only_fn, s->fn))
		return (false);
	if (o->only_id && c->id != o->only_id)
		return (false);
	return (true);
}

static double	elapsed_ms(struct timespec a, struct timespec b)
{
	return ((b.tv_sec - a.tv_sec) * 1000.0
		+ (b.tv_nsec - a.tv_nsec) / 1000000.0);
}

static int	run_suite(const t_suite *s, const t_opts *o, int *failures)
{
	struct timespec	t0;
	struct timespec	t1;
	t_result	r;
	size_t		i;

	i = 0;
	while (i < s->count)
	{
		if (!selected(o, s, &s->cases[i]))
		{
			i++;
			continue ;
		}
		clock_gettime(CLOCK_MONOTONIC, &t0);
		if (bro_run_case(s, &s->cases[i], &o->lim, &r) < 0)
			return (-1);
		clock_gettime(CLOCK_MONOTONIC, &t1);
		bro_emit(s, &s->cases[i], &r, elapsed_ms(t0, t1));
		if (bro_status_is_scored(r.status) && r.status != BRO_OK)
			(*failures)++;
		i++;
	}
	return (0);
}

static int	run_all(t_opts *o, int *failures)
{
	size_t	i;

	i = 0;
	while (i < g_suite_count)
	{
		if (is_skipped(o, g_suites[i]))
			emit_missing(g_suites[i], o);
		else if (run_suite(g_suites[i], o, failures) < 0)
			return (-1);
		i++;
	}
	return (0);
}

/*
** parse_only() strdup()s the selector, so main owns it. Freeing it is not
** hygiene theatre: tools/selftest.py runs this binary under valgrind and
** refuses to trust a tester that leaks, so an 11-byte leak here fails the
** gate exactly as loudly as a leak in a student's ft_split.
*/
int	main(int ac, char **av)
{
	t_opts	o;
	int	failures;
	int	rc;

	memset(&o, 0, sizeof(o));
	o.lim.timeout_ms = 2000;
	o.lim.address_space = 512UL * 1024 * 1024;
	o.lim.fork = true;
	if (parse_args(ac, av, &o) < 0)
		return (2);
	failures = 0;
	rc = run_all(&o, &failures);
	free(o.only_fn);
	if (rc < 0)
	{
		fprintf(stderr, "bro_micro: harness failure\n");
		return (2);
	}
	if (failures > 125)
		return (125);
	return (failures);
}
