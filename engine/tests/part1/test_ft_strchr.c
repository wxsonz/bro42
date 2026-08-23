#include "bro.h"
#include "libft_proto.h"
#include <string.h>

/*
** ft_strchr - T0. Cases from _dev/plan/rank00/libft-01-cases.md section 15.
** Oracle: strchr(s, c).
**
** The result is a pointer INTO the input, so it is compared as an offset from
** the base rather than as an address - see design/04_TESTDESIGN.md.
*/

static void	chr_case(t_ctx *c, const char *s, int ch)
{
	bro_expect_offset(c->out, s, strchr(s, ch), ft_strchr(s, ch));
}

static void	case_01(t_ctx *c) { chr_case(c, "tripouille", 't'); }
static void	case_02(t_ctx *c) { chr_case(c, "tripouille", 'l'); }
static void	case_03(t_ctx *c) { chr_case(c, "tripouille", 'z'); }
static void	case_04(t_ctx *c) { chr_case(c, "tripouille", '\0'); }
static void	case_05(t_ctx *c) { chr_case(c, "", '\0'); }
static void	case_06(t_ctx *c) { chr_case(c, "", 'a'); }
static void	case_07(t_ctx *c) { chr_case(c, "tripouille", 't' + 256); }
static void	case_08(t_ctx *c) { chr_case(c, "caf\303\251", 0xC3); }

static const t_case	g_cases[] = {
{1, "ft_strchr(\"tripouille\", 't')", BRO_ORACLE, case_01},
{2, "ft_strchr(\"tripouille\", 'l')", BRO_ORACLE, case_02},
{3, "ft_strchr(\"tripouille\", 'z')", BRO_ORACLE, case_03},
{4, "ft_strchr(\"tripouille\", '\\0')", BRO_ORACLE, case_04},
{5, "ft_strchr(\"\", '\\0')", BRO_ORACLE, case_05},
{6, "ft_strchr(\"\", 'a')", BRO_ORACLE, case_06},
{7, "ft_strchr(\"tripouille\", 't' + 256)", BRO_ORACLE, case_07},
{8, "ft_strchr(\"caf\\303\\251\", 0xC3)", BRO_ORACLE, case_08},
};

const t_suite	g_suite_ft_strchr = {
	"ft_strchr", 1, 2, 0, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
