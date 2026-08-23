#include "bro.h"
#include "libft_proto.h"
#include <string.h>

/*
** ft_strlen - T0. Cases from _dev/SPEC_MICRO.md section 6.
** Oracle: strlen(s).
*/

static void	len_case(t_ctx *c, const char *s)
{
	bro_expect_num(c->out, (long long)strlen(s), (long long)ft_strlen(s));
}

static void	case_01(t_ctx *c) { len_case(c, ""); }
static void	case_02(t_ctx *c) { len_case(c, "a"); }
static void	case_03(t_ctx *c) { len_case(c, "Hello 42!"); }
static void	case_04(t_ctx *c) { len_case(c, "abc\0def"); }

/*
** 100 KB, to catch an implementation that copies before it counts. The buffer
** is static rather than allocated: engine scratch must never look like student
** allocation once alloc.c is armed.
*/
static void	case_05(t_ctx *c)
{
	static char	big[100001];

	memset(big, 'a', sizeof(big) - 1);
	big[sizeof(big) - 1] = '\0';
	len_case(c, big);
}

static const t_case	g_cases[] = {
{1, "ft_strlen(\"\")", BRO_ORACLE, case_01},
{2, "ft_strlen(\"a\")", BRO_ORACLE, case_02},
{3, "ft_strlen(\"Hello 42!\")", BRO_ORACLE, case_03},
{4, "ft_strlen(\"abc\\0def\")", BRO_ORACLE, case_04},
{5, "ft_strlen(<100000 'a' chars>)", BRO_ORACLE, case_05},
};

const t_suite	g_suite_ft_strlen = {
	"ft_strlen", 1, 2, 0, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
