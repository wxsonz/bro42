#include "bro.h"
#include "libft_proto.h"
#include <stdlib.h>
#include <string.h>

/*
** ft_strdup - T2. Cases from _dev/plan/rank00/libft-01-cases.md section 23.
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

/*
** Cases 6-8 close a hole found by writing a wrong implementation that
** mallocs strlen(s) bytes (no +1) and writes the terminator one byte past
** its own allocation: the text that comes back is still byte-for-byte
** correct, so strcmp above never notices. Only the size actually asked of
** the allocator, via the wrapped allocator's live-bytes accounting, tells
** it apart from a correct implementation.
*/
static void	strdup_size_case(t_ctx *c, const char *s, size_t want_bytes)
{
	t_alloc	before;
	t_alloc	after;
	char	*got;
	size_t	got_bytes;

	bro_alloc_snapshot(&before);
	got = ft_strdup(s);
	bro_track(c, got);
	if (bro_injecting(c))
		return ;
	bro_alloc_snapshot(&after);
	if (!got)
		return (bro_fail(c->out, "returned NULL"));
	got_bytes = after.live_bytes - before.live_bytes;
	if (got_bytes < want_bytes)
		bro_fail(c->out, "the result needs at least %zu byte(s), only %zu "
			"were allocated", want_bytes, got_bytes);
	free(got);
}

static void	case_06(t_ctx *c) { strdup_size_case(c, "Hello 42", 9); }
static void	case_07(t_ctx *c) { strdup_size_case(c, "", 1); }

static void	case_08(t_ctx *c)
{
	char	buf[101];

	memset(buf, 'q', 100);
	buf[100] = '\0';
	strdup_size_case(c, buf, 101);
}

static const t_case	g_cases[] = {
{1, "ft_strdup(\"Hello 42\")", BRO_ORACLE | BRO_INJECT, case_01},
{2, "ft_strdup(\"\")", BRO_ORACLE | BRO_INJECT, case_02},
{3, "ft_strdup(<100 chars>)", BRO_ORACLE | BRO_INJECT, case_03},
{4, "ft_strdup(\"abc\\0def\")", BRO_ORACLE, case_04},
{5, "ft_strdup(s)", BRO_ORACLE, case_05},
{6, "ft_strdup(\"Hello 42\"): exact allocation", BRO_INJECT, case_06},
{7, "ft_strdup(\"\"): exact allocation", BRO_INJECT, case_07},
{8, "ft_strdup(<100 chars>): exact allocation", BRO_INJECT, case_08},
};

const t_suite	g_suite_ft_strdup = {
	"ft_strdup", 1, 5, 2, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
