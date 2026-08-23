#include "bro.h"
#include <string.h>
#include <stdio.h>
#include <limits.h>

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

/*
** Sweeps. Two forms - a contiguous range, and an explicit list of integers -
** and two meanings, chosen by whether the function classifies or converts.
**
** Every sweep records BOTH what the student's function did and what the
** oracle did. The report colours by the student's answer, so the grid shows
** the character class itself and stays worth reading when everything passes;
** the oracle's answer is what turns a difference into a failure. A grid that
** only showed pass/fail would be a wall of green on correct code, which
** teaches nothing.
**
** The whole range is always walked, never stopping at the first difference:
** "every byte above 127" and "one byte past z" are different bugs, and a loop
** that stopped early would report them identically.
*/

static void	sweep_begin(t_result *r, size_t n, int scored)
{
	r->sweep_n = 0;
	r->sweep_scored = scored;
	(void)n;
}

static void	sweep_put(t_result *r, long v, int mine, int theirs)
{
	if (r->sweep_n >= BRO_SWEEP_MAX)
		return ;
	r->sweep_val[r->sweep_n] = v;
	r->sweep[r->sweep_n] = (unsigned char)(mine != 0);
	r->sweep_ref[r->sweep_n] = (unsigned char)(theirs != 0);
	r->sweep_n++;
}

static void	sweep_verdict(t_result *r, long bad, int mine, int theirs)
{
	if (!r->sweep_scored)
		return (bro_mark_ub(r, "%zu value(s) outside the range C defines "
				"for this function - shown, not graded",
				r->sweep_n));
	if (bad != LONG_MIN)
		bro_fail(r, "at %ld: expected %d, got %d", bad, theirs, mine);
}

void	bro_sweep_class(t_result *r, int (*fn)(int), int (*ref)(int),
		long lo, long hi, int scored)
{
	long	v;
	long	bad;

	if (hi - lo + 1 > (long)BRO_SWEEP_MAX)
		return (bro_fail(r, "range [%ld, %ld] needs %ld slots, "
				"BRO_SWEEP_MAX is %d - widen it rather than "
				"testing less than the case claims",
				lo, hi, hi - lo + 1, BRO_SWEEP_MAX));
	bad = LONG_MIN;
	sweep_begin(r, (size_t)(hi - lo + 1), scored);
	v = lo;
	while (v <= hi)
	{
		sweep_put(r, v, fn((int)v) != 0, scored && ref((int)v) != 0);
		if (scored && bad == LONG_MIN
			&& r->sweep[r->sweep_n - 1] != r->sweep_ref[r->sweep_n - 1])
			bad = v;
		v++;
	}
	if (bad != LONG_MIN)
		return (sweep_verdict(r, bad, fn((int)bad) != 0, ref((int)bad) != 0));
	sweep_verdict(r, bad, 0, 0);
}

/*
** The mapper form. "Did this byte change?" is the readable question for
** toupper and tolower - it lights exactly the letters that get converted -
** while correctness still compares the returned VALUES, not the flags.
*/
void	bro_sweep_map(t_result *r, int (*fn)(int), int (*ref)(int),
		long lo, long hi, int scored)
{
	long	v;
	long	bad;

	if (hi - lo + 1 > (long)BRO_SWEEP_MAX)
		return (bro_fail(r, "range [%ld, %ld] needs %ld slots, "
				"BRO_SWEEP_MAX is %d - widen it rather than "
				"testing less than the case claims",
				lo, hi, hi - lo + 1, BRO_SWEEP_MAX));
	bad = LONG_MIN;
	sweep_begin(r, (size_t)(hi - lo + 1), scored);
	v = lo;
	while (v <= hi)
	{
		sweep_put(r, v, fn((int)v) != (int)v,
			scored && ref((int)v) != (int)v);
		if (scored && bad == LONG_MIN && fn((int)v) != ref((int)v))
			bad = v;
		v++;
	}
	if (bad != LONG_MIN)
		return (sweep_verdict(r, bad, fn((int)bad), ref((int)bad)));
	sweep_verdict(r, bad, 0, 0);
}

/*
** An explicit list instead of a range: the notable integers worth showing
** beside the ASCII grid. EOF and the high bytes are defined and are checked;
** anything past unsigned char is undefined, so it is shown and not graded.
*/
/*
** The mapper form of the same list. Split rather than parameterised so a call
** site cannot silently pick the wrong question: asking "is it non-zero" of
** toupper lights every value, which looks like data and means nothing.
*/
void	bro_sweep_ints_map(t_result *r, int (*fn)(int), int (*ref)(int),
		const long *vals, size_t n, int scored)
{
	size_t	i;
	long	bad;

	bad = LONG_MIN;
	sweep_begin(r, n, scored);
	i = 0;
	while (i < n)
	{
		sweep_put(r, vals[i], fn((int)vals[i]) != (int)vals[i],
			scored && ref((int)vals[i]) != (int)vals[i]);
		if (scored && bad == LONG_MIN
			&& fn((int)vals[i]) != ref((int)vals[i]))
			bad = vals[i];
		i++;
	}
	if (bad != LONG_MIN)
		return (sweep_verdict(r, bad, fn((int)bad), ref((int)bad)));
	sweep_verdict(r, bad, 0, 0);
}

void	bro_sweep_ints(t_result *r, int (*fn)(int), int (*ref)(int),
		const long *vals, size_t n, int scored)
{
	size_t	i;
	long	bad;

	bad = LONG_MIN;
	sweep_begin(r, n, scored);
	i = 0;
	while (i < n)
	{
		sweep_put(r, vals[i], fn((int)vals[i]) != 0,
			scored && ref((int)vals[i]) != 0);
		if (scored && bad == LONG_MIN
			&& r->sweep[r->sweep_n - 1] != r->sweep_ref[r->sweep_n - 1])
			bad = vals[i];
		i++;
	}
	if (bad != LONG_MIN)
		return (sweep_verdict(r, bad, fn((int)bad) != 0, ref((int)bad) != 0));
	sweep_verdict(r, bad, 0, 0);
}
