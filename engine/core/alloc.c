#include "bro.h"
#include <stdlib.h>
#include <string.h>

/*
** Allocation accounting and failure injection (design/02_ENGINE.md, B2).
**
** Linked with -Wl,--wrap=malloc,--wrap=free,--wrap=calloc,--wrap=realloc, so
** the linker rewrites every undefined reference to malloc - INCLUDING the ones
** inside the student's libft.a - to __wrap_malloc, and exposes the original as
** __real_malloc. Probed working through a static archive before this was
** committed to: an injected failure returned NULL from inside the archive's
** own function and the live-block count returned to zero.
**
** Alternatives rejected: LD_PRELOAD does not apply to a statically linked
** archive; valgrind is slow and cannot inject failures; a custom allocator
** cannot reach inside the .a.
**
** THE RULE: engine bookkeeping calls __real_malloc directly. If the engine's
** own scratch counted as student allocation, every case would report a leak.
** Enforced by a grep in the self-test - no bare malloc under engine/core/ or
** engine/packs/<pack>/ except this file (and a pack's own list.c-equivalent,
** whose fixtures are deliberately student-visible allocation).
*/

# ifdef BRO_HAVE_WRAP

extern void	*__real_malloc(size_t n);
extern void	__real_free(void *p);
extern void	*__real_calloc(size_t n, size_t size);
extern void	*__real_realloc(void *p, size_t n);

/*
** Pointer -> size map, so __wrap_free knows how much to give back. Open
** addressed with linear probing; grown with __real_malloc, never a fixed
** array: a 100-word ft_split makes 101 allocations and a fixed table would
** overflow silently, corrupting every leak number after it (plan gap 1).
*/
typedef struct s_entry
{
	void	*ptr;
	size_t	size;
}	t_entry;

# define TOMBSTONE	((void *)-1)

static t_alloc	g_alloc;
static t_entry	*g_table;
static size_t	g_cap;
static size_t	g_used;

static size_t	hash_ptr(const void *p, size_t cap)
{
	uintptr_t	x;

	x = (uintptr_t)p;
	x = (x >> 4) * 0x9E3779B97F4A7C15ULL;
	return ((size_t)(x >> 32) & (cap - 1));
}

static void	table_put(t_entry *tab, size_t cap, void *p, size_t size)
{
	size_t	i;

	i = hash_ptr(p, cap);
	while (tab[i].ptr && tab[i].ptr != TOMBSTONE && tab[i].ptr != p)
		i = (i + 1) & (cap - 1);
	tab[i].ptr = p;
	tab[i].size = size;
}

static bool	table_grow(void)
{
	t_entry	*fresh;
	size_t	cap;
	size_t	i;

	cap = g_cap ? g_cap * 2 : 1024;
	fresh = __real_malloc(cap * sizeof(*fresh));
	if (!fresh)
		return (false);
	memset(fresh, 0, cap * sizeof(*fresh));
	i = 0;
	while (i < g_cap)
	{
		if (g_table[i].ptr && g_table[i].ptr != TOMBSTONE)
			table_put(fresh, cap, g_table[i].ptr, g_table[i].size);
		i++;
	}
	if (g_table)
		__real_free(g_table);
	g_table = fresh;
	g_cap = cap;
	return (true);
}

static void	record(void *p, size_t size)
{
	if (!g_table || (g_used + 1) * 10 >= g_cap * 7)
	{
		if (!table_grow())
			return ;
		g_used = 0;
	}
	table_put(g_table, g_cap, p, size);
	g_used++;
}

/* Returns the size and clears the slot; 0 means "not ours" - engine scratch
** allocated with __real_malloc, which must not move the counters. */
static size_t	forget(void *p)
{
	size_t	i;
	size_t	start;
	size_t	size;

	if (!g_table)
		return (0);
	i = hash_ptr(p, g_cap);
	start = i;
	while (g_table[i].ptr)
	{
		if (g_table[i].ptr == p)
		{
			size = g_table[i].size;
			g_table[i].ptr = TOMBSTONE;
			g_table[i].size = 0;
			return (size);
		}
		i = (i + 1) & (g_cap - 1);
		if (i == start)
			break ;
	}
	return (0);
}

