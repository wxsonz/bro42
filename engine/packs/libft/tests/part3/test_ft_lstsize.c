#include "bro.h"
#include "proto.h"
#include "pack.h"
#include <stdlib.h>

/*
** ft_lstsize - T5. Cases from _dev/plan/rank00/libft-01-cases.md section 37.
**
** size counts nodes, not contents - a node holding NULL still counts, and
** the fixture is left alive afterward (size only reads the list).
*/

static void	size_case(t_ctx *c, t_list *lst, int expected)
{
	int	got;

	bro_ready(c);
	got = ft_lstsize(lst);
	if (got != expected)
		bro_fail(c->out, "expected %d, got %d", expected, got);
	bro_list_free(lst);
}

static void	case_01(t_ctx *c) { size_case(c, NULL, 0); }

static void	case_02(t_ctx *c)
{
	t_list	*lst;

	lst = bro_list_build(1);
	if (!lst)
		return (bro_fail(c->out, "engine: could not build the fixture"));
	size_case(c, lst, 1);
}

static void	case_03(t_ctx *c)
{
	t_list	*lst;

	lst = bro_list_build(3);
	if (!lst)
		return (bro_fail(c->out, "engine: could not build the fixture"));
	size_case(c, lst, 3);
}

static void	case_04(t_ctx *c)
{
	t_list	*lst;

	lst = bro_list_build(100);
	if (!lst)
		return (bro_fail(c->out, "engine: could not build the fixture"));
	size_case(c, lst, 100);
}

/* content is irrelevant to size - three nodes with NULL content still count. */
static void	case_05(t_ctx *c)
{
	t_list	*lst;
	t_list	*node;
	int		i;

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
	size_case(c, lst, 3);
}

static const t_case	g_cases[] = {
{1, "ft_lstsize(NULL)", 0, case_01},
{2, "ft_lstsize(n1)", 0, case_02},
{3, "ft_lstsize(n1->n2->n3)", 0, case_03},
{4, "ft_lstsize(<100 nodes>)", 0, case_04},
{5, "ft_lstsize(<3 nodes, all content NULL>)", 0, case_05},
};

const t_suite	g_suite_ft_lstsize = {
	"ft_lstsize", 3, 8, 5, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
