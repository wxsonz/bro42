#include "bro.h"
#include <string.h>

/*
** Scratch owned by the context, so no case allocates for itself. Once alloc.c
** interposes on malloc in stage B, any allocation a case made on its own would
** show up as a student leak.
*/

t_buf	*bro_ctx_dst(t_ctx *c, size_t len)
{
	bro_guard_free(&c->dst);
	if (!bro_guard_alloc(&c->dst, len))
		return (NULL);
	return (&c->dst);
}

/*
** The oracle's destination. Separate memory from ctx->dst by construction:
** if libc and the student wrote to one buffer, every T1 case would compare a
** buffer against itself and pass (design/02_ENGINE.md, gap 4).
*/
t_buf	*bro_ctx_ref(t_ctx *c, size_t len)
{
	bro_guard_free(&c->ref);
	if (!bro_guard_alloc(&c->ref, len))
		return (NULL);
	return (&c->ref);
}

void	*bro_ctx_src(t_ctx *c, const void *data, size_t len)
{
	bro_guard_free(&c->src);
	if (!bro_guard_alloc(&c->src, len))
		return (NULL);
	memcpy(c->src.ptr, data, len);
	return (c->src.ptr);
}

void	bro_ctx_reset(t_ctx *c)
{
	bro_guard_free(&c->dst);
	bro_guard_free(&c->ref);
	bro_guard_free(&c->src);
}
