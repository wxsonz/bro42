#include "bro.h"
#include "libft_proto.h"
#include <string.h>
#include <unistd.h>

/*
** ft_putchar_fd - T4. Cases from _dev/plan/rank00/libft-01-cases.md section 31.
**
** No return value, so the only observable is what reached the descriptor.
** '\0' is a legitimate byte here, not a terminator - the comparison is by
** exact byte, never by strlen.
*/

static void	putchar_case(t_ctx *c, char ch, unsigned char expected)
{
	char	buf[BRO_CAPTURE_MAX];
	size_t	len;

	if (!bro_capture_bind(c, 0, STDOUT_FILENO))
		return (bro_fail(c->out, "engine: could not capture stdout"));
	ft_putchar_fd(ch, STDOUT_FILENO);
	len = bro_capture_take(c, 0, buf, sizeof(buf) - 1);
	if (len != 1 || (unsigned char)buf[0] != expected)
		bro_fail(c->out, "expected exactly the byte 0x%02X, got %zu byte(s)",
			expected, len);
}

/*
** fd is a parameter for a reason: writing to 1 unconditionally passes any
** test that only looks at stdout and fails the moment fd 2 is asked for.
*/
static void	case_04(t_ctx *c)
{
	char	out[BRO_CAPTURE_MAX];
	char	err[BRO_CAPTURE_MAX];
	size_t	nout;
	size_t	nerr;

	if (!bro_capture_bind(c, 0, STDOUT_FILENO)
		|| !bro_capture_bind(c, 1, STDERR_FILENO))
		return (bro_fail(c->out, "engine: could not capture descriptors"));
	ft_putchar_fd('A', STDERR_FILENO);
	nout = bro_capture_take(c, 0, out, sizeof(out) - 1);
	nerr = bro_capture_take(c, 1, err, sizeof(err) - 1);
	if (nout != 0)
		return (bro_fail(c->out,
				"wrote %zu byte(s) to fd 1 when asked for fd 2", nout));
	if (nerr != 1 || (unsigned char)err[0] != 0x41)
		bro_fail(c->out, "expected the byte 0x41 on fd 2, got %zu byte(s)",
			nerr);
}

static void	case_01(t_ctx *c) { putchar_case(c, 'A', 0x41); }
static void	case_02(t_ctx *c) { putchar_case(c, '\n', 0x0A); }
static void	case_03(t_ctx *c) { putchar_case(c, '\0', 0x00); }

static const t_case	g_cases[] = {
{1, "ft_putchar_fd('A', fd)", BRO_CAPTURES_FD, case_01},
{2, "ft_putchar_fd('\\n', fd)", BRO_CAPTURES_FD, case_02},
{3, "ft_putchar_fd('\\0', fd)", BRO_CAPTURES_FD, case_03},
{4, "ft_putchar_fd('A', 2)", BRO_CAPTURES_FD, case_04},
};

const t_suite	g_suite_ft_putchar_fd = {
	"ft_putchar_fd", 2, 7, 4, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
