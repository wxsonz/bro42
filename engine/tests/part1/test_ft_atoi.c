#include "bro.h"
#include "libft_proto.h"
#include <stdlib.h>

/*
** ft_atoi - T0. Cases from _dev/plan/rank00/libft-01-cases.md section 21.
** Oracle: atoi(nptr).
*/

static void	atoi_case(t_ctx *c, const char *s)
{
	bro_expect_num(c->out, atoi(s), ft_atoi(s));
}

static void	case_01(t_ctx *c) { atoi_case(c, "42"); }
static void	case_02(t_ctx *c) { atoi_case(c, "   -42"); }
static void	case_03(t_ctx *c) { atoi_case(c, "  +42"); }
static void	case_04(t_ctx *c) { atoi_case(c, "0"); }
static void	case_05(t_ctx *c) { atoi_case(c, "-0"); }
static void	case_06(t_ctx *c) { atoi_case(c, "42abc5"); }
static void	case_07(t_ctx *c) { atoi_case(c, " 42 "); }
static void	case_08(t_ctx *c) { atoi_case(c, "\t\n\v\f\r 123"); }
static void	case_09(t_ctx *c) { atoi_case(c, "+-42"); }
static void	case_10(t_ctx *c) { atoi_case(c, "--42"); }
static void	case_11(t_ctx *c) { atoi_case(c, ""); }
static void	case_12(t_ctx *c) { atoi_case(c, "   "); }
static void	case_13(t_ctx *c) { atoi_case(c, "2147483647"); }
static void	case_14(t_ctx *c) { atoi_case(c, "-2147483648"); }

/*
** Undefined: atoi past INT_MAX has no defined answer, so there is nothing to
** compare against. The case still runs and shows both values - evaluators do
** poke this, and the right answer is "that is UB", not silence.
*/
static void	case_15(t_ctx *c)
{
	const char	*s = "99999999999999999999";

	bro_mark_ub(c->out, "yours %d, glibc %d - overflow in atoi is undefined",
		ft_atoi(s), atoi(s));
}

static const t_case	g_cases[] = {
{1, "ft_atoi(\"42\")", BRO_ORACLE, case_01},
{2, "ft_atoi(\"   -42\")", BRO_ORACLE, case_02},
{3, "ft_atoi(\"  +42\")", BRO_ORACLE, case_03},
{4, "ft_atoi(\"0\")", BRO_ORACLE, case_04},
{5, "ft_atoi(\"-0\")", BRO_ORACLE, case_05},
{6, "ft_atoi(\"42abc5\")", BRO_ORACLE, case_06},
{7, "ft_atoi(\" 42 \")", BRO_ORACLE, case_07},
{8, "ft_atoi(\"\\t\\n\\v\\f\\r 123\")", BRO_ORACLE, case_08},
{9, "ft_atoi(\"+-42\")", BRO_ORACLE, case_09},
{10, "ft_atoi(\"--42\")", BRO_ORACLE, case_10},
{11, "ft_atoi(\"\")", BRO_ORACLE, case_11},
{12, "ft_atoi(\"   \")", BRO_ORACLE, case_12},
{13, "ft_atoi(\"2147483647\")", BRO_ORACLE, case_13},
{14, "ft_atoi(\"-2147483648\")", BRO_ORACLE, case_14},
{15, "ft_atoi(\"99999999999999999999\")", BRO_UB_CASE, case_15},
};

const t_suite	g_suite_ft_atoi = {
	"ft_atoi", 1, 4, 0, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
