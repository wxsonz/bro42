#include "bro.h"
#include "libft_proto.h"

/*
** ft_lstiter - T5. Cases from _dev/plan/rank00/libft-01-cases.md section 42.
**
** iter owns nothing: it must never free, and the list must stay fully
** intact and traversable once it returns. Case 1 checks both that every
** content was transformed and that the visits happened front to back.
*/

# define ITER_SEEN_MAX	128

static void	*g_seen[ITER_SEEN_MAX];
static size_t	g_seen_n;
static size_t	g_calls;

static void	reset_iter(void)
{
	g_seen_n = 0;
	g_calls = 0;
}

static void	count_f(void *content)
{
	(void)content;
	g_calls++;
}

static void	upper_f(void *content)
{
	char	*s;

	if (g_seen_n < ITER_SEEN_MAX)
		g_seen[g_seen_n++] = content;
	s = content;
	while (*s)
	{
		if (*s >= 'a' && *s <= 'z')
			*s = *s - 'a' + 'A';
		s++;
	}
}

static bool	no_lower(const char *s)
{
	while (*s)
	{
		if (*s >= 'a' && *s <= 'z')
			return (false);
		s++;
	}
	return (true);
}

static void	case_01(t_ctx *c)
{
	t_list	*lst;
	t_list	*walk;
	size_t	i;

	lst = bro_list_build(3);
	if (!lst)
		return (bro_fail(c->out, "engine: could not build the fixture"));
	reset_iter();
	bro_ready(c);
	ft_lstiter(lst, upper_f);
	walk = lst;
	i = 0;
	while (walk)
	{
		if (g_seen_n <= i || g_seen[i] != walk->content)
			bro_fail(c->out, "node %zu was not visited in list order", i);
		else if (!no_lower(walk->content))
			bro_fail(c->out, "node %zu was not transformed", i);
		walk = walk->next;
		i++;
	}
	bro_list_free(lst);
}

static void	case_02(t_ctx *c)
{
	t_list	*lst;

	lst = bro_list_build(1);
	if (!lst)
		return (bro_fail(c->out, "engine: could not build the fixture"));
	reset_iter();
	bro_ready(c);
	ft_lstiter(lst, count_f);
	if (g_calls != 1)
		bro_fail(c->out, "f was called %zu time(s), expected 1", g_calls);
	bro_list_free(lst);
}

static void	case_03(t_ctx *c)
{
	t_list	*lst;

	lst = bro_list_build(100);
	if (!lst)
		return (bro_fail(c->out, "engine: could not build the fixture"));
	reset_iter();
	bro_ready(c);
	ft_lstiter(lst, count_f);
	if (g_calls != 100)
		bro_fail(c->out, "f was called %zu time(s), expected 100", g_calls);
	bro_list_free(lst);
}

static void	case_04(t_ctx *c)
{
	t_list	*lst;
	long	len;

	lst = bro_list_build(3);
	if (!lst)
		return (bro_fail(c->out, "engine: could not build the fixture"));
	reset_iter();
	bro_ready(c);
	ft_lstiter(lst, count_f);
	len = bro_list_len(lst);
	if (len != 3)
		bro_fail(c->out, "expected the list to stay 3 long, got %ld", len);
	bro_list_free(lst);
}

static void	case_05(t_ctx *c)
{
	reset_iter();
	ft_lstiter(NULL, count_f);
	if (g_calls != 0)
		bro_fail(c->out, "f must never be called on an empty list");
}

static const t_case	g_cases[] = {
{1, "lst=n1->n2->n3, iter(lst, <uppercase>)", 0, case_01},
{2, "lst=n1, iter(lst, f)", 0, case_02},
{3, "lst=<100 nodes>, iter(lst, <counter>)", 0, case_03},
{4, "lst=n1->n2->n3, iter(lst, f)", 0, case_04},
{5, "lst=NULL, iter(lst, f)", 0, case_05},
};

const t_suite	g_suite_ft_lstiter = {
	"ft_lstiter", 3, 8, 5, g_cases, sizeof(g_cases) / sizeof(*g_cases)
};
