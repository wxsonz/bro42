#include "tester.h"

static char map_char_plus_idx(unsigned int i, char c)
{
	return (char)(c + i);
}

static void t1(void) {
	TEST_START(1, "ft_strmapi(\"abc\", f)", "\"abc\", f", "Applies function with index", "Subject Section IV.3");
	char *res = ft_strmapi("abc", map_char_plus_idx);
	ASSERT_NOT_NULL(res);
	ASSERT_EQ_STR(res, "ace");
	free(res);
}
static void t2(void) {
	TEST_START(2, "ft_strmapi(\"\", f)", "\"\", f", "Empty string returns allocated \"\"", "Subject Section IV.3");
	char *res = ft_strmapi("", map_char_plus_idx);
	ASSERT_NOT_NULL(res);
	ASSERT_EQ_STR(res, "");
	free(res);
}

void	suite_strmapi(t_func_suite *s)
{
	s->func_name = "ft_strmapi";
	s->part = 2;
	s->test_count = 2;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2};
	run_suite_tests(s, tests);
}
