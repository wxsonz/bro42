#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_strlcpy(dst, \"hello\", 10)", "dst, \"hello\", 10", "Standard copy into larger buffer", "man 3 strlcpy");
	char d1[20] = "0000000000000000000";
	char d2[20] = "0000000000000000000";
	size_t r1 = ft_strlcpy(d1, "hello", 10);
	size_t r2 = oracle_strlcpy(d2, "hello", 10);
	ASSERT_TRUE(r1 == r2 && strcmp(d1, d2) == 0, "r1 == r2 && d1 == d2", (r1 == r2 ? "Match" : "Mismatch"));
}
static void t2(void) {
	TEST_START(2, "ft_strlcpy(dst, \"hello\", 3)", "dst, \"hello\", 3", "Truncates and null-terminates ('he\\0')", "man 3 strlcpy");
	char d1[20] = "0000000000000000000";
	char d2[20] = "0000000000000000000";
	size_t r1 = ft_strlcpy(d1, "hello", 3);
	size_t r2 = oracle_strlcpy(d2, "hello", 3);
	ASSERT_TRUE(r1 == r2 && strcmp(d1, d2) == 0, "r1 == r2 && d1 == d2", (r1 == r2 ? "Match" : "Mismatch"));
}
static void t3(void) {
	TEST_START(3, "ft_strlcpy(dst, \"hello\", 0)", "dst, \"hello\", 0", "size = 0 must NOT write to dst", "man 3 strlcpy");
	char d1[20] = "0000000000000000000";
	char d2[20] = "0000000000000000000";
	size_t r1 = ft_strlcpy(d1, "hello", 0);
	size_t r2 = oracle_strlcpy(d2, "hello", 0);
	ASSERT_TRUE(r1 == r2 && memcmp(d1, d2, 20) == 0, "r1 == r2 && d1 == d2", (r1 == r2 ? "Match" : "Mismatch"));
}
static void t4(void) {
	TEST_START(4, "ft_strlcpy(dst, \"hello\", 1)", "dst, \"hello\", 1", "size = 1 writes only '\\0'", "man 3 strlcpy");
	char d1[20] = "0000000000000000000";
	char d2[20] = "0000000000000000000";
	size_t r1 = ft_strlcpy(d1, "hello", 1);
	size_t r2 = oracle_strlcpy(d2, "hello", 1);
	ASSERT_TRUE(r1 == r2 && strcmp(d1, d2) == 0, "r1 == r2 && d1 == d2", (r1 == r2 ? "Match" : "Mismatch"));
}

void	suite_strlcpy(t_func_suite *s)
{
	s->func_name = "ft_strlcpy";
	s->part = 1;
	s->test_count = 4;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3, t4};
	run_suite_tests(s, tests);
}
