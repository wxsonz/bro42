#include "bro.h"
#include "proto.h"
#include "printf_assert.h"
#include <stdio.h>

/*
** pf_pct - T4. Cases from _dev/plan/rank01/ftprintf-01-cases.md section 2.
**
** "%%" is a two-character literal-percent escape, consumed as its own thing
** before any conversion letter is even looked at, and it contributes exactly
** one byte to the shared return accumulator - never two, for the two format
** characters it consumed.
*/

static void	case_01(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%%");
	eret = snprintf(ref, sizeof(ref), "%%");
	pf_check(c, aret, ref, eret);
}

static void	case_02(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("100%%");
	eret = snprintf(ref, sizeof(ref), "100%%");
	pf_check(c, aret, ref, eret);
}

/* two independent %% pairs, not one four-character escape. */
static void	case_03(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%%%%");
	eret = snprintf(ref, sizeof(ref), "%%%%");
	pf_check(c, aret, ref, eret);
}

static void	case_04(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%d%%", 50);
	eret = snprintf(ref, sizeof(ref), "%d%%", 50);
	pf_check(c, aret, ref, eret);
}

/* the escape consumes exactly two characters; "d" is literal, not %d. */
static void	case_05(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%%d");
	eret = snprintf(ref, sizeof(ref), "%%d");
	pf_check(c, aret, ref, eret);
}

/* the first two characters resolve as %%, then %d starts fresh. */
static void	case_06(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%%%d", 7);
	eret = snprintf(ref, sizeof(ref), "%%%d", 7);
	pf_check(c, aret, ref, eret);
}

static const t_case	g_cases[] = {
{1, "ft_printf(\"%%\")", BRO_ORACLE | BRO_CAPTURES_FD, case_01},
{2, "ft_printf(\"100%%\")", BRO_ORACLE | BRO_CAPTURES_FD, case_02},
{3, "ft_printf(\"%%%%\")", BRO_ORACLE | BRO_CAPTURES_FD, case_03},
{4, "ft_printf(\"%d%%\", 50)", BRO_ORACLE | BRO_CAPTURES_FD, case_04},
{5, "ft_printf(\"%%d\")", BRO_ORACLE | BRO_CAPTURES_FD, case_05},
{6, "ft_printf(\"%%%d\", 7)", BRO_ORACLE | BRO_CAPTURES_FD, case_06},
};

const t_suite	g_suite_pf_pct = {
	"pf_pct", 1, 1, 4, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
