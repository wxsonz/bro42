#ifndef BRO_PACK_LIBFT_H
# define BRO_PACK_LIBFT_H

# include "bro.h"

/*
** Libft pack interfaces. A linked list is this PROJECT's subject matter
** (Part 3, subject v19.2), not the harness's - bro.h is the core's public
** header and has no business declaring t_list, so the Part 3 fixtures and
** their ownership-tracking hooks live here instead, next to the pack that
** actually needs them.
*/

typedef struct s_list
{
	void		*content;
	struct s_list	*next;
}	t_list;

# define BRO_LIST_MAX	4096

t_list	*bro_list_build(size_t n);
void	bro_list_free(t_list *lst);
long	bro_list_len(const t_list *lst);
void	bro_del_counting(void *content);
size_t	bro_del_calls(void);
void	bro_del_reset(void);

#endif
