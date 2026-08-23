#include "bro.h"
#include "libft_proto.h"
#include <stdlib.h>
#include <string.h>

/*
** ft_split - T3. Cases from _dev/SPEC_MICRO.md section 27.
**
** The hardest function in the project: two levels of allocation and two levels
** of cleanup. The rollback cases are where the real bugs are, and the harness
** generates them - one authored case yields one check per allocation the split
** makes.
*/

static void	free_split(char **r)
{
	size_t	i;

	if (!r)
		return ;
	i = 0;
	while (r[i])
		free(r[i++]);
	free(r);
}

static long	count_words(char **r)
{
	long	n;

	n = 0;
	while (r[n])
		n++;
	return (n);
}

/*
** `expected` is a NULL-terminated list of the words that should come back.
** The NULL terminator on the result is checked explicitly: it is how the
** caller knows where to stop, and it is not optional.
*/
static void	split_case(t_ctx *c, const char *s, char sep,
		const char *const *expected)
{
	char	**got;
	long	i;
	long	want;

	got = ft_split(s, sep);
	bro_track(c, got);
	if (bro_injecting(c))
		return ;
	if (!got)
		return (bro_fail(c->out, "returned NULL"));
	want = 0;
	while (expected[want])
		want++;
	if (count_words(got) != want)
	{
		bro_fail(c->out, "expected %ld word(s), got %ld", want,
			count_words(got));
		return (free_split(got));
	}
	i = 0;
	while (i < want && !strcmp(got[i], expected[i]))
		i++;
	if (i < want)
		bro_fail(c->out, "word %ld: expected \"%s\", got \"%s\"", i,
			expected[i], got[i]);
	free_split(got);
}

static const char *const	g_none[] = {NULL};

static void	case_01(t_ctx *c)
{
	static const char *const	e[] = {"hello", "world", "42", NULL};

	split_case(c, "hello world 42", ' ', e);
}

static void	case_02(t_ctx *c)
{
	static const char *const	e[] = {"a", "b", "c", NULL};

	split_case(c, "a b c", ' ', e);
}

static void	case_03(t_ctx *c)
{
	static const char *const	e[] = {"hello", NULL};

	split_case(c, "hello", ' ', e);
}

static void	case_04(t_ctx *c)
{
	static const char	*e[101];
	static char		big[401];
	int			i;

	i = 0;
	while (i < 100)
	{
		big[i * 4 + 0] = 'w';
		big[i * 4 + 1] = 'o';
		big[i * 4 + 2] = 'w';
		big[i * 4 + 3] = ' ';
		e[i] = "wow";
		i++;
	}
	big[400] = '\0';
	e[100] = NULL;
	split_case(c, big, ' ', (const char *const *)e);
}

static void	case_05(t_ctx *c)
{
	static const char *const	e[] = {"hello", "world", NULL};

	split_case(c, "   hello   world   ", ' ', e);
}

static void	case_06(t_ctx *c)
{
	static const char *const	e[] = {"hello", NULL};

	split_case(c, "  hello", ' ', e);
}

static void	case_07(t_ctx *c)
{
	static const char *const	e[] = {"hello", NULL};

	split_case(c, "hello  ", ' ', e);
}

static void	case_08(t_ctx *c)
{
	static const char *const	e[] = {"a", "b", NULL};

	split_case(c, "a  b", ' ', e);
}

static void	case_09(t_ctx *c) { split_case(c, "    ", ' ', g_none); }
static void	case_10(t_ctx *c) { split_case(c, "", ' ', g_none); }
static void	case_11(t_ctx *c) { split_case(c, "aaa", 'a', g_none); }

static void	case_12(t_ctx *c)
{
	static const char *const	e[] = {"hello world", NULL};

	split_case(c, "hello world", '\0', e);
}

/* The two dedicated rollback cases: the harness sweeps every allocation. */
static void	case_13(t_ctx *c)
{
	static const char *const	e[] = {"hello", "world", "42", NULL};

	split_case(c, "hello world 42", ' ', e);
}

static void	case_14(t_ctx *c)
{
	static const char *const	e[] = {"a", "b", "c", "d", "e", NULL};

	split_case(c, "a b c d e", ' ', e);
}

static const t_case	g_cases[] = {
{1, "ft_split(\"hello world 42\", ' ')", BRO_INJECT, case_01},
{2, "ft_split(\"a b c\", ' ')", BRO_INJECT, case_02},
{3, "ft_split(\"hello\", ' ')", BRO_INJECT, case_03},
{4, "ft_split(<100 words>, ' ')", BRO_INJECT, case_04},
{5, "ft_split(\"   hello   world   \", ' ')", BRO_INJECT, case_05},
{6, "ft_split(\"  hello\", ' ')", BRO_INJECT, case_06},
{7, "ft_split(\"hello  \", ' ')", BRO_INJECT, case_07},
{8, "ft_split(\"a  b\", ' ')", BRO_INJECT, case_08},
{9, "ft_split(\"    \", ' ')", BRO_INJECT, case_09},
{10, "ft_split(\"\", ' ')", BRO_INJECT, case_10},
{11, "ft_split(\"aaa\", 'a')", BRO_INJECT, case_11},
{12, "ft_split(\"hello world\", '\\0')", BRO_INJECT, case_12},
{13, "ft_split(\"hello world 42\", ' ')", BRO_INJECT, case_13},
{14, "ft_split(\"a b c d e\", ' ')", BRO_INJECT, case_14},
};

const t_suite	g_suite_ft_split = {
	"ft_split", 2, 6, 3, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
