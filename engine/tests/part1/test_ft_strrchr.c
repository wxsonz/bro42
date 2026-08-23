#include "bro.h"
#include "libft_proto.h"
#include <string.h>

/*
** ft_strrchr - T0. Cases from _dev/SPEC_MICRO.md section 16.
** Oracle: strrchr(s, c). Compared as an offset into the input.
*/

static void	rchr_case(t_ctx *c, const char *s, int ch)
{
	bro_expect_offset(c->out, s, strrchr(s, ch), ft_strrchr(s, ch));
}

static void	case_01(t_ctx *c) { rchr_case(c, "tripouille", 'l'); }
static void	case_02(t_ctx *c) { rchr_case(c, "tripouille", 't'); }
static void	case_03(t_ctx *c) { rchr_case(c, "tripouille", 'z'); }
static void	case_04(t_ctx *c) { rchr_case(c, "tripouille", '\0'); }
static void	case_05(t_ctx *c) { rchr_case(c, "", '\0'); }
static void	case_06(t_ctx *c) { rchr_case(c, "", 'a'); }
static void	case_07(t_ctx *c) { rchr_case(c, "tripouille", 'l' + 256); }
static void	case_08(t_ctx *c) { rchr_case(c, "caf\303\251", 0xC3); }

static const t_case	g_cases[] = {
{1, "ft_strrchr(\"tripouille\", 'l')", BRO_ORACLE, case_01},
{2, "ft_strrchr(\"tripouille\", 't')", BRO_ORACLE, case_02},
{3, "ft_strrchr(\"tripouille\", 'z')", BRO_ORACLE, case_03},
{4, "ft_strrchr(\"tripouille\", '\\0')", BRO_ORACLE, case_04},
{5, "ft_strrchr(\"\", '\\0')", BRO_ORACLE, case_05},
{6, "ft_strrchr(\"\", 'a')", BRO_ORACLE, case_06},
{7, "ft_strrchr(\"tripouille\", 'l' + 256)", BRO_ORACLE, case_07},
{8, "ft_strrchr(\"caf\\303\\251\", 0xC3)", BRO_ORACLE, case_08},
};

const t_suite	g_suite_ft_strrchr = {
	"ft_strrchr", 1, 2, 0, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
