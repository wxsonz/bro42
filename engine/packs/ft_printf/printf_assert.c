#include "printf_assert.h"
#include <string.h>
#include <unistd.h>

/*
** Every ft_printf case asserts two independent things: the bytes that
** reached the descriptor, and the return value. The return-count bug is the
** classic ft_printf defect (testing philosophy point 5,
** _dev/plan/rank01/ftprintf-01-cases.md) and must not hide behind output
** that merely looks right, which is why one helper checks both instead of
** each case rolling its own comparison.
*/

bool	pf_begin(t_ctx *c)
{
	if (bro_capture_bind(c, 0, STDOUT_FILENO))
		return (true);
	bro_fail(c->out, "engine: could not capture stdout");
	return (false);
}

/*
** buf is BRO_CAPTURE_MAX + 1, not BRO_CAPTURE_MAX: bro_capture_take's `max`
** argument is how many bytes it is willing to read, and the largest case in
** this pack (a 4096-byte %s, pf_s:04) needs all BRO_CAPTURE_MAX bytes
** captured - sizing buf at exactly BRO_CAPTURE_MAX and passing sizeof(buf)-1,
** the way the fd-only exemplars do, would silently cap that case one byte
** short of the fixture it is supposed to exercise.
**
** Zeroing buf first, rather than NUL-terminating only at `len`, means a
** short write compares its missing tail against real zero bytes instead of
** whatever garbage the stack held - the mismatch still lands on the first
** byte that was never written, which is where it belongs.
**
** expected_len bytes are compared, never strlen(expected): %c with '\0'
** writes one real NUL byte and returns 1 - a strlen-based comparison would
** see an empty string on both sides and miss a writer that wrote nothing at
** all (pf_c:02, the case this rule exists for).
**
** Bytes are checked before the return (bro_expect_bytes, then
** bro_expect_num) so a case that gets both wrong shows the byte evidence,
** which is what actually explains the failure - "first failure owns the
** evidence" (engine/core/assert.c).
*/
void	pf_check(t_ctx *c, int actual_ret, const char *expected,
		int expected_len)
{
	char	buf[BRO_CAPTURE_MAX + 1];
	size_t	len;

	memset(buf, 0, sizeof(buf));
	len = bro_capture_take(c, 0, buf, sizeof(buf) - 1);
	memcpy(c->out->captured, buf,
		len < BRO_CAPTURE_MAX ? len : BRO_CAPTURE_MAX);
	c->out->captured_len = len;
	bro_expect_bytes(c->out, expected, buf, (size_t)expected_len);
	bro_expect_num(c->out, expected_len, actual_ret);
}
