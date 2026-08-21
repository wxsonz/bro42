#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_strtrim spaces from both ends", "\"   hello world   \", \" \"", "Trims leading and trailing spaces", "Subject Section IV.3");
	char *res = ft_strtrim("   hello world   ", " ");
	ASSERT_NOT_NULL(res);
	ASSERT_EQ_STR(res, "hello world");
	free(res);
}
static void t2(void) {
	TEST_START(2, "ft_strtrim multiple chars in set", "\"xxxyyyhello worldyyyxxx\", \"xy\"", "Trims all chars in set", "Subject Section IV.3");
	char *res = ft_strtrim("xxxyyyhello worldyyyxxx", "xy");
	ASSERT_NOT_NULL(res);
	ASSERT_EQ_STR(res, "hello world");
	free(res);
}
static void t3(void) {
	TEST_START(3, "ft_strtrim all chars trimmed", "\"xxxxxx\", \"x\"", "Returns allocated \"\"", "Subject Section IV.3");
	char *res = ft_strtrim("xxxxxx", "x");
	ASSERT_NOT_NULL(res);
	ASSERT_EQ_STR(res, "");
	free(res);
}
static void t4(void) {
	TEST_START(4, "ft_strtrim no chars trimmed", "\"hello\", \"xyz\"", "Returns duplicate of s1", "Subject Section IV.3");
	char *res = ft_strtrim("hello", "xyz");
	ASSERT_NOT_NULL(res);
	ASSERT_EQ_STR(res, "hello");
	free(res);
}

void	suite_strtrim(t_func_suite *s)
{
	s->func_name = "ft_strtrim";
	s->part = 2;
	s->test_count = 4;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3, t4};
	run_suite_tests(s, tests);
}
