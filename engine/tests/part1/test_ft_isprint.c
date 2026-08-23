#include "bro.h"
#include "libft_proto.h"
#include <ctype.h>

/*
** ft_isprint - T0. Cases from _dev/plan/rank00/libft-01-cases.md section 5.
** Oracle: isprint(c) != 0 ? 1 : 0
**
** The oracle is normalised to exactly 1 or 0 on purpose: libc promises only
** "non-zero", but Subject IV.2 asks for strictly 1 or 0, so comparing against
** the raw libc value would let a wrong-but-truthy return pass.
*/

static void	is_case(t_ctx *c, int arg)
{
	bro_expect_num(c->out, isprint(arg) != 0, ft_isprint(arg));
}

static void	case_01(t_ctx *c) { is_case(c, 32); }
static void	case_02(t_ctx *c) { is_case(c, 126); }
static void	case_03(t_ctx *c) { is_case(c, 'A'); }
static void	case_04(t_ctx *c) { is_case(c, 31); }
static void	case_05(t_ctx *c) { is_case(c, 127); }
static void	case_06(t_ctx *c) { is_case(c, '\n'); }
static void	case_07(t_ctx *c) { is_case(c, -1); }

static const t_case	g_cases[] = {
{1, "ft_isprint(32)", BRO_ORACLE, case_01},
{2, "ft_isprint(126)", BRO_ORACLE, case_02},
{3, "ft_isprint('A')", BRO_ORACLE, case_03},
{4, "ft_isprint(31)", BRO_ORACLE, case_04},
{5, "ft_isprint(127)", BRO_ORACLE, case_05},
{6, "ft_isprint('\\n')", BRO_ORACLE, case_06},
{7, "ft_isprint(-1)", BRO_ORACLE, case_07},
};

const t_suite	g_suite_ft_isprint = {
	"ft_isprint", 1, 1, 0, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
