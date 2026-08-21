#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_tolower('A')", "'A'", "Uppercase to lowercase", "man 3 tolower");
	ASSERT_EQ_INT(ft_tolower('A'), tolower('A'));
}
static void t2(void) {
	TEST_START(2, "ft_tolower('a')", "'a'", "Already lowercase", "man 3 tolower");
	ASSERT_EQ_INT(ft_tolower('a'), tolower('a'));
}
static void t3(void) {
	TEST_START(3, "ft_tolower('0')", "'0'", "Digits unchanged", "man 3 tolower");
	ASSERT_EQ_INT(ft_tolower('0'), tolower('0'));
}

void	suite_tolower(t_func_suite *s)
{
	s->func_name = "ft_tolower";
	s->part = 1;
	s->test_count = 3;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3};
	run_suite_tests(s, tests);
}
