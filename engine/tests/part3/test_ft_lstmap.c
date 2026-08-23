#include "bro.h"
#include "libft_proto.h"
#include <stdlib.h>
#include <string.h>

/*
** ft_lstmap - T5. Cases from _dev/SPEC_MICRO.md section 43.
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
};

const t_suite	g_suite_ft_lstmap = {
	"ft_lstmap", 3, 8, 5, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
