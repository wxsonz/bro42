#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_strlcat standard append", "dst=\"hello\", src=\" world\", size=15", "Normal concatenation", "man 3 strlcat");
	char d1[30] = "hello";
	char d2[30] = "hello";
	size_t r1 = ft_strlcat(d1, " world", 15);
	size_t r2 = oracle_strlcat(d2, " world", 15);
	ASSERT_TRUE(r1 == r2 && strcmp(d1, d2) == 0, "r1 == r2 && d1 == d2", (r1 == r2 ? "Match" : "Mismatch"));
}
static void t2(void) {
	TEST_START(2, "ft_strlcat with truncate", "dst=\"hello\", src=\" world\", size=8", "Truncates properly", "man 3 strlcat");
	char d1[30] = "hello";
	char d2[30] = "hello";
	size_t r1 = ft_strlcat(d1, " world", 8);
	size_t r2 = oracle_strlcat(d2, " world", 8);
	ASSERT_TRUE(r1 == r2 && strcmp(d1, d2) == 0, "r1 == r2 && d1 == d2", (r1 == r2 ? "Match" : "Mismatch"));
}
static void t3(void) {
	TEST_START(3, "ft_strlcat size <= strlen(dst)", "dst=\"hello\", src=\" world\", size=5", "size <= strlen(dst) returns size + strlen(src)", "man 3 strlcat");
	char d1[30] = "hello";
	char d2[30] = "hello";
	size_t r1 = ft_strlcat(d1, " world", 5);
	size_t r2 = oracle_strlcat(d2, " world", 5);
	ASSERT_TRUE(r1 == r2 && strcmp(d1, d2) == 0, "r1 == r2 && d1 == d2", (r1 == r2 ? "Match" : "Mismatch"));
}
static void t4(void) {
	TEST_START(4, "ft_strlcat size = 0", "dst=\"hello\", src=\" world\", size=0", "size = 0 returns 0 + strlen(src)", "man 3 strlcat");
	char d1[30] = "hello";
	char d2[30] = "hello";
	size_t r1 = ft_strlcat(d1, " world", 0);
	size_t r2 = oracle_strlcat(d2, " world", 0);
	ASSERT_TRUE(r1 == r2 && strcmp(d1, d2) == 0, "r1 == r2 && d1 == d2", (r1 == r2 ? "Match" : "Mismatch"));
}

void	suite_strlcat(t_func_suite *s)
{
	s->func_name = "ft_strlcat";
	s->part = 1;
	s->test_count = 4;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3, t4};
	run_suite_tests(s, tests);
}
