#include "bro.h"
#include "proto.h"
#include "printf_assert.h"
#include <string.h>
#include <stdio.h>

/*
** pf_s - T4. Cases from _dev/plan/rank01/ftprintf-01-cases.md section 4.
**
** %s writes exactly strlen(s) bytes and returns that count - no terminator,
** no padding, stopping at the first NUL the same way strlen does. NULL is
** left undefined by the standard (cases 08-09): they run, they display, and
** they are never scored (design A5) - see the UB cases below for why pf_check
** must not be used on them.
*/

static void	case_01(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%s", "Hello, 42!");
	eret = snprintf(ref, sizeof(ref), "%s", "Hello, 42!");
	pf_check(c, aret, ref, eret);
}

static void	case_02(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%s", "");
	eret = snprintf(ref, sizeof(ref), "%s", "");
	pf_check(c, aret, ref, eret);
}

/* the embedded NUL ends the string; "cd" is unreachable. */
static void	case_03(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%s", "ab\0cd");
	eret = snprintf(ref, sizeof(ref), "%s", "ab\0cd");
	pf_check(c, aret, ref, eret);
}

/*
** 4 KB, well past any fixed local buffer an implementation might have been
** tempted to copy the string into before writing it. `ref` is sized one byte
** past BRO_CAPTURE_MAX - see printf_assert.c - so snprintf never truncates a
** fixture this size, and the truncation is asserted against rather than
** trusted.
*/
static void	case_04(t_ctx *c)
{
	static char	s[BRO_CAPTURE_MAX + 1];
	char		ref[BRO_CAPTURE_MAX + 1];
	int		eret;
	int		aret;

	memset(s, 'a', sizeof(s) - 1);
	s[sizeof(s) - 1] = '\0';
	if (!pf_begin(c))
		return ;
	aret = ft_printf("%s", s);
	eret = snprintf(ref, sizeof(ref), "%s", s);
	if (eret >= (int)sizeof(ref))
		return (bro_fail(c->out,
				"engine: fixture too large for the oracle buffer"));
	pf_check(c, aret, ref, eret);
}

static void	case_05(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%s", "return equals length");
	eret = snprintf(ref, sizeof(ref), "%s", "return equals length");
	pf_check(c, aret, ref, eret);
}

static void	case_06(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("[%s]", "hi");
	eret = snprintf(ref, sizeof(ref), "[%s]", "hi");
	pf_check(c, aret, ref, eret);
}

static void	case_07(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%s and %s", "cats", "dogs");
	eret = snprintf(ref, sizeof(ref), "%s and %s", "cats", "dogs");
	pf_check(c, aret, ref, eret);
}

/*
** Undefined: the standard requires %s's argument to point at a string; NULL
** does not, and nothing says what happens next. glibc's own choice is
** "(null)" - defensive behaviour, not a rule ft_printf is bound by. This
** cannot use pf_check: bro_expect_bytes/bro_expect_num call bro_fail on a
** mismatch, which would overwrite the UB status harness.c already preset for
** a BRO_UB_CASE-flagged case (design A5) - so this records what happened
** through bro_mark_ub and asserts nothing. A crash here is still classified
** UB by the harness, the same as ft_putstr_fd's NULL case.
*/
static void	case_08(t_ctx *c)
{
	char	buf[BRO_CAPTURE_MAX + 1];
	size_t	len;
	int	aret;

	memset(buf, 0, sizeof(buf));
	if (!pf_begin(c))
		return ;
	aret = ft_printf("%s", NULL);
	len = bro_capture_take(c, 0, buf, sizeof(buf) - 1);
	memcpy(c->out->captured, buf,
		len < BRO_CAPTURE_MAX ? len : BRO_CAPTURE_MAX);
	c->out->captured_len = len;
	bro_mark_ub(c->out, "wrote %zu byte(s) (\"%s\"), returned %d - "
		"%%s(NULL) is undefined; glibc prints \"(null)\" and returns 6",
		len, buf, aret);
}

/*
** A NULL %s does not have to break the rest of the call - the subject-space
** point here is that the %d conversions before and after it stay fully
** defined. The engine scores one case at a time, not one conversion within
** it, so the whole call is UB even though only its middle conversion truly
** is; what is worth checking by eye is displayed, not asserted, same
** reasoning as case_08.
*/
static void	case_09(t_ctx *c)
{
	char	buf[BRO_CAPTURE_MAX + 1];
	size_t	len;
	int	aret;

	memset(buf, 0, sizeof(buf));
	if (!pf_begin(c))
		return ;
	aret = ft_printf("%d%s%d", 1, NULL, 2);
	len = bro_capture_take(c, 0, buf, sizeof(buf) - 1);
	memcpy(c->out->captured, buf,
		len < BRO_CAPTURE_MAX ? len : BRO_CAPTURE_MAX);
	c->out->captured_len = len;
	bro_mark_ub(c->out, "wrote %zu byte(s) (\"%s\"), returned %d - only the "
		"middle conversion (%%s with NULL) is undefined",
		len, buf, aret);
}

