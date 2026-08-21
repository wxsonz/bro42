#include "tester.h"

const char	*g_current_func_under_test = NULL;
bool		g_func_not_found = false;

void	record_weak_stub_call(const char *func_name)
{
	if (g_current_func_under_test && func_name && strcmp(g_current_func_under_test, func_name) == 0)
		g_func_not_found = true;
}

/*
** Weak fallback stubs for all 43 Libft functions.
** When a student links their libft.a:
** - If their libft.a defines a function (strong symbol), the student's version is used.
** - If their libft.a does NOT define a function (unimplemented), the weak stub is used.
*/

/* Part 1: Libc functions */
__attribute__((weak)) int		ft_isalpha(int c) { (void)c; record_weak_stub_call("ft_isalpha"); return (-999); }
__attribute__((weak)) int		ft_isdigit(int c) { (void)c; record_weak_stub_call("ft_isdigit"); return (-999); }
__attribute__((weak)) int		ft_isalnum(int c) { (void)c; record_weak_stub_call("ft_isalnum"); return (-999); }
__attribute__((weak)) int		ft_isascii(int c) { (void)c; record_weak_stub_call("ft_isascii"); return (-999); }
__attribute__((weak)) int		ft_isprint(int c) { (void)c; record_weak_stub_call("ft_isprint"); return (-999); }
__attribute__((weak)) size_t	ft_strlen(const char *s) { (void)s; record_weak_stub_call("ft_strlen"); return (0); }
__attribute__((weak)) void		*ft_memset(void *b, int c, size_t len) { (void)b; (void)c; (void)len; record_weak_stub_call("ft_memset"); return (NULL); }
__attribute__((weak)) void		ft_bzero(void *s, size_t n) { (void)s; (void)n; record_weak_stub_call("ft_bzero"); }
__attribute__((weak)) void		*ft_memcpy(void *dst, const void *src, size_t n) { (void)dst; (void)src; (void)n; record_weak_stub_call("ft_memcpy"); return (NULL); }
__attribute__((weak)) void		*ft_memmove(void *dst, const void *src, size_t len) { (void)dst; (void)src; (void)len; record_weak_stub_call("ft_memmove"); return (NULL); }
__attribute__((weak)) size_t	ft_strlcpy(char *dst, const char *src, size_t dstsize) { (void)dst; (void)src; (void)dstsize; record_weak_stub_call("ft_strlcpy"); return (0); }
__attribute__((weak)) size_t	ft_strlcat(char *dst, const char *src, size_t dstsize) { (void)dst; (void)src; (void)dstsize; record_weak_stub_call("ft_strlcat"); return (0); }
__attribute__((weak)) int		ft_toupper(int c) { (void)c; record_weak_stub_call("ft_toupper"); return (-999); }
__attribute__((weak)) int		ft_tolower(int c) { (void)c; record_weak_stub_call("ft_tolower"); return (-999); }
__attribute__((weak)) char		*ft_strchr(const char *s, int c) { (void)s; (void)c; record_weak_stub_call("ft_strchr"); return (NULL); }
__attribute__((weak)) char		*ft_strrchr(const char *s, int c) { (void)s; (void)c; record_weak_stub_call("ft_strrchr"); return (NULL); }
__attribute__((weak)) int		ft_strncmp(const char *s1, const char *s2, size_t n) { (void)s1; (void)s2; (void)n; record_weak_stub_call("ft_strncmp"); return (-999); }
__attribute__((weak)) void		*ft_memchr(const void *s, int c, size_t n) { (void)s; (void)c; (void)n; record_weak_stub_call("ft_memchr"); return (NULL); }
__attribute__((weak)) int		ft_memcmp(const void *s1, const void *s2, size_t n) { (void)s1; (void)s2; (void)n; record_weak_stub_call("ft_memcmp"); return (-999); }
__attribute__((weak)) char		*ft_strnstr(const char *haystack, const char *needle, size_t len) { (void)haystack; (void)needle; (void)len; record_weak_stub_call("ft_strnstr"); return (NULL); }
__attribute__((weak)) int		ft_atoi(const char *str) { (void)str; record_weak_stub_call("ft_atoi"); return (-999); }
__attribute__((weak)) void		*ft_calloc(size_t count, size_t size) { (void)count; (void)size; record_weak_stub_call("ft_calloc"); return (NULL); }
__attribute__((weak)) char		*ft_strdup(const char *s1) { (void)s1; record_weak_stub_call("ft_strdup"); return (NULL); }

