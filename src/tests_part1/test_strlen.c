#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_strlen(\"\")", "\"\"", "Empty string length is 0", "man 3 strlen");
	ASSERT_EQ_SIZE(ft_strlen(""), strlen(""));
}
static void t2(void) {
	TEST_START(2, "ft_strlen(\"a\")", "\"a\"", "Single character string length is 1", "man 3 strlen");
	ASSERT_EQ_SIZE(ft_strlen("a"), strlen("a"));
}
static void t3(void) {
	TEST_START(3, "ft_strlen(\"Hello 42!\")", "\"Hello 42!\"", "Standard string length", "man 3 strlen");
	ASSERT_EQ_SIZE(ft_strlen("Hello 42!"), strlen("Hello 42!"));
}
static void t4(void) {
	TEST_START(4, "ft_strlen(\"12345678901234567890\")", "\"12345678901234567890\"", "20 chars", "man 3 strlen");
	ASSERT_EQ_SIZE(ft_strlen("12345678901234567890"), strlen("12345678901234567890"));
}

void	suite_strlen(t_func_suite *s)
{
	s->func_name = "ft_strlen";
	s->part = 1;
	s->test_count = 4;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3, t4};
	run_suite_tests(s, tests);
}