static void	case_10(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%s%s", "ab", "cde");
	eret = snprintf(ref, sizeof(ref), "%s%s", "ab", "cde");
	pf_check(c, aret, ref, eret);
}

static void	case_11(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%s", "x");
	eret = snprintf(ref, sizeof(ref), "%s", "x");
	pf_check(c, aret, ref, eret);
}

static void	case_12(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%s", "a\nb\tc");
	eret = snprintf(ref, sizeof(ref), "%s", "a\nb\tc");
	pf_check(c, aret, ref, eret);
}

/* 5 bytes: é is two UTF-8 bytes - %s copies bytes through unexamined. */
static void	case_13(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%s", "café");
	eret = snprintf(ref, sizeof(ref), "%s", "café");
	pf_check(c, aret, ref, eret);
}

/* the empty second string is zero bytes and no visible gap. */
static void	case_14(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%s%s", "foo", "");
	eret = snprintf(ref, sizeof(ref), "%s%s", "foo", "");
	pf_check(c, aret, ref, eret);
}

static void	case_15(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%s%s%s%s", "a", "b", "c", "d");
	eret = snprintf(ref, sizeof(ref), "%s%s%s%s", "a", "b", "c", "d");
	pf_check(c, aret, ref, eret);
}

/* the argument is opaque data, never re-scanned for '%'. */
static void	case_16(t_ctx *c)
{
	char	ref[BRO_CAPTURE_MAX];
	int	eret;
	int	aret;

	if (!pf_begin(c))
		return ;
	aret = ft_printf("%s", "100% sure");
	eret = snprintf(ref, sizeof(ref), "%s", "100% sure");
	pf_check(c, aret, ref, eret);
}

static const t_case	g_cases[] = {
{1, "ft_printf(\"%s\", \"Hello, 42!\")", BRO_ORACLE | BRO_CAPTURES_FD,
	case_01},
{2, "ft_printf(\"%s\", \"\")", BRO_ORACLE | BRO_CAPTURES_FD, case_02},
{3, "ft_printf(\"%s\", \"ab\\0cd\")", BRO_ORACLE | BRO_CAPTURES_FD, case_03},
{4, "ft_printf(\"%s\", <4096 bytes of 'a'>)", BRO_ORACLE | BRO_CAPTURES_FD,
	case_04},
{5, "ft_printf(\"%s\", \"return equals length\")",
	BRO_ORACLE | BRO_CAPTURES_FD, case_05},
{6, "ft_printf(\"[%s]\", \"hi\")", BRO_ORACLE | BRO_CAPTURES_FD, case_06},
{7, "ft_printf(\"%s and %s\", \"cats\", \"dogs\")",
	BRO_ORACLE | BRO_CAPTURES_FD, case_07},
{8, "ft_printf(\"%s\", NULL)", BRO_UB_CASE | BRO_CAPTURES_FD, case_08},
{9, "ft_printf(\"%d%s%d\", 1, NULL, 2)", BRO_UB_CASE | BRO_CAPTURES_FD,
	case_09},
{10, "ft_printf(\"%s%s\", \"ab\", \"cde\")", BRO_ORACLE | BRO_CAPTURES_FD,
	case_10},
{11, "ft_printf(\"%s\", \"x\")", BRO_ORACLE | BRO_CAPTURES_FD, case_11},
{12, "ft_printf(\"%s\", \"a\\nb\\tc\")", BRO_ORACLE | BRO_CAPTURES_FD,
	case_12},
{13, "ft_printf(\"%s\", \"caf\\303\\251\")", BRO_ORACLE | BRO_CAPTURES_FD,
	case_13},
{14, "ft_printf(\"%s%s\", \"foo\", \"\")", BRO_ORACLE | BRO_CAPTURES_FD,
	case_14},
{15, "ft_printf(\"%s%s%s%s\", \"a\", \"b\", \"c\", \"d\")",
	BRO_ORACLE | BRO_CAPTURES_FD, case_15},
{16, "ft_printf(\"%s\", \"100%% sure\")", BRO_ORACLE | BRO_CAPTURES_FD,
	case_16},
};

const t_suite	g_suite_pf_s = {
	"pf_s", 1, 2, 4, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
