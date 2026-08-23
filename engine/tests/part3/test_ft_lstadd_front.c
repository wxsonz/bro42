#include "bro.h"
#include "libft_proto.h"

/*
** ft_lstadd_front - T5. Cases from _dev/plan/rank00/libft-01-cases.md section 36.
**
** The fixture is left alive: add_front only relinks pointers and consumes
** nothing, so the whole combined list is freed at the end (never consumed
** by the function under test).
*/

static void	front_case(t_ctx *c, size_t base_n)
{
	t_list	*lst;
	t_list	*old;
	t_list	*n0;

	lst = bro_list_build(base_n);
	if (base_n && !lst)
		return (bro_fail(c->out, "engine: could not build the fixture"));
	n0 = bro_list_build(1);
	if (!n0)
	{
		bro_list_free(lst);
		return (bro_fail(c->out, "engine: could not build the fixture"));
	}
	old = lst;
	bro_ready(c);
	ft_lstadd_front(&lst, n0);
	if (lst != n0 || n0->next != old)
	{
		bro_fail(c->out, "the new node must become head, linked to the old head");
		return (bro_list_free(lst));
	}
	if (bro_list_len(lst) != (long)base_n + 1)
		bro_fail(c->out, "expected a list of length %zu, got %ld",
			base_n + 1, bro_list_len(lst));
	bro_list_free(lst);
}

static void	case_01(t_ctx *c) { front_case(c, 0); }
static void	case_02(t_ctx *c) { front_case(c, 1); }
static void	case_03(t_ctx *c) { front_case(c, 3); }
static void	case_04(t_ctx *c) { front_case(c, 3); }
static void	case_05(t_ctx *c) { front_case(c, 0); }

static const t_case	g_cases[] = {
{1, "lst=NULL, add_front(&lst, n1)", 0, case_01},
{2, "lst=n1, add_front(&lst, n2)", 0, case_02},
{3, "lst=n1->n2->n3, add_front(&lst, n0)", 0, case_03},
{4, "lst=n1->n2->n3, add_front(&lst, n0)", 0, case_04},
{5, "lst=NULL, add_front(&lst, n1)", 0, case_05},
};

const t_suite	g_suite_ft_lstadd_front = {
	"ft_lstadd_front", 3, 8, 5, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
