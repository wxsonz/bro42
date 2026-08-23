#include "bro.h"
#include "libft_proto.h"
#include <stdlib.h>
#include <string.h>

/*
** ft_lstdelone - T5. Cases from _dev/SPEC_MICRO.md section 40.
**
** delone frees exactly one node: content through del, the node with free,
** and it must never follow next - the rest of the list may still be alive.
** bro_del_counting proves del ran (a free(content) masquerading as correct
** would still pass a crash test but never bump the counter).
*/

static void	delone_case(t_ctx *c, bool check_calls)
{
	t_list	*node;
	size_t	before;

	node = bro_list_build(1);
	if (!node)
		return (bro_fail(c->out, "engine: could not build the fixture"));
	before = bro_del_calls();
	bro_ready(c);
	ft_lstdelone(node, bro_del_counting);
	if (check_calls && bro_del_calls() - before != 1)
		bro_fail(c->out, "del was called %zu time(s), expected 1",
			bro_del_calls() - before);
}

static void	case_01(t_ctx *c) { delone_case(c, false); }
static void	case_02(t_ctx *c) { delone_case(c, true); }

/* delone must not follow next: n3 stays fully intact after n2 is deleted. */
static void	case_03(t_ctx *c)
{
	t_list	*n1;
	t_list	*n2;
	t_list	*n3;
	char	saved[32];

	n1 = bro_list_build(3);
	if (!n1)
		return (bro_fail(c->out, "engine: could not build the fixture"));
	n2 = n1->next;
	n3 = n2->next;
	strcpy(saved, n3->content);
	bro_ready(c);
	ft_lstdelone(n2, bro_del_counting);
	if (strcmp(saved, n3->content))
		bro_fail(c->out, "n3 is no longer intact after n2 was deleted");
	free(n3->content);
	free(n3);
	free(n1->content);
	free(n1);
}

static void	case_04(t_ctx *c) { delone_case(c, true); }

/* content may legitimately be NULL; del is still called for it. */
static void	case_05(t_ctx *c)
{
	t_list	*node;
	size_t	before;

	node = malloc(sizeof(t_list));
	if (!node)
		return (bro_fail(c->out, "engine: out of memory"));
	node->content = NULL;
	node->next = NULL;
	before = bro_del_calls();
	bro_ready(c);
	ft_lstdelone(node, bro_del_counting);
	if (bro_del_calls() - before != 1)
		bro_fail(c->out, "del was called %zu time(s), expected 1",
			bro_del_calls() - before);
}

static const t_case	g_cases[] = {
{1, "delone(n2, del)", 0, case_01},
{2, "delone(n2, del)", 0, case_02},
{3, "n1->n2->n3, delone(n2, del)", 0, case_03},
{4, "delone(node, <counting del>)", 0, case_04},
{5, "delone(<node with NULL content>, del)", 0, case_05},
};

const t_suite	g_suite_ft_lstdelone = {
	"ft_lstdelone", 3, 8, 5, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
