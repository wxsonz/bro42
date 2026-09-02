#include "bro.h"
#include "proto.h"
#include "printf_assert.h"
#include <string.h>
#include <stdio.h>

/*
** pf_literal - T4. Cases from _dev/plan/rank01/ftprintf-01-cases.md section 1.
**
** No conversion in sight: every byte of the format that is not part of a
** conversion goes to write() exactly as it stands, and the return value is
** that byte count. The oracle is real snprintf on the identical literal, so
** the expected bytes and the expected return are never typed twice.
*/

static void	case_01(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("Hello, 42!");
	eret = snprintf(ref, sizeof(ref), "Hello, 42!");
	pf_check(c, aret, ref, eret);
}

static void	case_02(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	const char	*fmt = "";

	if (!pf_begin(c))
		return ;
	aret = ft_printf(fmt);
	eret = snprintf(ref, sizeof(ref), fmt);
	pf_check(c, aret, ref, eret);
}

/* control bytes are still just bytes - nothing about scanning for '%' looks
** at what a byte actually is, only at whether it equals '%'. */
static void	case_03(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("a\nb\tc");
	eret = snprintf(ref, sizeof(ref), "a\nb\tc");
	pf_check(c, aret, ref, eret);
}

static void	case_04(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("\n");
	eret = snprintf(ref, sizeof(ref), "\n");
	pf_check(c, aret, ref, eret);
}

/*
** A literal NUL cannot travel through a C string: format is `const char *`,
** and the format string itself already ends at the first NUL byte before
** ft_printf is ever called - "cd" is unreachable, at compile time, not by
** any choice ft_printf makes.
*/
static void	case_05(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	const char	*fmt = "ab\0cd";

	if (!pf_begin(c))
		return ;
	aret = ft_printf(fmt);
	eret = snprintf(ref, sizeof(ref), fmt);
	pf_check(c, aret, ref, eret);
}

static void	case_06(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("no percent signs here");
	eret = snprintf(ref, sizeof(ref), "no percent signs here");
	pf_check(c, aret, ref, eret);
}

static void	case_07(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("42");
	eret = snprintf(ref, sizeof(ref), "42");
	pf_check(c, aret, ref, eret);
}

/*
** A long literal run is not batched: Subject IV forbids implementing
** printf's own buffer management, so 1024 bytes has to reach write() the
** same way 5 bytes does. `fmt` is used as the format string itself (no
** conversions in it), which is what the case is actually about; the oracle
** wraps it in "%s" instead of passing it as printf's own format so -Werror
** does not trip on -Wformat-security's "format string is not a literal"
** check for a call with no variadic arguments - the bytes it produces are
** identical either way, since fmt contains no '%'.
**
** `fmt` and `ref` are both sized one byte past BRO_CAPTURE_MAX so snprintf
** never truncates a fixture this size; see printf_assert.c's own comment on
** why BRO_CAPTURE_MAX alone is one byte short for a full-width case.
*/
static void	case_08(t_ctx *c)
{
	static char	fmt[1025];
	char		ref[BRO_CAPTURE_MAX + 1];
	int		eret;
	int		aret;

	memset(fmt, 'x', sizeof(fmt) - 1);
	fmt[sizeof(fmt) - 1] = '\0';
	if (!pf_begin(c))
		return ;
	aret = ft_printf(fmt);
	eret = snprintf(ref, sizeof(ref), "%s", fmt);
	if (eret >= (int)sizeof(ref))
		return (bro_fail(c->out,
				"engine: fixture too large for the oracle buffer"));
	pf_check(c, aret, ref, eret);
}

static const t_case	g_cases[] = {
{1, "ft_printf(\"Hello, 42!\")", BRO_ORACLE | BRO_CAPTURES_FD, case_01},
{2, "ft_printf(\"\")", BRO_ORACLE | BRO_CAPTURES_FD, case_02},
{3, "ft_printf(\"a\\nb\\tc\")", BRO_ORACLE | BRO_CAPTURES_FD, case_03},
{4, "ft_printf(\"\\n\")", BRO_ORACLE | BRO_CAPTURES_FD, case_04},
{5, "ft_printf(\"ab\\0cd\")", BRO_ORACLE | BRO_CAPTURES_FD, case_05},
{6, "ft_printf(\"no percent signs here\")", BRO_ORACLE | BRO_CAPTURES_FD,
	case_06},
{7, "ft_printf(\"42\")", BRO_ORACLE | BRO_CAPTURES_FD, case_07},
{8, "ft_printf(<1024 bytes of 'x'>)", BRO_ORACLE | BRO_CAPTURES_FD, case_08},
};

const t_suite	g_suite_pf_literal = {
	"pf_literal", 1, 1, 4, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
