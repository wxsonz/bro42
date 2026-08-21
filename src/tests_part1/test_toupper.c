#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_toupper('a')", "'a'", "Lowercase to uppercase", "man 3 toupper");
	ASSERT_EQ_INT(ft_toupper('a'), toupper('a'));
}
static void t2(void) {
	TEST_START(2, "ft_toupper('A')", "'A'", "Already uppercase", "man 3 toupper");
	ASSERT_EQ_INT(ft_toupper('A'), toupper('A'));
}
static void t3(void) {
	TEST_START(3, "ft_toupper('0')", "'0'", "Digits unchanged", "man 3 toupper");
	ASSERT_EQ_INT(ft_toupper('0'), toupper('0'));
}

void	suite_toupper(t_func_suite *s)
{
	s->func_name = "ft_toupper";
	s->part = 1;
	s->test_count = 3;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3};
	run_suite_tests(s, tests);
}
