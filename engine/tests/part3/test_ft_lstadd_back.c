#include "bro.h"
#include "libft_proto.h"

/*
** ft_lstadd_back - T5. Cases from _dev/SPEC_MICRO.md section 39.
**
** Empty list aside, the head never moves - the walk to the tail is where the
** bug lives, so cases 3-5 build lists of 2, 3 and 10 nodes rather than
** stopping at one element. safe_tail() is bounded so a broken add_back that
** links a node into a cycle is reported, not hung.
*/

static t_list	*safe_tail(t_list *lst)
{
	long	guard;

	if (!lst)
		return (NULL);
	guard = 0;
	while (lst->next && guard++ <= BRO_LIST_MAX)
		lst = lst->next;
	return (lst);
}

static void	back_case(t_ctx *c, size_t base_n)
{
	t_list	*lst;
	t_list	*old;
	t_list	*n;

	lst = bro_list_build(base_n);
	if (base_n && !lst)
		return (bro_fail(c->out, "engine: could not build the fixture"));
	n = bro_list_build(1);
	if (!n)
	{
		bro_list_free(lst);
		return (bro_fail(c->out, "engine: could not build the fixture"));
	}
	old = lst;
	bro_ready(c);
	ft_lstadd_back(&lst, n);
	if (lst != (base_n ? old : n))
	{
		bro_fail(c->out, "the head must not move once the list is non-empty");
		return (bro_list_free(lst));
	}
	if (bro_list_len(lst) != (long)base_n + 1)
	{
		bro_fail(c->out, "expected length %zu, got %ld", base_n + 1,
			bro_list_len(lst));
		return (bro_list_free(lst));
	}
	if (safe_tail(lst) != n || n->next != NULL)
		bro_fail(c->out, "the new node must terminate the list");
	bro_list_free(lst);
}

static void	case_01(t_ctx *c) { back_case(c, 0); }
static void	case_02(t_ctx *c) { back_case(c, 1); }
static void	case_03(t_ctx *c) { back_case(c, 2); }
static void	case_04(t_ctx *c) { back_case(c, 3); }
static void	case_05(t_ctx *c) { back_case(c, 10); }
static void	case_06(t_ctx *c) { back_case(c, 2); }

static const t_case	g_cases[] = {
{1, "lst=NULL, add_back(&lst, n1)", 0, case_01},
{2, "lst=n1, add_back(&lst, n2)", 0, case_02},
{3, "lst=n1->n2, add_back(&lst, n3)", 0, case_03},
{4, "lst=n1->n2->n3, add_back(&lst, n4)", 0, case_04},
{5, "lst=<10 nodes>, add_back(&lst, n)", 0, case_05},
{6, "lst=n1->n2, add_back(&lst, n3)", 0, case_06},
};

const t_suite	g_suite_ft_lstadd_back = {
	"ft_lstadd_back", 3, 8, 5, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