void	*__wrap_malloc(size_t n)
{
	void	*p;

	if (!g_alloc.armed)
		return (__real_malloc(n));
	g_alloc.calls++;
	if (g_alloc.fail_after >= 0
		&& g_alloc.calls > (size_t)g_alloc.fail_after)
		return (NULL);
	p = __real_malloc(n);
	if (!p)
		return (NULL);
	g_alloc.live_blocks++;
	g_alloc.live_bytes += n;
	if (g_alloc.live_bytes > g_alloc.peak_bytes)
		g_alloc.peak_bytes = g_alloc.live_bytes;
	record(p, n);
	return (p);
}

void	__wrap_free(void *p)
{
	size_t	size;

	if (!p)
		return ;
	if (g_alloc.armed)
	{
		size = forget(p);
		if (size)
		{
			g_alloc.live_blocks--;
			g_alloc.live_bytes -= size;
		}
	}
	__real_free(p);
}

void	*__wrap_calloc(size_t n, size_t size)
{
	void	*p;

	if (n && size > (size_t)-1 / n)
		return (NULL);
	p = __wrap_malloc(n * size);
	if (p)
		memset(p, 0, n * size);
	return (p);
}

void	*__wrap_realloc(void *p, size_t n)
{
	void	*fresh;
	size_t	old;

	if (!g_alloc.armed)
		return (__real_realloc(p, n));
	fresh = __wrap_malloc(n);
	if (!fresh || !p)
		return (fresh);
	old = 0;
	if (g_table)
		old = forget(p);
	memcpy(fresh, p, old < n ? old : n);
	if (old)
	{
		g_alloc.live_blocks--;
		g_alloc.live_bytes -= old;
	}
	__real_free(p);
	return (fresh);
}

bool	bro_alloc_available(void)
{
	return (true);
}

void	bro_alloc_reset(void)
{
	memset(&g_alloc, 0, sizeof(g_alloc));
	g_alloc.fail_after = -1;
	if (g_table)
		memset(g_table, 0, g_cap * sizeof(*g_table));
	g_used = 0;
}

void	bro_alloc_arm(long fail_after)
{
	bro_alloc_reset();
	g_alloc.fail_after = fail_after;
	g_alloc.armed = true;
}

/*
** Records how many allocations the case made before the function under test
** was invoked. The harness uses it to decide WHICH absolute allocation indices
** to sweep - it no longer affects the failure decision itself.
**
** It used to: the check subtracted base from the running count. That was wrong,
** because base is only set once the fixture is already built, so at the moment
** a fixture allocated, base was still zero and a low failure index broke the
** test's own setup. bro.h promised fixture allocations "can never be the
** injected failure" and the code did not deliver it.
*/
void	bro_alloc_ready(void)
{
	g_alloc.base = g_alloc.calls;
}

void	bro_alloc_disarm(void)
{
	g_alloc.armed = false;
}

void	bro_alloc_snapshot(t_alloc *out)
{
	*out = g_alloc;
}

# else

/*
** No --wrap on this platform (Apple's ld64 has no equivalent). Injection and
** leak accounting turn off; the affected cases report SKIP with the reason.
** Every correctness test, crash classification and macro check is unaffected
** (decision A8).
*/
bool	bro_alloc_available(void) { return (false); }
void	bro_alloc_reset(void) { }
void	bro_alloc_arm(long f) { (void)f; }
/*
** No g_alloc on this path (it is the wrap branch's own static), so there is
** nothing to record base into. Harmless: with bro_alloc_available() false,
** harness.c's bro_run_case() reports SKIP for BRO_INJECT cases before ever
** calling this - it only exists so every entry point in bro.h still links.
*/
void	bro_alloc_ready(void) { }
void	bro_alloc_disarm(void) { }
void	bro_alloc_snapshot(t_alloc *out) { memset(out, 0, sizeof(*out)); }

# endif
