#include "bro.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/*
** Engine-owned scratch: guard buffers, oracle reference buffers, the size
** table. It goes STRAIGHT to the real allocator, bypassing the interposition
** in alloc.c - if the engine's own memory counted as student allocation,
** every case would report a leak (design/02_ENGINE.md).
**
** Memory the student is expected to free - list fixtures - deliberately does
** NOT come through here. See list.c.
*/
# ifdef BRO_HAVE_WRAP
extern void	*__real_malloc(size_t n);
extern void	__real_free(void *p);

void	*bro_alloc(size_t n)
{
	return (__real_malloc(n));
}

void	bro_free(void *p)
{
	__real_free(p);
}
# else

void	*bro_alloc(size_t n)
{
	return (malloc(n));
}

void	bro_free(void *p)
{
	free(p);
}
# endif

void	bro_track(t_ctx *c, void *p)
{
	c->returned = p;
}

bool	bro_injecting(const t_ctx *c)
{
	return (c->out->fail_after >= 0);
}

void	bro_ready(t_ctx *c)
{
	(void)c;
	bro_alloc_ready();
}

const char	*bro_status_name(t_status s)
{
	static const char	*names[] = {
		"OK", "KO", "SIGSEGV", "SIGBUS", "SIGABRT",
		"TIMEOUT", "LEAK", "UB", "MISSING", "SKIP"
	};

	if ((size_t)s >= sizeof(names) / sizeof(*names))
		return ("KO");
	return (names[s]);
}

bool	bro_status_is_scored(t_status s)
{
	return (s != BRO_UB && s != BRO_MISSING && s != BRO_SKIP);
}

static void	set_msg(t_result *r, const char *fmt, va_list ap)
{
	vsnprintf(r->msg, sizeof(r->msg), fmt, ap);
}

void	bro_fail(t_result *r, const char *fmt, ...)
{
	va_list	ap;

	r->status = BRO_KO;
	va_start(ap, fmt);
	set_msg(r, fmt, ap);
	va_end(ap);
}

void	bro_mark_ub(t_result *r, const char *fmt, ...)
{
	va_list	ap;

	r->status = BRO_UB;
	va_start(ap, fmt);
	set_msg(r, fmt, ap);
	va_end(ap);
}
