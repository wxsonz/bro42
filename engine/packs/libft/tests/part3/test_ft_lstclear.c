#include "bro.h"
#include "proto.h"
#include "pack.h"
#include <stdlib.h>

/*
** ft_lstclear - T5. Cases from _dev/plan/rank00/libft-01-cases.md section 41.
**
** The fixtures are built with the WRAPPED allocator on purpose: ft_lstclear
** takes ownership of them, so failing to free them has to show up as a leak.
** Engine scratch would be invisible to the accounting and would let a
** do-nothing lstclear pass.
*/

static void	clear_case(t_ctx *c, size_t n, bool check_del)
{
	t_list	*lst;
	size_t	before;

	lst = bro_list_build(n);
	if (n && !lst)
		return (bro_fail(c->out, "engine: could not build the fixture"));
	before = bro_del_calls();
	bro_ready(c);
	ft_lstclear(&lst, bro_del_counting);
	if (lst != NULL)
		return (bro_fail(c->out,
				"the caller's pointer still points at freed memory - "
				"it was not set to NULL"));
	if (check_del && bro_del_calls() - before != n)
		bro_fail(c->out, "del was called %zu time(s) for %zu node(s)",
			bro_del_calls() - before, n);
}

static void	case_01(t_ctx *c) { clear_case(c, 3, false); }
static void	case_02(t_ctx *c) { clear_case(c, 3, true); }
static void	case_03(t_ctx *c) { clear_case(c, 100, true); }
static void	case_04(t_ctx *c) { clear_case(c, 1, false); }

/* An empty list is a no-op, not an error, and must dereference nothing. */
static void	case_05(t_ctx *c)
{
	t_list	*lst;

	lst = NULL;
	ft_lstclear(&lst, bro_del_counting);
	if (lst != NULL)
		bro_fail(c->out, "an empty list must stay NULL");
}

/* content may legitimately be NULL; del is still called for every node. */
static void	case_06(t_ctx *c)
{
	t_list	*lst;
	t_list	*node;
	size_t	before;
	size_t	i;

	lst = NULL;
	i = 0;
	while (i++ < 3)
	{
		node = malloc(sizeof(t_list));
		if (!node)
			return (bro_fail(c->out, "engine: out of memory"));
		node->content = NULL;
		node->next = lst;
		lst = node;
	}
	before = bro_del_calls();
	bro_ready(c);
	ft_lstclear(&lst, bro_del_counting);
	if (lst != NULL)
		return (bro_fail(c->out, "the caller's pointer was not set to NULL"));
	if (bro_del_calls() - before != 3)
		bro_fail(c->out, "del was called %zu time(s) for 3 node(s)",
			bro_del_calls() - before);
}

static const t_case	g_cases[] = {
{1, "lst=n1->n2->n3, ft_lstclear(&lst, del)", BRO_INJECT, case_01},
{2, "lst=n1->n2->n3, ft_lstclear(&lst, del)", BRO_INJECT, case_02},
{3, "lst=<100 nodes>, ft_lstclear(&lst, del)", BRO_INJECT, case_03},
{4, "lst=n1, ft_lstclear(&lst, del)", BRO_INJECT, case_04},
{5, "lst=NULL, ft_lstclear(&lst, del)", 0, case_05},
{6, "lst=<nodes with NULL content>, ft_lstclear(&lst, del)", BRO_INJECT, case_06},
};

const t_suite	g_suite_ft_lstclear = {
	"ft_lstclear", 3, 8, 5, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
