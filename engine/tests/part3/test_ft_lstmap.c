#include "bro.h"
#include "libft_proto.h"
#include <stdlib.h>
#include <string.h>

/*
** ft_lstmap - T5. Cases from _dev/plan/rank00/libft-01-cases.md section 43.
**
** The tier-model check: a structure of allocations (T3's ft_split) built on
** top of a caller-owned source list (T5's fixtures). dup_upper allocates its
** own content, so every node costs two allocations and the injection sweep
** exercises both.
**
** The source is a normal wrapped fixture: ft_lstmap never takes ownership of
** its input, so the case frees it itself. The injection sweep fails only
** ABSOLUTE allocation indices at or above the count recorded by bro_ready(),
** so the fixture's own allocations are never the ones broken.
**
** Cases 6-7 (the sweep) only prove 0 bytes stay live on failure - and
** free(content) satisfies that exactly as well as del(content) does, so a
** rollback that frees the orphaned content directly instead of routing it
** through del passes every injected index silently. harness.c's
** check_injected() is why a case-level bro_fail() cannot close this from
** inside the sweep either: for any BRO_INJECT case it OVERWRITES status/msg
** from scratch once c->run() returns, using only live_bytes and the tracked
** return value - a custom assertion made during that same run is discarded,
** not merely outvoted. Case 8 checks it a different way: it arms the
** allocator itself, outside BRO_INJECT, so nothing overwrites its result.
*/

static t_list	*build_src(void)
{
	return (bro_list_build(3));
}

static void	free_src(t_list *lst)
{
	bro_list_free(lst);
}

static void	*dup_upper(void *content)
{
	char	*s;
	char	*out;
	size_t	i;

	s = content;
	out = malloc(strlen(s) + 1);
	if (!out)
		return (NULL);
	i = 0;
	while (s[i])
	{
		if (s[i] >= 'a' && s[i] <= 'z')
			out[i] = s[i] - 'a' + 'A';
		else
			out[i] = s[i];
		i++;
	}
	out[i] = '\0';
	return (out);
}

static bool	matches_upper(const char *orig, const char *got)
{
	char	want;

	while (*orig)
	{
		want = *orig;
		if (want >= 'a' && want <= 'z')
			want = want - 'a' + 'A';
		if (*got++ != want)
			return (false);
		orig++;
	}
	return (*got == '\0');
}

static void	check_map_result(t_ctx *c, t_list *src, t_list *got,
		char before[3][32])
{
	t_list	*sw;
	t_list	*gw;
	size_t	i;

	if (!got)
		return (bro_fail(c->out, "returned NULL"));
	sw = src;
	gw = got;
	i = 0;
	while (sw && gw)
	{
		if (strcmp(sw->content, before[i]))
			bro_fail(c->out, "node %zu: the original list was modified", i);
		else if (gw == sw || gw->content == sw->content)
			bro_fail(c->out, "node %zu: not a distinct allocation", i);
		else if (!matches_upper(before[i], gw->content))
			bro_fail(c->out, "node %zu: content was not transformed", i);
		sw = sw->next;
		gw = gw->next;
		i++;
	}
	if (i != 3 || sw || gw)
		bro_fail(c->out, "expected two 3-node lists, matched %zu", i);
}

static void	full_map_case(t_ctx *c)
{
	t_list	*src;
	t_list	*got;
	t_list	*sw;
	char	before[3][32];
	size_t	i;

	src = build_src();
	if (!src)
		return (bro_fail(c->out, "engine: could not build the fixture"));
	i = 0;
	sw = src;
	while (sw)
	{
		strcpy(before[i++], sw->content);
		sw = sw->next;
	}
	bro_ready(c);
	got = ft_lstmap(src, dup_upper, bro_del_counting);
	bro_track(c, got);
	if (bro_injecting(c))
		return (free_src(src));
	check_map_result(c, src, got, before);
	free_src(src);
	bro_list_free(got);
}

/*
** free(content) instead of del(content) on the orphaned-content rollback
** path is invisible to the sweep above (see the file header): the block is
** genuinely freed, nothing leaks, nothing crashes. The only witness is
** whether del itself ran. This arms the allocator directly - outside
** BRO_INJECT - specifically so this case's own bro_fail() is what stands;
** check_injected() never runs for a case without that flag, so it never
** gets a chance to erase the verdict the way it would inside the sweep.
**
** A 1-node source pins the allocation order exactly: dup_upper's malloc is
** call 1 and always succeeds, ft_lstnew's is call 2 and is the one forced
** to fail, with nothing yet in the new list for ft_lstclear's rollback to
** walk - so a correct implementation calls del exactly once here, and a
** platform without --wrap (bro_alloc_available() false) leaves the call
** un-forced, which this treats as nothing to check rather than a failure.
*/
static void	case_08(t_ctx *c)
{
	t_list	*src;
	t_list	*got;
	size_t	del_before;

	src = bro_list_build(1);
	if (!src)
		return (bro_fail(c->out, "engine: could not build the fixture"));
	if (!bro_alloc_available())
		return (bro_list_free(src));
	del_before = bro_del_calls();
	bro_alloc_arm(1);
	got = ft_lstmap(src, dup_upper, bro_del_counting);
	bro_alloc_disarm();
	if (got != NULL)
	{
		bro_fail(c->out, "the second allocation was forced to fail, "
			"but a non-NULL result came back");
		bro_list_free(got);
	}
	else if (bro_del_calls() - del_before < 1)
		bro_fail(c->out, "the second allocation was forced to fail: the "
			"orphaned content was freed directly instead of through del");
	bro_list_free(src);
}

/* An empty source produces NULL - there is no such thing as an allocated
** empty list. */
static void	empty_map_case(t_ctx *c)
{
	t_list	*got;

	bro_ready(c);
	got = ft_lstmap(NULL, dup_upper, bro_del_counting);
	bro_track(c, got);
	if (bro_injecting(c))
		return ;
	if (got != NULL)
		bro_fail(c->out, "expected NULL for an empty source");
}

static void	case_01(t_ctx *c) { full_map_case(c); }
static void	case_02(t_ctx *c) { full_map_case(c); }
static void	case_03(t_ctx *c) { full_map_case(c); }
static void	case_04(t_ctx *c) { full_map_case(c); }
static void	case_05(t_ctx *c) { empty_map_case(c); }
static void	case_06(t_ctx *c) { full_map_case(c); }
static void	case_07(t_ctx *c) { full_map_case(c); }

static const t_case	g_cases[] = {
{1, "lstmap(n1->n2->n3, <dup+upper>, del)", BRO_INJECT, case_01},
{2, "lstmap(n1->n2->n3, f, del)", BRO_INJECT, case_02},
{3, "lstmap(n1->n2->n3, f, del)", BRO_INJECT, case_03},
{4, "lstmap(n1->n2->n3, f, del)", BRO_INJECT, case_04},
{5, "lstmap(NULL, f, del)", BRO_INJECT, case_05},
{6, "lstmap(n1->n2->n3, f, del)", BRO_INJECT, case_06},
{7, "lstmap(n1->n2->n3, f, del)", BRO_INJECT, case_07},
{8, "lstmap(n1, f, del), 2nd allocation forced to fail", 0, case_08},
};

const t_suite	g_suite_ft_lstmap = {
	"ft_lstmap", 3, 8, 5, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
