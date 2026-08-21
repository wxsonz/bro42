#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_strdup(\"Hello 42\")", "\"Hello 42\"", "Duplicates string into heap", "man 3 strdup");
	const char *orig = "Hello 42";
	char *dup = ft_strdup(orig);
	ASSERT_NOT_NULL(dup);
	ASSERT_TRUE(strcmp(dup, orig) == 0 && dup != orig, "dup == orig && dup != orig", "Matched & distinct pointer");
	free(dup);
}
static void t2(void) {
	TEST_START(2, "ft_strdup(\"\")", "\"\"", "Duplicates empty string", "man 3 strdup");
	const char *orig = "";
	char *dup = ft_strdup(orig);
	ASSERT_NOT_NULL(dup);
	ASSERT_TRUE(strcmp(dup, orig) == 0 && dup != orig, "dup == \"\" && dup != orig", "Matched & distinct pointer");
	free(dup);
}

void	suite_strdup(t_func_suite *s)
{
	s->func_name = "ft_strdup";
	s->part = 1;
	s->test_count = 2;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2};
	run_suite_tests(s, tests);
}
