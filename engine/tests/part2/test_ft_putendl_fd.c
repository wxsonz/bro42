#include "bro.h"
#include "libft_proto.h"
#include <string.h>
#include <unistd.h>

/*
** ft_putendl_fd - T4. Cases from _dev/SPEC_MICRO.md section 33.
**
** putendl is putstr plus exactly one '\n' - always one, appended
** unconditionally, even when the string already ends in a newline (case 3:
** two newlines out, not one collapsed).
*/

static void	endl_case(t_ctx *c, char *s, const char *expected, size_t want)
{
	char	buf[BRO_CAPTURE_MAX];
	size_t	len;

	if (!bro_capture_bind(c, 0, STDOUT_FILENO))
		return (bro_fail(c->out, "engine: could not capture stdout"));
	ft_putendl_fd(s, STDOUT_FILENO);
	len = bro_capture_take(c, 0, buf, sizeof(buf) - 1);
	buf[len] = '\0';
	if (len != want || memcmp(buf, expected, want))
		bro_fail(c->out, "expected %zu byte(s) (\"%s\"), got %zu (\"%s\")",
			want, expected, len, buf);
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
	ft_putendl_fd("err", STDERR_FILENO);
	nout = bro_capture_take(c, 0, out, sizeof(out) - 1);
	nerr = bro_capture_take(c, 1, err, sizeof(err) - 1);
	if (nout != 0)
		return (bro_fail(c->out,
				"wrote %zu byte(s) to fd 1 when asked for fd 2", nout));
	if (nerr != 4 || memcmp(err, "err\n", 4))
		bro_fail(c->out, "expected \"err\\n\" on fd 2, got %zu byte(s)", nerr);
}

static void	case_01(t_ctx *c) { endl_case(c, "Hello 42", "Hello 42\n", 9); }
static void	case_02(t_ctx *c) { endl_case(c, "", "\n", 1); }
static void	case_03(t_ctx *c) { endl_case(c, "a\n", "a\n\n", 3); }

static const t_case	g_cases[] = {
{1, "ft_putendl_fd(\"Hello 42\", fd)", BRO_CAPTURES_FD, case_01},
{2, "ft_putendl_fd(\"\", fd)", BRO_CAPTURES_FD, case_02},
{3, "ft_putendl_fd(\"a\\n\", fd)", BRO_CAPTURES_FD, case_03},
{4, "ft_putendl_fd(\"err\", 2)", BRO_CAPTURES_FD, case_04},
};

const t_suite	g_suite_ft_putendl_fd = {
	"ft_putendl_fd", 2, 7, 4, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
