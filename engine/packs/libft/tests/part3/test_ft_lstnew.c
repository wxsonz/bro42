#include "bro.h"
#include "proto.h"
#include "pack.h"
#include <stdlib.h>

/*
** ft_lstnew - T5. Cases from _dev/plan/rank00/libft-01-cases.md section 35.
**
** lstnew stores the content POINTER, never a copy - pointer identity is
** asserted against the exact pointer handed in, because a strcmp would let
** a wrongly strdup'ing implementation pass.
*/

static void	case_01(t_ctx *c)
{
	int		x;
	t_list	*node;

	x = 42;
	node = ft_lstnew(&x);
	bro_track(c, node);
	if (bro_injecting(c))
		return ;
	if (!node)
		return (bro_fail(c->out, "returned NULL"));
	if (node->content != &x)
		bro_fail(c->out, "content is not the identical pointer passed in");
	free(node);
}

static void	case_02(t_ctx *c)
{
	char	x;
	t_list	*node;

	x = 'A';
	node = ft_lstnew(&x);
	bro_track(c, node);
	if (bro_injecting(c))
		return ;
	if (!node)
		return (bro_fail(c->out, "returned NULL"));
	if (node->next != NULL)
		bro_fail(c->out, "a freshly created node must have next == NULL");
	free(node);
}

/* content is void * and never dereferenced here: NULL is legitimate. */
static void	case_03(t_ctx *c)
{
	t_list	*node;

	node = ft_lstnew(NULL);
	bro_track(c, node);
	if (bro_injecting(c))
		return ;
	if (!node)
		return (bro_fail(c->out, "returned NULL for a NULL content"));
	if (node->content != NULL || node->next != NULL)
		bro_fail(c->out, "content and next must both be NULL");
	free(node);
}

static void	case_04(t_ctx *c)
{
	int		x;
	t_list	*node;

	x = 7;
	node = ft_lstnew(&x);
	bro_track(c, node);
	if (bro_injecting(c))
		return ;
	if (!node)
		return (bro_fail(c->out, "returned NULL"));
	free(node);
}

static const t_case	g_cases[] = {
{1, "node = ft_lstnew(p)", BRO_INJECT, case_01},
{2, "node = ft_lstnew(p)", BRO_INJECT, case_02},
{3, "node = ft_lstnew(NULL)", BRO_INJECT, case_03},
{4, "ft_lstnew(p)", BRO_INJECT, case_04},
};

const t_suite	g_suite_ft_lstnew = {
	"ft_lstnew", 3, 8, 5, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
