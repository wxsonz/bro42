#include "bro.h"
#include "libft_proto.h"
#include <stdlib.h>
#include <string.h>

/*
** ft_strmapi - T2. Cases from _dev/SPEC_MICRO.md section 29.
**
** Case 5's callback returns '\0' for every character: the result is three
** NUL bytes plus a terminator. strcmp would see two empty strings and pass
** no matter what the buffer actually holds, so that case compares 4 raw
** bytes with memcmp instead.
*/

static int	g_add_calls;

static char	f_add(unsigned int i, char c)
{
	g_add_calls++;
	return (c + i);
}

static char	f_zero(unsigned int i, char c)
{
	(void)i;
	(void)c;
	return ('\0');
}

static unsigned int	g_seen[8];
static size_t		g_seen_n;

static char	f_record(unsigned int i, char c)
{
	if (g_seen_n < 8)
		g_seen[g_seen_n++] = i;
	return (c);
}

static void	case_01(t_ctx *c)
{
	char	*got;

	got = ft_strmapi("abc", f_add);
	bro_track(c, got);
	if (bro_injecting(c))
		return ;
	if (!got)
		return (bro_fail(c->out, "returned NULL"));
	if (strcmp(got, "ace"))
		bro_fail(c->out, "expected \"ace\", got \"%s\"", got);
	free(got);
}

static void	case_02(t_ctx *c)
{
	char	*got;

	g_add_calls = 0;
	got = ft_strmapi("", f_add);
	bro_track(c, got);
	if (bro_injecting(c))
		return ;
	if (!got)
		return (bro_fail(c->out, "returned NULL"));
	if (g_add_calls != 0)
	{
		bro_fail(c->out, "f was called %d time(s) on an empty string",
			g_add_calls);
		return (free(got));
	}
	if (strcmp(got, ""))
		bro_fail(c->out, "expected \"\", got \"%s\"", got);
	free(got);
}

static void	case_03(t_ctx *c)
{
	char	s[4];
	char	*got;

	strcpy(s, "abc");
	got = ft_strmapi(s, f_add);
	bro_track(c, got);
	if (bro_injecting(c))
		return ;
	if (!got)
		return (bro_fail(c->out, "returned NULL"));
	if (strcmp(s, "abc"))
		bro_fail(c->out, "s was modified: now \"%s\"", s);
	free(got);
}

static void	case_04(t_ctx *c)
{
	char	*got;
	size_t	i;

	g_seen_n = 0;
	got = ft_strmapi("abcde", f_record);
	bro_track(c, got);
	if (bro_injecting(c))
		return ;
	if (!got)
		return (bro_fail(c->out, "returned NULL"));
	if (g_seen_n != 5)
	{
		bro_fail(c->out, "f was called %zu time(s), expected 5", g_seen_n);
		return (free(got));
	}
	i = 0;
	while (i < 5 && g_seen[i] == i)
		i++;
	if (i < 5)
		bro_fail(c->out, "call %zu: expected index %zu, saw %u", i, i,
			g_seen[i]);
	free(got);
}

static void	case_05(t_ctx *c)
{
	static const unsigned char	want[4] = {0, 0, 0, 0};
	char						*got;

	got = ft_strmapi("abc", f_zero);
	bro_track(c, got);
	if (bro_injecting(c))
		return ;
	if (!got)
		return (bro_fail(c->out, "returned NULL"));
	if (memcmp(got, want, 4))
		bro_fail(c->out, "expected 4 zero bytes, got a mismatch");
	free(got);
}

static void	case_06(t_ctx *c)
{
	char	s[4];
	char	*got;

	strcpy(s, "abc");
	got = ft_strmapi(s, f_add);
	bro_track(c, got);
	if (bro_injecting(c))
		return ;
	if (!got)
		return (bro_fail(c->out, "returned NULL"));
	if (got == s)
		bro_fail(c->out, "result aliases the input pointer");
	free(got);
}

static const t_case	g_cases[] = {
{1, "ft_strmapi(\"abc\", f): f = c + i", BRO_INJECT, case_01},
{2, "ft_strmapi(\"\", f): f never called", BRO_INJECT, case_02},
{3, "ft_strmapi(\"abc\", f): s unchanged", BRO_INJECT, case_03},
{4, "ft_strmapi(\"abcde\", f): index order", BRO_INJECT, case_04},
{5, "ft_strmapi(\"abc\", f): f returns '\\0'", BRO_INJECT, case_05},
{6, "ft_strmapi(\"abc\", f): result is distinct", BRO_INJECT, case_06},
};

const t_suite	g_suite_ft_strmapi = {
	"ft_strmapi", 2, 6, 2, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
