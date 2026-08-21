#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_substr standard extraction", "\"hello world\", 6, 5", "Extracts substring \"world\"", "Subject Section IV.3");
	char *res = ft_substr("hello world", 6, 5);
	ASSERT_NOT_NULL(res);
	ASSERT_EQ_STR(res, "world");
	free(res);
}
static void t2(void) {
	TEST_START(2, "ft_substr from start = 0", "\"hello world\", 0, 5", "Extracts substring \"hello\"", "Subject Section IV.3");
	char *res = ft_substr("hello world", 0, 5);
	ASSERT_NOT_NULL(res);
	ASSERT_EQ_STR(res, "hello");
	free(res);
}
static void t3(void) {
	TEST_START(3, "ft_substr start >= strlen(s)", "\"hello world\", 20, 5", "Returns allocated empty string \"\"", "Subject Section IV.3");
	char *res = ft_substr("hello world", 20, 5);
	ASSERT_NOT_NULL(res);
	ASSERT_EQ_STR(res, "");
	free(res);
}
static void t4(void) {
	TEST_START(4, "ft_substr len > remaining chars", "\"hello world\", 6, 50", "Does not over-allocate buffer", "Subject Section IV.3");
	char *res = ft_substr("hello world", 6, 50);
	ASSERT_NOT_NULL(res);
	ASSERT_EQ_STR(res, "world");
	free(res);
}

void	suite_substr(t_func_suite *s)
{
	s->func_name = "ft_substr";
	s->part = 2;
	s->test_count = 4;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3, t4};
	run_suite_tests(s, tests);
}
