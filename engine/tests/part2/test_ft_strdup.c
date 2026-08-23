#include "bro.h"
#include "libft_proto.h"
#include <stdlib.h>
#include <string.h>

/*
** ft_strdup - T2. Cases from _dev/SPEC_MICRO.md section 23.
**
** Cases 4 and 5 carry no INJECT: they are not about the allocation
** succeeding or failing, but about what gets copied (case 4, an embedded
** NUL) and whose memory the result is (case 5). Both need a mutable local
** buffer rather than a string literal, so a naive implementation that
** returns its input pointer instead of a copy is caught corrupting it.
*/

static void	strdup_case(t_ctx *c, const char *s)
{
	char	*got;

	got = ft_strdup(s);
	bro_track(c, got);
	if (bro_injecting(c))
		return ;
	if (!got)
		return (bro_fail(c->out, "returned NULL"));
	if (got == s)
	{
		free(got);
		return (bro_fail(c->out, "returned the input pointer, not a copy"));
	}
	if (strcmp(got, s))
		bro_fail(c->out, "expected \"%s\", got \"%s\"", s, got);
	free(got);
}

static void	case_01(t_ctx *c) { strdup_case(c, "Hello 42"); }
static void	case_02(t_ctx *c) { strdup_case(c, ""); }

static void	case_03(t_ctx *c)
{
	char	buf[101];

	memset(buf, 'q', 100);
	buf[100] = '\0';
	strdup_case(c, buf);
}

/* strdup stops at the terminator: 3 bytes, not 7. */
static void	case_04(t_ctx *c)
{
	char	*got;

	got = ft_strdup("abc\0def");
	if (!got)
		return (bro_fail(c->out, "returned NULL"));
	if (strlen(got) != 3 || strcmp(got, "abc"))
		bro_fail(c->out, "expected \"abc\" (len 3), got \"%s\" (len %zu)",
			got, strlen(got));
	free(got);
}

static void	case_05(t_ctx *c)
{
	char	s[6];
	char	*got;

	strcpy(s, "hello");
	got = ft_strdup(s);
	if (!got)
		return (bro_fail(c->out, "returned NULL"));
	if (got == s)
	{
		free(got);
		return (bro_fail(c->out, "returned the input pointer, not a copy"));
	}
	got[0] = 'X';
	if (strcmp(s, "hello"))
		bro_fail(c->out, "writing to the result modified the input");
	free(got);
}

static const t_case	g_cases[] = {
{1, "ft_strdup(\"Hello 42\")", BRO_ORACLE | BRO_INJECT, case_01},
{2, "ft_strdup(\"\")", BRO_ORACLE | BRO_INJECT, case_02},
{3, "ft_strdup(<100 chars>)", BRO_ORACLE | BRO_INJECT, case_03},
{4, "ft_strdup(\"abc\\0def\")", BRO_ORACLE, case_04},
{5, "ft_strdup(s)", BRO_ORACLE, case_05},
};

const t_suite	g_suite_ft_strdup = {
	"ft_strdup", 1, 5, 2, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
