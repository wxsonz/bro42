#include "bro.h"
#include "proto.h"
#include "printf_assert.h"
#include <stdio.h>

/*
** pf_c - T4. Cases from _dev/plan/rank01/ftprintf-01-cases.md section 3.
**
** %c writes exactly one byte - the promoted argument's low byte - and
** returns 1. Nothing about the byte's value is ever consulted: not for
** '\0' (the single most common ft_printf bug, a writer built on strlen sees
** an empty string), and not for anything above 7-bit ASCII.
*/

static void	case_01(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%c", 'a');
	eret = snprintf(ref, sizeof(ref), "%c", 'a');
	pf_check(c, aret, ref, eret);
}

/* '\0' is a real byte here, not a terminator - strlen sees nothing. */
static void	case_02(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%c", '\0');
	eret = snprintf(ref, sizeof(ref), "%c", '\0');
	pf_check(c, aret, ref, eret);
}

/* above 7-bit ASCII, still a single byte - never gated on isprint. */
static void	case_03(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%c", (char)200);
	eret = snprintf(ref, sizeof(ref), "%c", (char)200);
	pf_check(c, aret, ref, eret);
}

static void	case_04(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%c", (char)255);
	eret = snprintf(ref, sizeof(ref), "%c", (char)255);
	pf_check(c, aret, ref, eret);
}

static void	case_05(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("[%c]", 'x');
	eret = snprintf(ref, sizeof(ref), "[%c]", 'x');
	pf_check(c, aret, ref, eret);
}

static void	case_06(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%c%c%c", 'a', 'b', 'c');
	eret = snprintf(ref, sizeof(ref), "%c%c%c", 'a', 'b', 'c');
	pf_check(c, aret, ref, eret);
}

/*
** %c reads one int-sized argument, then narrows - a read that assumed a
** one-byte-wide argument would desynchronise the %s that follows it.
*/
static void	case_07(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%c%s", 'A', "42");
	eret = snprintf(ref, sizeof(ref), "%c%s", 'A', "42");
	pf_check(c, aret, ref, eret);
}

/* the byte 0x05, not the printable digit '5' - %c never looks up a digit. */
static void	case_08(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%c", 5);
	eret = snprintf(ref, sizeof(ref), "%c", 5);
	pf_check(c, aret, ref, eret);
}

static void	case_09(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%c", ' ');
	eret = snprintf(ref, sizeof(ref), "%c", ' ');
	pf_check(c, aret, ref, eret);
}

/* five independent int-sized arguments, not one five-character string. */
static void	case_10(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%c%c%c%c%c", 'H', 'e', 'l', 'l', 'o');
	eret = snprintf(ref, sizeof(ref), "%c%c%c%c%c", 'H', 'e', 'l', 'l', 'o');
	pf_check(c, aret, ref, eret);
}

static const t_case	g_cases[] = {
{1, "ft_printf(\"%c\", 'a')", BRO_ORACLE | BRO_CAPTURES_FD, case_01},
{2, "ft_printf(\"%c\", '\\0')", BRO_ORACLE | BRO_CAPTURES_FD, case_02},
{3, "ft_printf(\"%c\", (char)200)", BRO_ORACLE | BRO_CAPTURES_FD, case_03},
{4, "ft_printf(\"%c\", (char)255)", BRO_ORACLE | BRO_CAPTURES_FD, case_04},
{5, "ft_printf(\"[%c]\", 'x')", BRO_ORACLE | BRO_CAPTURES_FD, case_05},
{6, "ft_printf(\"%c%c%c\", 'a', 'b', 'c')", BRO_ORACLE | BRO_CAPTURES_FD,
	case_06},
{7, "ft_printf(\"%c%s\", 'A', \"42\")", BRO_ORACLE | BRO_CAPTURES_FD, case_07},
{8, "ft_printf(\"%c\", 5)", BRO_ORACLE | BRO_CAPTURES_FD, case_08},
{9, "ft_printf(\"%c\", ' ')", BRO_ORACLE | BRO_CAPTURES_FD, case_09},
{10, "ft_printf(\"%c%c%c%c%c\", 'H', 'e', 'l', 'l', 'o')",
	BRO_ORACLE | BRO_CAPTURES_FD, case_10},
};

const t_suite	g_suite_pf_c = {
	"pf_c", 1, 2, 4, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
