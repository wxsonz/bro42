#include "tester.h"

static void iter_char_plus_idx(unsigned int i, char *c)
{
	*c = (char)(*c + i);
}

static void t1(void) {
	TEST_START(1, "ft_striteri in-place modification", "\"abc\", f", "Modifies string in place with index", "Subject Section IV.3");
	char str[] = "abc";
	ft_striteri(str, iter_char_plus_idx);
	ASSERT_EQ_STR(str, "ace");
}
static void t2(void) {
	TEST_START(2, "ft_striteri empty string", "\"\", f", "Empty string untouched", "Subject Section IV.3");
	char str[] = "";
	ft_striteri(str, iter_char_plus_idx);
	ASSERT_EQ_STR(str, "");
}

void	suite_striteri(t_func_suite *s)
{
	s->func_name = "ft_striteri";
	s->part = 2;
	s->test_count = 2;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2};
	run_suite_tests(s, tests);
}
