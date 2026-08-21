#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_itoa(0)", "0", "Converts 0 to \"0\"", "Subject Section IV.3");
	char *res = ft_itoa(0);
	ASSERT_NOT_NULL(res);
	ASSERT_EQ_STR(res, "0");
	free(res);
}
static void t2(void) {
	TEST_START(2, "ft_itoa(42)", "42", "Converts 42 to \"42\"", "Subject Section IV.3");
	char *res = ft_itoa(42);
	ASSERT_NOT_NULL(res);
	ASSERT_EQ_STR(res, "42");
	free(res);
}
static void t3(void) {
	TEST_START(3, "ft_itoa(-42)", "-42", "Converts -42 to \"-42\"", "Subject Section IV.3");
	char *res = ft_itoa(-42);
	ASSERT_NOT_NULL(res);
	ASSERT_EQ_STR(res, "-42");
	free(res);
}
static void t4(void) {
	TEST_START(4, "ft_itoa(2147483647)", "2147483647", "INT_MAX", "Subject Section IV.3");
	char *res = ft_itoa(2147483647);
	ASSERT_NOT_NULL(res);
	ASSERT_EQ_STR(res, "2147483647");
	free(res);
}
static void t5(void) {
	TEST_START(5, "ft_itoa(-2147483648)", "-2147483648", "INT_MIN arithmetic overflow", "Subject Section IV.3");
	char *res = ft_itoa(-2147483648);
	ASSERT_NOT_NULL(res);
	ASSERT_EQ_STR(res, "-2147483648");
	free(res);
}

void	suite_itoa(t_func_suite *s)
{
	s->func_name = "ft_itoa";
	s->part = 2;
	s->test_count = 5;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3, t4, t5};
	run_suite_tests(s, tests);
}
