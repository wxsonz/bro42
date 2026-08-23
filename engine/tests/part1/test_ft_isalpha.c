#include "bro.h"
#include "libft_proto.h"
#include <ctype.h>

/*
** ft_isalpha - T0. Cases from _dev/plan/rank00/libft-01-cases.md section 1.
** Oracle: isalpha(c) != 0 ? 1 : 0
**
** The oracle is normalised to exactly 1 or 0 on purpose: libc promises only
** "non-zero", but Subject IV.2 asks for strictly 1 or 0, so comparing against
** the raw libc value would let a wrong-but-truthy return pass.
*/

static void	is_case(t_ctx *c, int arg)
{
	bro_expect_num(c->out, isalpha(arg) != 0, ft_isalpha(arg));
}

static void	case_01(t_ctx *c) { is_case(c, 'a'); }
static void	case_02(t_ctx *c) { is_case(c, 'Z'); }
static void	case_03(t_ctx *c) { is_case(c, '0'); }
static void	case_04(t_ctx *c) { is_case(c, 'a' - 1); }
static void	case_05(t_ctx *c) { is_case(c, 'z' + 1); }
static void	case_06(t_ctx *c) { is_case(c, 'A' - 1); }
static void	case_07(t_ctx *c) { is_case(c, 'Z' + 1); }
static void	case_08(t_ctx *c) { is_case(c, -1); }
static void	case_09(t_ctx *c) { is_case(c, 128); }

static const t_case	g_cases[] = {
{1, "ft_isalpha('a')", BRO_ORACLE, case_01},
{2, "ft_isalpha('Z')", BRO_ORACLE, case_02},
{3, "ft_isalpha('0')", BRO_ORACLE, case_03},
{4, "ft_isalpha('a' - 1)", BRO_ORACLE, case_04},
{5, "ft_isalpha('z' + 1)", BRO_ORACLE, case_05},
{6, "ft_isalpha('A' - 1)", BRO_ORACLE, case_06},
{7, "ft_isalpha('Z' + 1)", BRO_ORACLE, case_07},
{8, "ft_isalpha(-1)", BRO_ORACLE, case_08},
{9, "ft_isalpha(128)", BRO_ORACLE, case_09},
};

const t_suite	g_suite_ft_isalpha = {
	"ft_isalpha", 1, 1, 0, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