/* Part 2: Additional functions */
__attribute__((weak)) char		*ft_substr(char const *s, unsigned int start, size_t len) { (void)s; (void)start; (void)len; record_weak_stub_call("ft_substr"); return (NULL); }
__attribute__((weak)) char		*ft_strjoin(char const *s1, char const *s2) { (void)s1; (void)s2; record_weak_stub_call("ft_strjoin"); return (NULL); }
__attribute__((weak)) char		*ft_strtrim(char const *s1, char const *set) { (void)s1; (void)set; record_weak_stub_call("ft_strtrim"); return (NULL); }
__attribute__((weak)) char		**ft_split(char const *s, char c) { (void)s; (void)c; record_weak_stub_call("ft_split"); return (NULL); }
__attribute__((weak)) char		*ft_itoa(int n) { (void)n; record_weak_stub_call("ft_itoa"); return (NULL); }
__attribute__((weak)) char		*ft_strmapi(char const *s, char (*f)(unsigned int, char)) { (void)s; (void)f; record_weak_stub_call("ft_strmapi"); return (NULL); }
__attribute__((weak)) void		ft_striteri(char *s, void (*f)(unsigned int, char*)) { (void)s; (void)f; record_weak_stub_call("ft_striteri"); }
__attribute__((weak)) void		ft_putchar_fd(char c, int fd) { (void)c; (void)fd; record_weak_stub_call("ft_putchar_fd"); }
__attribute__((weak)) void		ft_putstr_fd(char *s, int fd) { (void)s; (void)fd; record_weak_stub_call("ft_putstr_fd"); }
__attribute__((weak)) void		ft_putendl_fd(char *s, int fd) { (void)s; (void)fd; record_weak_stub_call("ft_putendl_fd"); }
__attribute__((weak)) void		ft_putnbr_fd(int n, int fd) { (void)n; (void)fd; record_weak_stub_call("ft_putnbr_fd"); }

/* Part 3: Linked list functions */
__attribute__((weak)) t_list	*ft_lstnew(void *content) { (void)content; record_weak_stub_call("ft_lstnew"); return (NULL); }
__attribute__((weak)) void		ft_lstadd_front(t_list **lst, t_list *new) { (void)lst; (void)new; record_weak_stub_call("ft_lstadd_front"); }
__attribute__((weak)) int		ft_lstsize(t_list *lst) { (void)lst; record_weak_stub_call("ft_lstsize"); return (0); }
__attribute__((weak)) t_list	*ft_lstlast(t_list *lst) { (void)lst; record_weak_stub_call("ft_lstlast"); return (NULL); }
__attribute__((weak)) void		ft_lstadd_back(t_list **lst, t_list *new) { (void)lst; (void)new; record_weak_stub_call("ft_lstadd_back"); }
__attribute__((weak)) void		ft_lstdelone(t_list *lst, void (*del)(void *)) { (void)lst; (void)del; record_weak_stub_call("ft_lstdelone"); }
__attribute__((weak)) void		ft_lstclear(t_list **lst, void (*del)(void *)) { (void)lst; (void)del; record_weak_stub_call("ft_lstclear"); }
__attribute__((weak)) void		ft_lstiter(t_list *lst, void (*f)(void *)) { (void)lst; (void)f; record_weak_stub_call("ft_lstiter"); }
__attribute__((weak)) t_list	*ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *)) { (void)lst; (void)f; (void)del; record_weak_stub_call("ft_lstmap"); return (NULL); }
