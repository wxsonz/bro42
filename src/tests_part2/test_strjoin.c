#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_strjoin(\"hello \", \"world\")", "\"hello \", \"world\"", "Joins two strings", "Subject Section IV.3");
	char *res = ft_strjoin("hello ", "world");
	ASSERT_NOT_NULL(res);
	ASSERT_EQ_STR(res, "hello world");
	free(res);
}
static void t2(void) {
	TEST_START(2, "ft_strjoin(\"\", \"world\")", "\"\", \"world\"", "Empty prefix", "Subject Section IV.3");
	char *res = ft_strjoin("", "world");
	ASSERT_NOT_NULL(res);
	ASSERT_EQ_STR(res, "world");
	free(res);
}
static void t3(void) {
	TEST_START(3, "ft_strjoin(\"hello\", \"\")", "\"hello\", \"\"", "Empty suffix", "Subject Section IV.3");
	char *res = ft_strjoin("hello", "");
	ASSERT_NOT_NULL(res);
	ASSERT_EQ_STR(res, "hello");
	free(res);
}
static void t4(void) {
	TEST_START(4, "ft_strjoin(\"\", \"\")", "\"\", \"\"", "Two empty strings return \"\"", "Subject Section IV.3");
	char *res = ft_strjoin("", "");
	ASSERT_NOT_NULL(res);
	ASSERT_EQ_STR(res, "");
	free(res);
}

void	suite_strjoin(t_func_suite *s)
{
	s->func_name = "ft_strjoin";
	s->part = 2;
	s->test_count = 4;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3, t4};
	run_suite_tests(s, tests);
}
