#include "bro.h"
#include "libft_proto.h"

/*
** ft_lstlast - T5. Cases from _dev/SPEC_MICRO.md section 38.
**
** The last node is the one whose next is NULL - the walk must stop ON it.
** fixture_tail() walks the engine's own freshly built (and therefore
** cycle-free) fixture, never the student's output.
*/

static t_list	*fixture_tail(t_list *lst)
{
	if (!lst)
		return (NULL);
	while (lst->next)
		lst = lst->next;
	return (lst);
}

static void	last_case(t_ctx *c, t_list *lst)
{
	t_list	*want;
	t_list	*got;

	want = fixture_tail(lst);
	bro_ready(c);
	got = ft_lstlast(lst);
	if (got != want)
		bro_fail(c->out, "expected the tail node, got a different pointer");
	else if (got && got->next != NULL)
		bro_fail(c->out, "the returned node's next must be NULL");
	bro_list_free(lst);
}

static void	case_01(t_ctx *c) { last_case(c, NULL); }

static void	case_02(t_ctx *c)
{
	t_list	*lst;

	lst = bro_list_build(1);
	if (!lst)
		return (bro_fail(c->out, "engine: could not build the fixture"));
	last_case(c, lst);
}

static void	case_03(t_ctx *c)
{
	t_list	*lst;

	lst = bro_list_build(3);
	if (!lst)
		return (bro_fail(c->out, "engine: could not build the fixture"));
	last_case(c, lst);
}

static void	case_04(t_ctx *c)
{
	t_list	*lst;

	lst = bro_list_build(3);
	if (!lst)
		return (bro_fail(c->out, "engine: could not build the fixture"));
	last_case(c, lst);
}

static void	case_05(t_ctx *c)
{
	t_list	*lst;

	lst = bro_list_build(100);
	if (!lst)
		return (bro_fail(c->out, "engine: could not build the fixture"));
	last_case(c, lst);
}

static const t_case	g_cases[] = {
{1, "ft_lstlast(NULL)", 0, case_01},
{2, "ft_lstlast(n1)", 0, case_02},
{3, "ft_lstlast(n1->n2->n3)", 0, case_03},
{4, "ft_lstlast(n1->n2->n3)", 0, case_04},
{5, "ft_lstlast(<100 nodes>)", 0, case_05},
};

const t_suite	g_suite_ft_lstlast = {
	"ft_lstlast", 3, 8, 5, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
