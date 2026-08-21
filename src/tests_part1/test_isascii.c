#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_isascii(0)", "0", "0 is ASCII", "man 3 isascii");
	ASSERT_EQ_INT(ft_isascii(0), 1);
}
static void t2(void) {
	TEST_START(2, "ft_isascii(127)", "127", "127 is ASCII", "man 3 isascii");
	ASSERT_EQ_INT(ft_isascii(127), 1);
}
static void t3(void) {
	TEST_START(3, "ft_isascii('A')", "'A'", "'A' is ASCII", "man 3 isascii");
	ASSERT_EQ_INT(ft_isascii('A'), 1);
}
static void t4(void) {
	TEST_START(4, "ft_isascii(-1)", "-1", "Negative input is not ASCII", "man 3 isascii");
	ASSERT_EQ_INT(ft_isascii(-1), 0);
}
static void t5(void) {
	TEST_START(5, "ft_isascii(128)", "128", "128 is not ASCII", "man 3 isascii");
	ASSERT_EQ_INT(ft_isascii(128), 0);
}

void	suite_isascii(t_func_suite *s)
{
	s->func_name = "ft_isascii";
	s->part = 1;
	s->test_count = 5;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3, t4, t5};
	run_suite_tests(s, tests);
}
