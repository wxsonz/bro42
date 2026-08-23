#include "bro.h"
#include <string.h>
#include <stdio.h>

/*
** Expectation helpers. Each records enough for the renderer to draw evidence
** without knowing anything about the function under test.
**
** THE FIRST FAILURE OWNS THE EVIDENCE. A case may assert more than one thing -
** ft_strlcat checks the buffer contents and then the return value - and each
** helper overwrites the evidence fields. Without this guard, a case whose
** BUFFER is wrong but whose return value is right displayed
**
**     buffers differ at index 5
**     expected 11    got 11
**
** which is two lines that contradict each other, with the byte grid thrown
** away in favour of a scalar comparison that passed.
*/
static bool	already_failed(const t_result *r)
{
	return (r->status != BRO_OK && r->status != BRO_UB);
}

void	bro_expect_num(t_result *r, long long expected, long long actual)
{
	if (already_failed(r))
		return ;
	r->kind = EV_SCALAR;
	r->expected_num = expected;
	r->actual_num = actual;
	if (expected != actual)
		bro_fail(r, "expected %lld, got %lld", expected, actual);
}

/*
** Comparison functions specify the SIGN of the result and nothing more, so
** testing for an exact 1 or -1 is what makes other testers wrong
** (libft-01-cases.md ft_strncmp / ft_memcmp, concept `return-contract`).
*/
static int	sign_of(long long v)
{
	if (v < 0)
		return (-1);
	return (v > 0);
}

void	bro_expect_sign(t_result *r, long long expected, long long actual)
{
	if (already_failed(r))
		return ;
	r->kind = EV_SCALAR;
	r->expected_num = expected;
	r->actual_num = actual;
	if (sign_of(expected) == sign_of(actual))
		return ;
	bro_fail(r, "expected a %s result, got %lld",
		sign_of(expected) < 0 ? "negative"
		: (sign_of(expected) > 0 ? "positive" : "zero"), actual);
}

static long	first_difference(const unsigned char *a, const unsigned char *b,
			size_t n)
{
	size_t	i;

	i = 0;
	while (i < n)
	{
		if (a[i] != b[i])
			return ((long)i);
		i++;
	}
	return (-1);
}

/*
** Byte windows are capped at BRO_EVID_MAX and centred on the divergence, so a
** long buffer still shows the part that matters.
*/
void	bro_expect_bytes(t_result *r, const void *expected, const void *actual,
		size_t n)
{
	long	diff;
	size_t	base;

	if (already_failed(r))
		return ;
	diff = first_difference(expected, actual, n);
	base = 0;
	if (diff >= 0 && (size_t)diff > BRO_EVID_MAX / 2)
		base = (size_t)diff - BRO_EVID_MAX / 2;
	r->kind = EV_BYTES;
	r->nbytes = n - base;
	if (r->nbytes > BRO_EVID_MAX)
		r->nbytes = BRO_EVID_MAX;
	r->window_base = base;
	r->diverge = diff;
	memcpy(r->expected_b, (const unsigned char *)expected + base, r->nbytes);
	memcpy(r->actual_b, (const unsigned char *)actual + base, r->nbytes);
	if (diff >= 0)
		bro_fail(r, "buffers differ at index %ld", diff);
}

/*
** Pointer returns are compared as OFFSETS into the input: the oracle and the
** student may be handed different buffers, so raw addresses are meaningless
** (design/04_TESTDESIGN.md).
*/
void	bro_expect_offset(t_result *r, const void *base, const void *expected,
		const void *actual)
{
	if (already_failed(r))
		return ;
	r->kind = EV_OFFSET;
	r->expected_num = expected ? (long long)((const char *)expected
			- (const char *)base) : -1;
	r->actual_num = actual ? (long long)((const char *)actual
			- (const char *)base) : -1;
	if (r->expected_num == r->actual_num)
		return ;
	if (r->actual_num < 0)
		bro_fail(r, "expected &s[%lld], got NULL", r->expected_num);
	else if (r->expected_num < 0)
		bro_fail(r, "expected NULL, got &s[%lld]", r->actual_num);
	else
		bro_fail(r, "expected &s[%lld], got &s[%lld]",
			r->expected_num, r->actual_num);
}
