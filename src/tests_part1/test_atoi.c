#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_atoi(\"42\")", "\"42\"", "Basic positive number", "man 3 atoi");
	ASSERT_EQ_INT(ft_atoi("42"), atoi("42"));
}
static void t2(void) {
	TEST_START(2, "ft_atoi(\"   -42\")", "\"   -42\"", "Spaces and negative sign", "man 3 atoi");
	ASSERT_EQ_INT(ft_atoi("   -42"), atoi("   -42"));
}
static void t3(void) {
	TEST_START(3, "ft_atoi(\"  +42\")", "\"  +42\"", "Spaces and positive sign", "man 3 atoi");
	ASSERT_EQ_INT(ft_atoi("  +42"), atoi("  +42"));
}
static void t4(void) {
	TEST_START(4, "ft_atoi all isspace chars", "\"\\t\\n\\v\\f\\r 123\"", "Handles \\t\\n\\v\\f\\r space", "man 3 atoi");
	ASSERT_EQ_INT(ft_atoi("\t\n\v\f\r 123"), atoi("\t\n\v\f\r 123"));
}
static void t5(void) {
	TEST_START(5, "ft_atoi(\"2147483647\")", "\"2147483647\"", "INT_MAX", "man 3 atoi");
	ASSERT_EQ_INT(ft_atoi("2147483647"), 2147483647);
}
static void t6(void) {
	TEST_START(6, "ft_atoi(\"-2147483648\")", "\"-2147483648\"", "INT_MIN", "man 3 atoi");
	ASSERT_EQ_INT(ft_atoi("-2147483648"), -2147483648);
}
static void t7(void) {
	TEST_START(7, "ft_atoi multiple signs", "\"--42\"", "Multiple signs return 0", "man 3 atoi");
	ASSERT_EQ_INT(ft_atoi("--42"), 0);
}
static void t8(void) {
	TEST_START(8, "ft_atoi stops at char", "\"42abc5\"", "Stops at non-digit char", "man 3 atoi");
	ASSERT_EQ_INT(ft_atoi("42abc5"), 42);
}

void	suite_atoi(t_func_suite *s)
{
	s->func_name = "ft_atoi";
	s->part = 1;
	s->test_count = 8;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3, t4, t5, t6, t7, t8};
	run_suite_tests(s, tests);
}
