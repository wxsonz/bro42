#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_memcmp identical buffers", "\"abcdef\", \"abcdef\", 6", "Equal buffers return 0", "man 3 memcmp");
	ASSERT_EQ_INT(ft_memcmp("abcdef", "abcdef", 6), 0);
}
static void t2(void) {
	TEST_START(2, "ft_memcmp past null byte", "\"abc\\0def\", \"abc\\0deg\", 7", "Compares raw bytes past '\\0'", "man 3 memcmp");
	int r1 = ft_memcmp("abc\0def", "abc\0deg", 7);
	int r2 = memcmp("abc\0def", "abc\0deg", 7);
	ASSERT_TRUE((r1 < 0 && r2 < 0) || (r1 == r2), "Sign matches", (r1 < 0 ? "Negative" : "Non-negative"));
}
static void t3(void) {
	TEST_START(3, "ft_memcmp unsigned char comparison", "\"\\200\", \"\\0\", 1", "Must compare as unsigned char", "man 3 memcmp");
	int r1 = ft_memcmp("\200", "\0", 1);
	ASSERT_TRUE(r1 > 0, "Positive (>0)", (r1 > 0 ? "Positive" : "Non-positive"));
}
static void t4(void) {
	TEST_START(4, "ft_memcmp n = 0", "\"abc\", \"xyz\", 0", "n=0 returns 0", "man 3 memcmp");
	ASSERT_EQ_INT(ft_memcmp("abc", "xyz", 0), 0);
}

void	suite_memcmp(t_func_suite *s)
{
	s->func_name = "ft_memcmp";
	s->part = 1;
	s->test_count = 4;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3, t4};
	run_suite_tests(s, tests);
}
