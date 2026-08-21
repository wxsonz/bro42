#ifndef TESTER_H
# define TESTER_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <stdint.h>
# include <stdbool.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <signal.h>
# include <fcntl.h>
# include <errno.h>

# include "color.h"
# include "libc_oracle.h"
# include "test_utils.h"

/* Include student/target libft header */
# if __has_include("libft.h")
#  include "libft.h"
# else
typedef struct s_list
{
	void			*content;
	struct s_list	*next;
}	t_list;
# endif

typedef enum e_test_status
{
	TEST_OK = 0,
	TEST_KO,
	TEST_NOT_FOUND,
	TEST_SIGSEGV,
	TEST_SIGBUS,
	TEST_SIGABRT,
	TEST_SIGFPE,
	TEST_TIMEOUT,
	TEST_LEAK
}	t_test_status;

typedef struct s_test_case
{
	int				test_num;
	char			*desc;
	char			*input;
	char			*expected;
	char			*actual;
	char			*hint;
	char			*ref;
	t_test_status	status;
	int				signal_num;
}	t_test_case;

typedef struct s_test_ipc
{
	int				test_num;
	t_test_status	status;
	int				signal_num;
	char			desc[128];
	char			input[128];
	char			expected[128];
	char			actual[128];
	char			hint[256];
	char			ref[128];
}	t_test_ipc;

# define MAX_TESTS_PER_FUNC 30
# define MAX_FUNCS 50

typedef struct s_func_suite
{
	const char	*func_name;
	int			part; /* 1, 2, or 3 */
	int			test_count;
	int			pass_count;
	int			fail_count;
	bool		not_found;
	t_test_case	cases[MAX_TESTS_PER_FUNC];
}	t_func_suite;

typedef struct s_suite_summary
{
	int			total_funcs;
	int			passed_funcs;
	int			not_found_funcs;
	int			total_tests;
	int			passed_tests;
	int			failed_tests;
	int			crashed_tests;
	int			timed_out_tests;
	int			leaked_tests;
}	t_suite_summary;

/* Global test context during single test execution */
extern t_test_ipc	g_current_ipc;
extern const char	*g_current_func_under_test;
extern bool			g_func_not_found;
extern bool			g_verbose;
extern bool			g_json_mode;

/* Weak stub call recorder */
void	record_weak_stub_call(const char *func_name);

/* Test helper functions for linked list suites */
t_list	*test_create_node(void *content);
void	test_free_list(t_list *lst);
void	test_free_list_with_del(t_list *lst, void (*del)(void *));

/* Assertion engine functions */
void	set_test_info(int num, const char *desc, const char *input, const char *hint, const char *ref);
void	record_test_ok(const char *actual_val);
void	record_test_ko(const char *expected_val, const char *actual_val);
void	record_test_crash(int sig);
void	record_test_timeout(void);
void	record_test_leak(size_t leaked_bytes);

/* Assertion Macros */
# define TEST_START(num, desc, input, hint, ref) \
	set_test_info(num, desc, input, hint, ref)

# define ASSERT_TRUE(cond, exp_str, act_str) do { \
	if (cond) { \
		record_test_ok(act_str); \
	} else { \
		record_test_ko(exp_str, act_str); \
		return ; \
	} \
} while (0)

# define ASSERT_EQ_INT(act, exp) do { \
	char _exp_buf[64], _act_buf[64]; \
	snprintf(_exp_buf, sizeof(_exp_buf), "%d", (int)(exp)); \
	snprintf(_act_buf, sizeof(_act_buf), "%d", (int)(act)); \
	if ((int)(act) == (int)(exp)) { \
		record_test_ok(_act_buf); \
	} else { \
		record_test_ko(_exp_buf, _act_buf); \
		return ; \
	} \
} while (0)

# define ASSERT_EQ_SIZE(act, exp) do { \
	char _exp_buf[64], _act_buf[64]; \
	snprintf(_exp_buf, sizeof(_exp_buf), "%zu", (size_t)(exp)); \
	snprintf(_act_buf, sizeof(_act_buf), "%zu", (size_t)(act)); \
	if ((size_t)(act) == (size_t)(exp)) { \
		record_test_ok(_act_buf); \
	} else { \
		record_test_ko(_exp_buf, _act_buf); \
		return ; \
	} \
} while (0)

# define ASSERT_EQ_STR(act, exp) do { \
	const char *_a = (const char *)(act); \
	const char *_e = (const char *)(exp); \
	if (_a == NULL && _e == NULL) { \
		record_test_ok("NULL"); \
	} else if (_a == NULL) { \
		record_test_ko(_e, "NULL"); \
		return ; \
	} else if (_e == NULL) { \
		record_test_ko("NULL", _a); \
		return ; \
	} else if (strcmp(_a, _e) == 0) { \
		record_test_ok(_a); \
	} else { \
		record_test_ko(_e, _a); \
		return ; \
	} \
} while (0)

