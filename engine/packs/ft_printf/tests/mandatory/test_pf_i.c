#include "bro.h"
#include "proto.h"
#include "printf_assert.h"
#include <stdio.h>
#include <limits.h>

/*
** pf_i - T4. Cases from _dev/plan/rank01/ftprintf-01-cases.md section 6.
**
** A deliberate mirror of pf_d: the C standard defines %d and %i identically
** on output, so every case here exists to say, explicitly, that they are one
** routine under two names - including the INT_MIN trap (case_04), which
** either breaks both conversions or neither. See test_pf_d.c for why INT_MIN
** is reached through the <limits.h> macro rather than the literal
** -2147483648.
*/

static void	case_01(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%i", 0);
	eret = snprintf(ref, sizeof(ref), "%i", 0);
	pf_check(c, aret, ref, eret);
}

static void	case_02(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%i", 42);
	eret = snprintf(ref, sizeof(ref), "%i", 42);
	pf_check(c, aret, ref, eret);
}

static void	case_03(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%i", -42);
	eret = snprintf(ref, sizeof(ref), "%i", -42);
	pf_check(c, aret, ref, eret);
}

/* %i inherits the INT_MIN trap too - the identical routine, or a second
** copy that only looks the same. */
static void	case_04(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%i", INT_MIN);
	eret = snprintf(ref, sizeof(ref), "%i", INT_MIN);
	pf_check(c, aret, ref, eret);
}

static void	case_05(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%i", INT_MAX);
	eret = snprintf(ref, sizeof(ref), "%i", INT_MAX);
	pf_check(c, aret, ref, eret);
}

/* %d and %i mixed in one call - two independently written implementations
** would quietly drift apart exactly here. */
static void	case_06(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%d and %i", 5, -5);
	eret = snprintf(ref, sizeof(ref), "%d and %i", 5, -5);
	pf_check(c, aret, ref, eret);
}

static void	case_07(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("[%i]", 7);
	eret = snprintf(ref, sizeof(ref), "[%i]", 7);
	pf_check(c, aret, ref, eret);
}

static void	case_08(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%i%i", 1, 2);
	eret = snprintf(ref, sizeof(ref), "%i%i", 1, 2);
	pf_check(c, aret, ref, eret);
}

/* the digit-count boundary is shared too. */
static void	case_09(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%i", 10);
	eret = snprintf(ref, sizeof(ref), "%i", 10);
	pf_check(c, aret, ref, eret);
}

static void	case_10(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%i", -1);
	eret = snprintf(ref, sizeof(ref), "%i", -1);
	pf_check(c, aret, ref, eret);
}

static const t_case	g_cases[] = {
{1, "ft_printf(\"%i\", 0)", BRO_ORACLE | BRO_CAPTURES_FD, case_01},
{2, "ft_printf(\"%i\", 42)", BRO_ORACLE | BRO_CAPTURES_FD, case_02},
{3, "ft_printf(\"%i\", -42)", BRO_ORACLE | BRO_CAPTURES_FD, case_03},
{4, "ft_printf(\"%i\", -2147483648)", BRO_ORACLE | BRO_CAPTURES_FD, case_04},
{5, "ft_printf(\"%i\", 2147483647)", BRO_ORACLE | BRO_CAPTURES_FD, case_05},
{6, "ft_printf(\"%d and %i\", 5, -5)", BRO_ORACLE | BRO_CAPTURES_FD, case_06},
{7, "ft_printf(\"[%i]\", 7)", BRO_ORACLE | BRO_CAPTURES_FD, case_07},
{8, "ft_printf(\"%i%i\", 1, 2)", BRO_ORACLE | BRO_CAPTURES_FD, case_08},
{9, "ft_printf(\"%i\", 10)", BRO_ORACLE | BRO_CAPTURES_FD, case_09},
{10, "ft_printf(\"%i\", -1)", BRO_ORACLE | BRO_CAPTURES_FD, case_10},
};

const t_suite	g_suite_pf_i = {
	"pf_i", 1, 3, 4, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
