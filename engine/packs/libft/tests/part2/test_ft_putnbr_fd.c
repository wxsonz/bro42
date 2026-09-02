#include "bro.h"
#include "proto.h"
#include <limits.h>
#include <string.h>
#include <unistd.h>

/*
** ft_putnbr_fd - T4. Cases from _dev/plan/rank00/libft-01-cases.md section 34.
**
** No return value, so the only observable is what reached the descriptor.
** Every case binds a real fd to a pipe and reads back exactly what was
** written - which is also how the fd-is-a-parameter case works.
*/

static void	nbr_case(t_ctx *c, int n, const char *expected)
{
	char	buf[BRO_CAPTURE_MAX];
	size_t	len;

	if (!bro_capture_bind(c, 0, STDOUT_FILENO))
		return (bro_fail(c->out, "engine: could not capture stdout"));
	ft_putnbr_fd(n, STDOUT_FILENO);
	len = bro_capture_take(c, 0, buf, sizeof(buf) - 1);
	buf[len] = '\0';
	memcpy(c->out->captured, buf, len < BRO_CAPTURE_MAX ? len : 0);
	c->out->captured_len = len;
	if (len != strlen(expected) || memcmp(buf, expected, len))
		bro_fail(c->out, "expected \"%s\", got \"%s\"", expected, buf);
}

/*
** fd is a parameter for a reason: writing to 1 unconditionally passes every
** test that only looks at stdout, and fails the moment an evaluator asks for
** stderr.
*/
static void	case_08(t_ctx *c)
{
	char	out[BRO_CAPTURE_MAX];
	char	err[BRO_CAPTURE_MAX];
	size_t	nout;
	size_t	nerr;

	if (!bro_capture_bind(c, 0, STDOUT_FILENO)
		|| !bro_capture_bind(c, 1, STDERR_FILENO))
		return (bro_fail(c->out, "engine: could not capture descriptors"));
	ft_putnbr_fd(-42, STDERR_FILENO);
	nout = bro_capture_take(c, 0, out, sizeof(out) - 1);
	nerr = bro_capture_take(c, 1, err, sizeof(err) - 1);
	err[nerr] = '\0';
	if (nout != 0)
		return (bro_fail(c->out,
				"wrote %zu byte(s) to fd 1 when asked for fd 2", nout));
	if (nerr != 3 || memcmp(err, "-42", 3))
		bro_fail(c->out, "expected \"-42\" on fd 2, got \"%s\"", err);
}

static void	case_01(t_ctx *c) { nbr_case(c, 0, "0"); }
static void	case_02(t_ctx *c) { nbr_case(c, 42, "42"); }
static void	case_03(t_ctx *c) { nbr_case(c, -42, "-42"); }
static void	case_04(t_ctx *c) { nbr_case(c, -1, "-1"); }
static void	case_05(t_ctx *c) { nbr_case(c, 10, "10"); }
static void	case_06(t_ctx *c) { nbr_case(c, INT_MAX, "2147483647"); }
static void	case_07(t_ctx *c) { nbr_case(c, INT_MIN, "-2147483648"); }

static const t_case	g_cases[] = {
{1, "ft_putnbr_fd(0, fd)", BRO_CAPTURES_FD, case_01},
{2, "ft_putnbr_fd(42, fd)", BRO_CAPTURES_FD, case_02},
{3, "ft_putnbr_fd(-42, fd)", BRO_CAPTURES_FD, case_03},
{4, "ft_putnbr_fd(-1, fd)", BRO_CAPTURES_FD, case_04},
{5, "ft_putnbr_fd(10, fd)", BRO_CAPTURES_FD, case_05},
{6, "ft_putnbr_fd(2147483647, fd)", BRO_CAPTURES_FD, case_06},
{7, "ft_putnbr_fd(-2147483648, fd)", BRO_CAPTURES_FD, case_07},
{8, "ft_putnbr_fd(-42, 2)", BRO_CAPTURES_FD, case_08},
};

const t_suite	g_suite_ft_putnbr_fd = {
	"ft_putnbr_fd", 2, 7, 4, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
