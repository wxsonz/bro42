#include "tester.h"

static void free_tab(char **tab)
{
	if (!tab) return;
	for (int i = 0; tab[i]; i++)
		free(tab[i]);
	free(tab);
}

static void t1(void) {
	TEST_START(1, "ft_split standard words", "\"hello world 42\", ' '", "Splits into 3 words", "Subject Section IV.3");
	char **tab = ft_split("hello world 42", ' ');
	ASSERT_NOT_NULL(tab);
	ASSERT_EQ_STR(tab[0], "hello");
	ASSERT_EQ_STR(tab[1], "world");
	ASSERT_EQ_STR(tab[2], "42");
	ASSERT_NULL(tab[3]);
	free_tab(tab);
}
static void t2(void) {
	TEST_START(2, "ft_split leading & trailing delimiters", "\"   hello   world   \", ' '", "Handles multiple consecutive spaces", "Subject Section IV.3");
	char **tab = ft_split("   hello   world   ", ' ');
	ASSERT_NOT_NULL(tab);
	ASSERT_EQ_STR(tab[0], "hello");
	ASSERT_EQ_STR(tab[1], "world");
	ASSERT_NULL(tab[2]);
	free_tab(tab);
}
static void t3(void) {
	TEST_START(3, "ft_split only delimiters", "\"    \", ' '", "Returns array with tab[0] == NULL", "Subject Section IV.3");
	char **tab = ft_split("    ", ' ');
	ASSERT_NOT_NULL(tab);
	ASSERT_NULL(tab[0]);
	free_tab(tab);
}
static void t4(void) {
	TEST_START(4, "ft_split empty string", "\"\", ' '", "Returns array with tab[0] == NULL", "Subject Section IV.3");
	char **tab = ft_split("", ' ');
	ASSERT_NOT_NULL(tab);
	ASSERT_NULL(tab[0]);
	free_tab(tab);
}
static void t5(void) {
	TEST_START(5, "ft_split delimiter not in string", "\"hello\", ' '", "Returns array with [\"hello\", NULL]", "Subject Section IV.3");
	char **tab = ft_split("hello", ' ');
	ASSERT_NOT_NULL(tab);
	ASSERT_EQ_STR(tab[0], "hello");
	ASSERT_NULL(tab[1]);
	free_tab(tab);
}

void	suite_split(t_func_suite *s)
{
	s->func_name = "ft_split";
	s->part = 2;
	s->test_count = 5;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3, t4, t5};
	run_suite_tests(s, tests);
}
