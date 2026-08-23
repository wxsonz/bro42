#include "bro.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
** t_list fixtures (T5).
**
** Nodes and contents are allocated through the WRAPPED allocator on purpose.
** They are memory the student is expected to free - ft_lstclear takes
** ownership of them - so if the function under test fails to free them, that
** has to show up as a leak. Engine scratch that the student never touches
** (guard buffers, oracle reference buffers) goes through __real_malloc and
** stays out of the accounting entirely.
*/

static size_t	g_del_calls;

t_list	*bro_list_build(size_t n)
{
	t_list	*head;
	t_list	*node;
	char	*content;
	size_t	i;

	head = NULL;
	i = n;
	while (i-- > 0)
	{
		content = malloc(32);
		node = malloc(sizeof(t_list));
		if (!content || !node)
		{
			free(content);
			free(node);
			return (head);
		}
		snprintf(content, 32, "node%lu", (unsigned long)i);
		node->content = content;
		node->next = head;
		head = node;
	}
	return (head);
}

/*
** Cycle-safe: a broken ft_lstadd_back can link a node to itself, and a naive
** while (lst) would hang the case instead of reporting it.
*/
long	bro_list_len(const t_list *lst)
{
	long	n;

	n = 0;
	while (lst)
	{
		if (n > BRO_LIST_MAX)
			return (-1);
		lst = lst->next;
		n++;
	}
	return (n);
}

/*
** Release a fixture the function under test did NOT consume.
**
** ft_lstclear takes ownership of its list, so the exemplar never needed this.
** Every other list function leaves the fixture alive, and since fixtures are
** deliberately allocated through the wrapped allocator - so that a student who
** fails to free them is caught - a test that walks away from one reports a
** leak that belongs to the test, not to the student.
**
** Cycle-safe: a broken ft_lstadd_back can link a node to itself, and freeing
** in a naive loop would run forever or double-free.
*/
void	bro_list_free(t_list *lst)
{
	t_list	*next;
	long	guard;

	guard = 0;
	while (lst && guard++ <= BRO_LIST_MAX)
	{
		next = lst->next;
		free(lst->content);
		free(lst);
		lst = next;
	}
}

void	bro_del_counting(void *content)
{
	g_del_calls++;
	free(content);
}

size_t	bro_del_calls(void)
{
	return (g_del_calls);
}

void	bro_del_reset(void)
{
	g_del_calls = 0;
}