# define ASSERT_EQ_MEM(act, exp, n) do { \
	if (memcmp(act, exp, n) == 0) { \
		record_test_ok("[Memory buffer identical]"); \
	} else { \
		record_test_ko("[Expected memory buffer]", "[Actual memory buffer mismatch]"); \
		return ; \
	} \
} while (0)

# define ASSERT_NULL(act) do { \
	if ((act) == NULL) { \
		record_test_ok("NULL"); \
	} else { \
		record_test_ko("NULL", "[Non-NULL pointer]"); \
		return ; \
	} \
} while (0)

# define ASSERT_NOT_NULL(act) do { \
	if ((act) != NULL) { \
		record_test_ok("[Valid pointer]"); \
	} else { \
		record_test_ko("[Non-NULL pointer]", "NULL"); \
		return ; \
	} \
} while (0)

# define ASSERT_PTR_EQ(act, exp) do { \
	char _exp_buf[64], _act_buf[64]; \
	snprintf(_exp_buf, sizeof(_exp_buf), "%p", (void*)(exp)); \
	snprintf(_act_buf, sizeof(_act_buf), "%p", (void*)(act)); \
	if ((void*)(act) == (void*)(exp)) { \
		record_test_ok(_act_buf); \
	} else { \
		record_test_ko(_exp_buf, _act_buf); \
		return ; \
	} \
} while (0)

/* Test Runner functions */
void	run_isolated_test(void (*test_fn)(void), t_test_case *tc);
void	run_suite_tests(t_func_suite *s, void (**tests)(void));
void	print_suite_header(const char *func_name, int part);
void	print_case_result(const t_test_case *tc);
void	print_not_found(const char *func_name);
void	print_suite_summary(const t_func_suite *suite);
void	print_total_summary(const t_suite_summary *summary);

/* Macro Suite */
int		run_macro_suite(const char *target_dir, bool verbose);

/* JSON Report Generator */
void	generate_json_report(const char *filepath, t_func_suite *suites, int suite_count, int macro_score);

/* Browser Launcher */
void	launch_browser(const char *url_or_file);

/* Micro Suite Function Declarations - Part 1 */
void	suite_isalpha(t_func_suite *s);
void	suite_isdigit(t_func_suite *s);
void	suite_isalnum(t_func_suite *s);
void	suite_isascii(t_func_suite *s);
void	suite_isprint(t_func_suite *s);
void	suite_strlen(t_func_suite *s);
void	suite_memset(t_func_suite *s);
void	suite_bzero(t_func_suite *s);
void	suite_memcpy(t_func_suite *s);
void	suite_memmove(t_func_suite *s);
void	suite_strlcpy(t_func_suite *s);
void	suite_strlcat(t_func_suite *s);
void	suite_toupper(t_func_suite *s);
void	suite_tolower(t_func_suite *s);
void	suite_strchr(t_func_suite *s);
void	suite_strrchr(t_func_suite *s);
void	suite_strncmp(t_func_suite *s);
void	suite_memchr(t_func_suite *s);
void	suite_memcmp(t_func_suite *s);
void	suite_strnstr(t_func_suite *s);
void	suite_atoi(t_func_suite *s);
void	suite_calloc(t_func_suite *s);
void	suite_strdup(t_func_suite *s);

/* Micro Suite Function Declarations - Part 2 */
void	suite_substr(t_func_suite *s);
void	suite_strjoin(t_func_suite *s);
void	suite_strtrim(t_func_suite *s);
void	suite_split(t_func_suite *s);
void	suite_itoa(t_func_suite *s);
void	suite_strmapi(t_func_suite *s);
void	suite_striteri(t_func_suite *s);
void	suite_putchar_fd(t_func_suite *s);
void	suite_putstr_fd(t_func_suite *s);
void	suite_putendl_fd(t_func_suite *s);
void	suite_putnbr_fd(t_func_suite *s);

/* Micro Suite Function Declarations - Part 3 */
void	suite_lstnew(t_func_suite *s);
void	suite_lstadd_front(t_func_suite *s);
void	suite_lstsize(t_func_suite *s);
void	suite_lstlast(t_func_suite *s);
void	suite_lstadd_back(t_func_suite *s);
void	suite_lstdelone(t_func_suite *s);
void	suite_lstclear(t_func_suite *s);
void	suite_lstiter(t_func_suite *s);
void	suite_lstmap(t_func_suite *s);

#endif
