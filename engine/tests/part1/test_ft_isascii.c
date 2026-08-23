#include "bro.h"
#include "libft_proto.h"
#include <ctype.h>

/*
** ft_isascii - T0. Cases from _dev/SPEC_MICRO.md section 4.
** Oracle: isascii(c) != 0 ? 1 : 0
**
** The oracle is normalised to exactly 1 or 0 on purpose: libc promises only
** "non-zero", but Subject IV.2 asks for strictly 1 or 0, so comparing against
** the raw libc value would let a wrong-but-truthy return pass.
*/

static void	is_case(t_ctx *c, int arg)
{
	bro_expect_num(c->out, isascii(arg) != 0, ft_isascii(arg));
}

static void	case_01(t_ctx *c) { is_case(c, 0); }
static void	case_02(t_ctx *c) { is_case(c, 127); }
static void	case_03(t_ctx *c) { is_case(c, 'A'); }
static void	case_04(t_ctx *c) { is_case(c, -1); }
static void	case_05(t_ctx *c) { is_case(c, 128); }

static const t_case	g_cases[] = {
{1, "ft_isascii(0)", BRO_ORACLE, case_01},
{2, "ft_isascii(127)", BRO_ORACLE, case_02},
{3, "ft_isascii('A')", BRO_ORACLE, case_03},
{4, "ft_isascii(-1)", BRO_ORACLE, case_04},
{5, "ft_isascii(128)", BRO_ORACLE, case_05},
};

const t_suite	g_suite_ft_isascii = {
	"ft_isascii", 1, 1, 0, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
