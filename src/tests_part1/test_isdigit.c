#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_isdigit('0')", "'0'", "Digit '0' must return 1", "man 3 isdigit");
	ASSERT_EQ_INT(ft_isdigit('0'), (isdigit('0') != 0 ? 1 : 0));
}
static void t2(void) {
	TEST_START(2, "ft_isdigit('9')", "'9'", "Digit '9' must return 1", "man 3 isdigit");
	ASSERT_EQ_INT(ft_isdigit('9'), (isdigit('9') != 0 ? 1 : 0));
}
static void t3(void) {
	TEST_START(3, "ft_isdigit('5')", "'5'", "Digit '5' must return 1", "man 3 isdigit");
	ASSERT_EQ_INT(ft_isdigit('5'), 1);
}
static void t4(void) {
	TEST_START(4, "ft_isdigit('0' - 1)", "'/'", "Character before '0' must return 0", "man 3 isdigit");
	ASSERT_EQ_INT(ft_isdigit('0' - 1), 0);
}
static void t5(void) {
	TEST_START(5, "ft_isdigit('9' + 1)", "':'", "Character after '9' must return 0", "man 3 isdigit");
	ASSERT_EQ_INT(ft_isdigit('9' + 1), 0);
}
static void t6(void) {
	TEST_START(6, "ft_isdigit('a')", "'a'", "Letter must return 0", "man 3 isdigit");
	ASSERT_EQ_INT(ft_isdigit('a'), 0);
}

void	suite_isdigit(t_func_suite *s)
{
	s->func_name = "ft_isdigit";
	s->part = 1;
	s->test_count = 6;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3, t4, t5, t6};
	run_suite_tests(s, tests);
}
