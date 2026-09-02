#ifndef LIBFT_PROTO_H
# define LIBFT_PROTO_H

/*
** The SUBJECT's prototypes, declared by the engine rather than read from the
** student's libft.h. Two reasons: the header may not exist yet, and if their
** prototype disagrees with the subject the link fails here - which is itself
** a finding, not something to paper over.
**
** Milestone 2 in progress: the six tier exemplars, the T0 batch, the T1
** batch and the T2 batch.
*/

# include <stddef.h>

/* pack.h pulls in bro.h plus t_list - several prototypes below (ft_lstnew
** and friends) return or take it, and t_list is Libft's own concept, not
** the core's (see pack.h). */
# include "pack.h"

int		ft_isalpha(int c);
int		ft_isdigit(int c);
int		ft_isalnum(int c);
int		ft_isascii(int c);
int		ft_isprint(int c);
int		ft_toupper(int c);
int		ft_tolower(int c);
size_t		ft_strlen(const char *s);
char		*ft_strchr(const char *s, int c);
char		*ft_strrchr(const char *s, int c);
int		ft_strncmp(const char *s1, const char *s2, size_t n);
void		*ft_memchr(const void *s, int c, size_t n);
char		*ft_strnstr(const char *big, const char *little, size_t len);
int		ft_atoi(const char *nptr);
int		ft_memcmp(const void *s1, const void *s2, size_t n);
void		*ft_memmove(void *dest, const void *src, size_t n);
char		*ft_itoa(int n);
char		**ft_split(char const *s, char c);
void		ft_putnbr_fd(int n, int fd);
void		ft_lstclear(t_list **lst, void (*del)(void *));
void		*ft_memset(void *s, int c, size_t n);
void		ft_bzero(void *s, size_t n);
void		*ft_memcpy(void *dest, const void *src, size_t n);
size_t		ft_strlcpy(char *dst, const char *src, size_t size);
size_t		ft_strlcat(char *dst, const char *src, size_t size);
void		ft_striteri(char *s, void (*f)(unsigned int, char *));
void		*ft_calloc(size_t nmemb, size_t size);
char		*ft_strdup(const char *s);
char		*ft_substr(char const *s, unsigned int start, size_t len);
char		*ft_strjoin(char const *s1, char const *s2);
char		*ft_strtrim(char const *s1, char const *set);
char		*ft_strmapi(char const *s, char (*f)(unsigned int, char));
void		ft_putchar_fd(char c, int fd);
void		ft_putstr_fd(char *s, int fd);
void		ft_putendl_fd(char *s, int fd);
t_list		*ft_lstnew(void *content);
void		ft_lstadd_front(t_list **lst, t_list *new);
unsigned int	ft_lstsize(t_list *lst);
t_list		*ft_lstlast(t_list *lst);
void		ft_lstadd_back(t_list **lst, t_list *new);
void		ft_lstdelone(t_list *lst, void (*del)(void *));
void		ft_lstiter(t_list *lst, void (*f)(void *));
t_list		*ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *));

#endif
