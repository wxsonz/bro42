#include "bro.h"
#include <string.h>

/*
** Canary buffers (design/02_ENGINE.md, decision B3).
**
** Comparing return values cannot see a function that writes one byte too far:
** ft_memset writing n+1 produces the right visible result AND the right return
** value. Only a poisoned byte past the end catches it.
**
** Layout:
**   [ 16 x 0x5A ][ len x 0xAA payload ][ 16 x 0x5A ]
**                ^ b->ptr, what the student's function receives
**
** These buffers are ENGINE-owned: bro_alloc, never the student's allocator,
** so they can never appear in leak accounting once stage B lands.
*/

bool	bro_guard_alloc(t_buf *b, size_t len)
{
	size_t	total;

	total = len + 2 * BRO_CANARY_LEN;
	b->mem = bro_alloc(total);
	if (!b->mem)
		return (false);
	memset(b->mem, BRO_CANARY, total);
	b->ptr = b->mem + BRO_CANARY_LEN;
	memset(b->ptr, BRO_POISON, len);
	b->len = len;
	b->guarded = true;
	return (true);
}

void	bro_guard_free(t_buf *b)
{
	if (b->mem)
		bro_free(b->mem);
	b->mem = NULL;
	b->ptr = NULL;
	b->len = 0;
	b->guarded = false;
}

static long	scan_canary(const unsigned char *p, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n)
	{
		if (p[i] != BRO_CANARY)
			return ((long)i);
		i++;
	}
	return (-1);
}

/*
** How far the function actually wrote: the last byte still holding poison
** marks the end of what it touched. Feeds the "wrote N bytes past the end"
** line in the renderer.
*/
long	bro_guard_extent(const t_buf *b)
{
	size_t	i;

	if (!b->guarded)
		return (-1);
	i = b->len;
	while (i > 0)
	{
		if (b->ptr[i - 1] != BRO_POISON)
			return ((long)i);
		i--;
	}
	return (0);
}

bool	bro_guard_check(const t_buf *b, t_result *r)
{
	long	before;
	long	after;

	if (!b->guarded)
		return (true);
	before = scan_canary(b->mem, BRO_CANARY_LEN);
	after = scan_canary(b->ptr + b->len, BRO_CANARY_LEN);
	if (before < 0 && after < 0)
		return (true);
	r->kind = EV_GUARD;
	r->write_extent = bro_guard_extent(b);
	if (after >= 0)
		bro_fail(r, "wrote %ld byte(s) past the end of a %zu-byte buffer",
			after + 1, b->len);
	else
		bro_fail(r, "wrote %ld byte(s) before the start of the buffer",
			BRO_CANARY_LEN - before);
	return (false);
}
