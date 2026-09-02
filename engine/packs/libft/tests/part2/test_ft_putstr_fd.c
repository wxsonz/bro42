#include "bro.h"
#include "proto.h"
#include <string.h>
#include <unistd.h>

/*
** ft_putstr_fd - T4. Cases from _dev/plan/rank00/libft-01-cases.md section 32.
**
** The terminator is not part of the output: an 8-character string produces
** exactly 8 bytes, and an empty string produces none at all.
*/

static void	str_case(t_ctx *c, char *s)
{
	char	buf[BRO_CAPTURE_MAX];
	size_t	len;
	size_t	want;

	want = strlen(s);
	if (!bro_capture_bind(c, 0, STDOUT_FILENO))
		return (bro_fail(c->out, "engine: could not capture stdout"));
	ft_putstr_fd(s, STDOUT_FILENO);
	len = bro_capture_take(c, 0, buf, sizeof(buf) - 1);
	buf[len] = '\0';
	if (len != want || memcmp(buf, s, want))
		bro_fail(c->out, "expected %zu byte(s) (\"%s\"), got %zu (\"%s\")",
			want, s, len, buf);
}

/* fd is a parameter for a reason: same as the rest of the family. */
static void	case_04(t_ctx *c)
{
	char	out[BRO_CAPTURE_MAX];
	char	err[BRO_CAPTURE_MAX];
	size_t	nout;
	size_t	nerr;

	if (!bro_capture_bind(c, 0, STDOUT_FILENO)
		|| !bro_capture_bind(c, 1, STDERR_FILENO))
		return (bro_fail(c->out, "engine: could not capture descriptors"));
	ft_putstr_fd("err", STDERR_FILENO);
	nout = bro_capture_take(c, 0, out, sizeof(out) - 1);
	nerr = bro_capture_take(c, 1, err, sizeof(err) - 1);
	if (nout != 0)
		return (bro_fail(c->out,
				"wrote %zu byte(s) to fd 1 when asked for fd 2", nout));
	if (nerr != 3 || memcmp(err, "err", 3))
		bro_fail(c->out, "expected \"err\" on fd 2, got %zu byte(s)", nerr);
}

/*
** Undefined: the subject does not define ft_putstr_fd(NULL, fd), and neither
** does write. Some implementations print "(null)", some crash, some do
** nothing - all three are defensible, so this only records what happened.
** A crash is still classified UB by the harness (design A5); no libc
** function is called with a literal NULL here, so the -Werror=nonnull trap
** that other UB cases hit does not apply.
*/
static void	case_05(t_ctx *c)
{
	char	buf[BRO_CAPTURE_MAX];
	size_t	len;

	if (!bro_capture_bind(c, 0, STDOUT_FILENO))
		return (bro_fail(c->out, "engine: could not capture stdout"));
	ft_putstr_fd(NULL, STDOUT_FILENO);
	len = bro_capture_take(c, 0, buf, sizeof(buf) - 1);
	buf[len] = '\0';
	bro_mark_ub(c->out,
		"wrote %zu byte(s) (\"%s\") for a NULL string - the subject leaves "
		"this undefined", len, buf);
}

static void	case_01(t_ctx *c) { str_case(c, "Hello 42"); }
static void	case_02(t_ctx *c) { str_case(c, ""); }
static void	case_03(t_ctx *c) { str_case(c, "a\nb"); }

static const t_case	g_cases[] = {
{1, "ft_putstr_fd(\"Hello 42\", fd)", BRO_CAPTURES_FD, case_01},
{2, "ft_putstr_fd(\"\", fd)", BRO_CAPTURES_FD, case_02},
{3, "ft_putstr_fd(\"a\\nb\", fd)", BRO_CAPTURES_FD, case_03},
{4, "ft_putstr_fd(\"err\", 2)", BRO_CAPTURES_FD, case_04},
{5, "ft_putstr_fd(NULL, fd)", BRO_UB_CASE, case_05},
};

const t_suite	g_suite_ft_putstr_fd = {
	"ft_putstr_fd", 2, 7, 4, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
