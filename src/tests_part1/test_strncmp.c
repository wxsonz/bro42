#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_strncmp identical", "\"abcdef\", \"abcdef\", 6", "Identical strings return 0", "man 3 strncmp");
	ASSERT_EQ_INT(ft_strncmp("abcdef", "abcdef", 6), 0);
}
static void t2(void) {
	TEST_START(2, "ft_strncmp s1 < s2", "\"abc\", \"abd\", 3", "Returns negative difference", "man 3 strncmp");
	int r1 = ft_strncmp("abc", "abd", 3);
	int r2 = strncmp("abc", "abd", 3);
	ASSERT_TRUE((r1 < 0 && r2 < 0) || (r1 == r2), "Sign matches", (r1 < 0 ? "Negative" : "Non-negative"));
}
static void t3(void) {
	TEST_START(3, "ft_strncmp s1 > s2", "\"abd\", \"abc\", 3", "Returns positive difference", "man 3 strncmp");
	int r1 = ft_strncmp("abd", "abc", 3);
	int r2 = strncmp("abd", "abc", 3);
	ASSERT_TRUE((r1 > 0 && r2 > 0) || (r1 == r2), "Sign matches", (r1 > 0 ? "Positive" : "Non-positive"));
}
static void t4(void) {
	TEST_START(4, "ft_strncmp n stops early", "\"abc\", \"abd\", 2", "n=2 matches first 2 chars", "man 3 strncmp");
	ASSERT_EQ_INT(ft_strncmp("abc", "abd", 2), 0);
}
static void t5(void) {
	TEST_START(5, "ft_strncmp unsigned char comparison", "\"test\\200\", \"test\\0\", 6", "Must compare characters as unsigned char", "man 3 strncmp");
	int r1 = ft_strncmp("test\200", "test\0", 6);
	ASSERT_TRUE(r1 > 0, "Positive (>0)", (r1 > 0 ? "Positive" : "Non-positive"));
}
static void t6(void) {
	TEST_START(6, "ft_strncmp n = 0", "\"abc\", \"xyz\", 0", "n=0 returns 0", "man 3 strncmp");
	ASSERT_EQ_INT(ft_strncmp("abc", "xyz", 0), 0);
}

void	suite_strncmp(t_func_suite *s)
{
	s->func_name = "ft_strncmp";
	s->part = 1;
	s->test_count = 6;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3, t4, t5, t6};
	run_suite_tests(s, tests);
}
