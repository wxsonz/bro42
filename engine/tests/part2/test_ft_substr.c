#include "bro.h"
#include "libft_proto.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
** ft_substr - T2. Cases from _dev/SPEC_MICRO.md section 24.
**
** Case 8 (len = SIZE_MAX) is a trap for the implementation, not the test: a
** correct substr caps the allocation at the characters actually remaining
** after start, so it allocates 6 bytes. A naive one trusts len directly and
** asks the kernel for 18 exabytes, gets NULL, and fails the case. That is
** the intended catch - the test does not "help" it by passing a smaller len.
*/

static void	substr_case(t_ctx *c, const char *s, unsigned int start,
		size_t len, const char *expected)
{
	char	*got;

	got = ft_substr(s, start, len);
	bro_track(c, got);
	if (bro_injecting(c))
		return ;
	if (!got)
		return (bro_fail(c->out, "returned NULL"));
	if (strcmp(got, expected))
		bro_fail(c->out, "expected \"%s\", got \"%s\"", expected, got);
	free(got);
}

static void	case_01(t_ctx *c) { substr_case(c, "hello world", 6, 5, "world"); }
static void	case_02(t_ctx *c) { substr_case(c, "hello world", 0, 5, "hello"); }
static void	case_03(t_ctx *c) { substr_case(c, "hello world", 0, 0, ""); }
static void	case_04(t_ctx *c) { substr_case(c, "hello world", 20, 5, ""); }
static void	case_05(t_ctx *c) { substr_case(c, "hello world", 11, 5, ""); }
static void	case_06(t_ctx *c) { substr_case(c, "", 0, 5, ""); }
static void	case_07(t_ctx *c) { substr_case(c, "hello world", 6, 50, "world"); }
static void	case_08(t_ctx *c) { substr_case(c, "hello world", 6, SIZE_MAX, "world"); }
static void	case_09(t_ctx *c) { substr_case(c, "hello", 1, 1000000000, "ello"); }

static const t_case	g_cases[] = {
{1, "ft_substr(\"hello world\", 6, 5)", BRO_INJECT, case_01},
{2, "ft_substr(\"hello world\", 0, 5)", BRO_INJECT, case_02},
{3, "ft_substr(\"hello world\", 0, 0)", BRO_INJECT, case_03},
{4, "ft_substr(\"hello world\", 20, 5)", BRO_INJECT, case_04},
{5, "ft_substr(\"hello world\", 11, 5)", BRO_INJECT, case_05},
{6, "ft_substr(\"\", 0, 5)", BRO_INJECT, case_06},
{7, "ft_substr(\"hello world\", 6, 50)", BRO_INJECT, case_07},
{8, "ft_substr(\"hello world\", 6, SIZE_MAX)", BRO_INJECT, case_08},
{9, "ft_substr(\"hello\", 1, 1000000000)", BRO_INJECT, case_09},
};

const t_suite	g_suite_ft_substr = {
	"ft_substr", 2, 5, 2, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
