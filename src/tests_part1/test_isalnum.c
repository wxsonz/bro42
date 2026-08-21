#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_isalnum('a')", "'a'", "Letter must return 1", "man 3 isalnum");
	ASSERT_EQ_INT(ft_isalnum('a'), 1);
}
static void t2(void) {
	TEST_START(2, "ft_isalnum('Z')", "'Z'", "Letter must return 1", "man 3 isalnum");
	ASSERT_EQ_INT(ft_isalnum('Z'), 1);
}
static void t3(void) {
	TEST_START(3, "ft_isalnum('5')", "'5'", "Digit must return 1", "man 3 isalnum");
	ASSERT_EQ_INT(ft_isalnum('5'), 1);
}
static void t4(void) {
	TEST_START(4, "ft_isalnum(' ')", "' '", "Space must return 0", "man 3 isalnum");
	ASSERT_EQ_INT(ft_isalnum(' '), 0);
}
static void t5(void) {
	TEST_START(5, "ft_isalnum('@')", "'@'", "Special char must return 0", "man 3 isalnum");
	ASSERT_EQ_INT(ft_isalnum('@'), 0);
}

void	suite_isalnum(t_func_suite *s)
{
	s->func_name = "ft_isalnum";
	s->part = 1;
	s->test_count = 5;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3, t4, t5};
	run_suite_tests(s, tests);
}
