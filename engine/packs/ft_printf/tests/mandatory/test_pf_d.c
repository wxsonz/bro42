#include "bro.h"
#include "proto.h"
#include "printf_assert.h"
#include <stdio.h>
#include <limits.h>

/*
** pf_d - T4. Cases from _dev/plan/rank01/ftprintf-01-cases.md section 5.
**
** Signed decimal: a minus sign for negative values, then the same
** divide-and-remainder digit loop either way. INT_MIN cannot be negated -
** cases 11 use the macro from <limits.h>, never the literal -2147483648,
** which is not even an `int` (it is `long` on this platform: the decimal
** constant 2147483648 does not fit in int, so it promotes to long before the
** unary minus, and handing that to ft_printf's `...` for a %d conversion
** that reads va_arg(ap, int) would itself be undefined behaviour the case
** never meant to exercise).
*/

static void	case_01(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%d", 0);
	eret = snprintf(ref, sizeof(ref), "%d", 0);
	pf_check(c, aret, ref, eret);
}

static void	case_02(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%d", 5);
	eret = snprintf(ref, sizeof(ref), "%d", 5);
	pf_check(c, aret, ref, eret);
}

static void	case_03(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%d", -5);
	eret = snprintf(ref, sizeof(ref), "%d", -5);
	pf_check(c, aret, ref, eret);
}

static void	case_04(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%d", 1);
	eret = snprintf(ref, sizeof(ref), "%d", 1);
	pf_check(c, aret, ref, eret);
}

static void	case_05(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%d", -1);
	eret = snprintf(ref, sizeof(ref), "%d", -1);
	pf_check(c, aret, ref, eret);
}

/* digit-count boundaries: correct at 9, wrong at 10 is the classic symptom. */
static void	case_06(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%d", 9);
	eret = snprintf(ref, sizeof(ref), "%d", 9);
	pf_check(c, aret, ref, eret);
}

static void	case_07(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%d", 10);
	eret = snprintf(ref, sizeof(ref), "%d", 10);
	pf_check(c, aret, ref, eret);
}

static void	case_08(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%d", 99);
	eret = snprintf(ref, sizeof(ref), "%d", 99);
	pf_check(c, aret, ref, eret);
}

static void	case_09(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%d", 100);
	eret = snprintf(ref, sizeof(ref), "%d", 100);
	pf_check(c, aret, ref, eret);
}

static void	case_10(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%d", -100);
	eret = snprintf(ref, sizeof(ref), "%d", -100);
	pf_check(c, aret, ref, eret);
}

/* the INT_MIN trap: no positive int counterpart exists to negate into. */
static void	case_11(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%d", INT_MIN);
	eret = snprintf(ref, sizeof(ref), "%d", INT_MIN);
	pf_check(c, aret, ref, eret);
}

static void	case_12(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%d", INT_MAX);
	eret = snprintf(ref, sizeof(ref), "%d", INT_MAX);
	pf_check(c, aret, ref, eret);
}

/* one away from INT_MIN - negates fine with ordinary arithmetic. */
static void	case_13(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%d", -2147483647);
	eret = snprintf(ref, sizeof(ref), "%d", -2147483647);
	pf_check(c, aret, ref, eret);
}

static void	case_14(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("[%d]", 5);
	eret = snprintf(ref, sizeof(ref), "[%d]", 5);
	pf_check(c, aret, ref, eret);
}

static void	case_15(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%d apples, %d oranges", 3, 5);
	eret = snprintf(ref, sizeof(ref), "%d apples, %d oranges", 3, 5);
	pf_check(c, aret, ref, eret);
}

/* two single-digit arguments, not the number 12. */
static void	case_16(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%d%d", 1, 2);
	eret = snprintf(ref, sizeof(ref), "%d%d", 1, 2);
	pf_check(c, aret, ref, eret);
}

static void	case_17(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%d%d%d", -1, 0, 1);
	eret = snprintf(ref, sizeof(ref), "%d%d%d", -1, 0, 1);
	pf_check(c, aret, ref, eret);
}

/* the minus sign is a counted byte, distinct from the digits. */
static void	case_18(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%d", -9);
	eret = snprintf(ref, sizeof(ref), "%d", -9);
	pf_check(c, aret, ref, eret);
}

static void	case_19(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%d", -10);
	eret = snprintf(ref, sizeof(ref), "%d", -10);
	pf_check(c, aret, ref, eret);
}

static void	case_20(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%d", 1000000);
	eret = snprintf(ref, sizeof(ref), "%d", 1000000);
	pf_check(c, aret, ref, eret);
}

static const t_case	g_cases[] = {
{1, "ft_printf(\"%d\", 0)", BRO_ORACLE | BRO_CAPTURES_FD, case_01},
{2, "ft_printf(\"%d\", 5)", BRO_ORACLE | BRO_CAPTURES_FD, case_02},
{3, "ft_printf(\"%d\", -5)", BRO_ORACLE | BRO_CAPTURES_FD, case_03},
{4, "ft_printf(\"%d\", 1)", BRO_ORACLE | BRO_CAPTURES_FD, case_04},
{5, "ft_printf(\"%d\", -1)", BRO_ORACLE | BRO_CAPTURES_FD, case_05},
{6, "ft_printf(\"%d\", 9)", BRO_ORACLE | BRO_CAPTURES_FD, case_06},
{7, "ft_printf(\"%d\", 10)", BRO_ORACLE | BRO_CAPTURES_FD, case_07},
{8, "ft_printf(\"%d\", 99)", BRO_ORACLE | BRO_CAPTURES_FD, case_08},
{9, "ft_printf(\"%d\", 100)", BRO_ORACLE | BRO_CAPTURES_FD, case_09},
{10, "ft_printf(\"%d\", -100)", BRO_ORACLE | BRO_CAPTURES_FD, case_10},
{11, "ft_printf(\"%d\", -2147483648)", BRO_ORACLE | BRO_CAPTURES_FD, case_11},
{12, "ft_printf(\"%d\", 2147483647)", BRO_ORACLE | BRO_CAPTURES_FD, case_12},
{13, "ft_printf(\"%d\", -2147483647)", BRO_ORACLE | BRO_CAPTURES_FD, case_13},
{14, "ft_printf(\"[%d]\", 5)", BRO_ORACLE | BRO_CAPTURES_FD, case_14},
{15, "ft_printf(\"%d apples, %d oranges\", 3, 5)",
	BRO_ORACLE | BRO_CAPTURES_FD, case_15},
{16, "ft_printf(\"%d%d\", 1, 2)", BRO_ORACLE | BRO_CAPTURES_FD, case_16},
{17, "ft_printf(\"%d%d%d\", -1, 0, 1)", BRO_ORACLE | BRO_CAPTURES_FD,
	case_17},
{18, "ft_printf(\"%d\", -9)", BRO_ORACLE | BRO_CAPTURES_FD, case_18},
{19, "ft_printf(\"%d\", -10)", BRO_ORACLE | BRO_CAPTURES_FD, case_19},
{20, "ft_printf(\"%d\", 1000000)", BRO_ORACLE | BRO_CAPTURES_FD, case_20},
};

const t_suite	g_suite_pf_d = {
	"pf_d", 1, 3, 4, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
