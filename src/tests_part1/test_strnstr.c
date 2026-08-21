#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_strnstr find substring", "\"Hello World\", \"World\", 11", "Find needle in haystack", "man 3 strnstr");
	const char *big = "Hello World";
	ASSERT_PTR_EQ(ft_strnstr(big, "World", 11), oracle_strnstr(big, "World", 11));
}
static void t2(void) {
	TEST_START(2, "ft_strnstr needle past len", "\"Hello World\", \"World\", 10", "Needle exceeds search len", "man 3 strnstr");
	const char *big = "Hello World";
	ASSERT_PTR_EQ(ft_strnstr(big, "World", 10), oracle_strnstr(big, "World", 10));
}
static void t3(void) {
	TEST_START(3, "ft_strnstr empty needle", "\"Hello World\", \"\", 5", "Empty needle returns big", "man 3 strnstr");
	const char *big = "Hello World";
	ASSERT_PTR_EQ(ft_strnstr(big, "", 5), oracle_strnstr(big, "", 5));
}
static void t4(void) {
	TEST_START(4, "ft_strnstr duplicate prefix", "\"aaabc\", \"aabc\", 5", "Matches correct prefix", "man 3 strnstr");
	const char *big = "aaabc";
	ASSERT_PTR_EQ(ft_strnstr(big, "aabc", 5), oracle_strnstr(big, "aabc", 5));
}

void	suite_strnstr(t_func_suite *s)
{
	s->func_name = "ft_strnstr";
	s->part = 1;
	s->test_count = 4;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3, t4};
	run_suite_tests(s, tests);
}
