#include "bro.h"

/*
** The suites, statically listed. No dynamic registration, no constructor
** magic: adding a function is adding a line (design/02_ENGINE.md non-goals).
**
** Stage B: six suites, one per engine tier T0-T5. Milestone 2 adds the other
** 37 by tier batch, each following its tier's exemplar here.
*/

extern const t_suite	g_suite_ft_isalpha;
extern const t_suite	g_suite_ft_isdigit;
extern const t_suite	g_suite_ft_isalnum;
extern const t_suite	g_suite_ft_isascii;
extern const t_suite	g_suite_ft_isprint;
extern const t_suite	g_suite_ft_toupper;
extern const t_suite	g_suite_ft_tolower;
extern const t_suite	g_suite_ft_strlen;
extern const t_suite	g_suite_ft_strchr;
extern const t_suite	g_suite_ft_strrchr;
extern const t_suite	g_suite_ft_strncmp;
extern const t_suite	g_suite_ft_memchr;
extern const t_suite	g_suite_ft_strnstr;
extern const t_suite	g_suite_ft_atoi;
extern const t_suite	g_suite_ft_memcmp;
extern const t_suite	g_suite_ft_memmove;
extern const t_suite	g_suite_ft_itoa;
extern const t_suite	g_suite_ft_split;
extern const t_suite	g_suite_ft_putnbr_fd;
extern const t_suite	g_suite_ft_lstclear;
extern const t_suite	g_suite_ft_memset;
extern const t_suite	g_suite_ft_bzero;
extern const t_suite	g_suite_ft_memcpy;
extern const t_suite	g_suite_ft_strlcpy;
extern const t_suite	g_suite_ft_strlcat;
extern const t_suite	g_suite_ft_striteri;
extern const t_suite	g_suite_ft_calloc;
extern const t_suite	g_suite_ft_strdup;
extern const t_suite	g_suite_ft_substr;
extern const t_suite	g_suite_ft_strjoin;
extern const t_suite	g_suite_ft_strtrim;
extern const t_suite	g_suite_ft_strmapi;
extern const t_suite	g_suite_ft_putchar_fd;
extern const t_suite	g_suite_ft_putstr_fd;
extern const t_suite	g_suite_ft_putendl_fd;
extern const t_suite	g_suite_ft_lstnew;
extern const t_suite	g_suite_ft_lstadd_front;
extern const t_suite	g_suite_ft_lstsize;
extern const t_suite	g_suite_ft_lstlast;
extern const t_suite	g_suite_ft_lstadd_back;
extern const t_suite	g_suite_ft_lstdelone;
extern const t_suite	g_suite_ft_lstiter;
extern const t_suite	g_suite_ft_lstmap;

const t_suite	*g_suites[] = {
	&g_suite_ft_isalpha,		/* T0 */
	&g_suite_ft_isdigit,		/* T0 */
	&g_suite_ft_isalnum,		/* T0 */
	&g_suite_ft_isascii,		/* T0 */
	&g_suite_ft_isprint,		/* T0 */
	&g_suite_ft_toupper,		/* T0 */
	&g_suite_ft_tolower,		/* T0 */
	&g_suite_ft_strlen,		/* T0 */
	&g_suite_ft_strchr,		/* T0 */
	&g_suite_ft_strrchr,		/* T0 */
	&g_suite_ft_strncmp,		/* T0 */
	&g_suite_ft_memchr,		/* T0 */
	&g_suite_ft_strnstr,		/* T0 */
	&g_suite_ft_atoi,		/* T0 */
	&g_suite_ft_memcmp,		/* T0 */
	&g_suite_ft_memmove,		/* T1 */
	&g_suite_ft_memset,		/* T1 */
	&g_suite_ft_bzero,		/* T1 */
	&g_suite_ft_memcpy,		/* T1 */
	&g_suite_ft_strlcpy,		/* T1 */
	&g_suite_ft_strlcat,		/* T1 */
	&g_suite_ft_striteri,		/* T1 */
	&g_suite_ft_itoa,		/* T2 */
	&g_suite_ft_calloc,		/* T2 */
	&g_suite_ft_strdup,		/* T2 */
	&g_suite_ft_substr,		/* T2 */
	&g_suite_ft_strjoin,		/* T2 */
	&g_suite_ft_strtrim,		/* T2 */
	&g_suite_ft_strmapi,		/* T2 */
	&g_suite_ft_split,		/* T3 */
	&g_suite_ft_putchar_fd,		/* T4 */
	&g_suite_ft_putstr_fd,		/* T4 */
	&g_suite_ft_putendl_fd,		/* T4 */
	&g_suite_ft_putnbr_fd,		/* T4 */
	&g_suite_ft_lstnew,		/* T5 */
	&g_suite_ft_lstadd_front,	/* T5 */
	&g_suite_ft_lstsize,		/* T5 */
	&g_suite_ft_lstlast,		/* T5 */
	&g_suite_ft_lstadd_back,	/* T5 */
	&g_suite_ft_lstdelone,		/* T5 */
	&g_suite_ft_lstclear,		/* T5 */
	&g_suite_ft_lstiter,		/* T5 */
	&g_suite_ft_lstmap,		/* T5 */
};

const size_t	g_suite_count = sizeof(g_suites) / sizeof(*g_suites);
