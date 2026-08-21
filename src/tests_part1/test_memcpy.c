#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_memcpy(dest, \"hello\", 6)", "dest, \"hello\", 6", "Standard copy of 6 bytes", "man 3 memcpy");
	char d1[10] = "000000000";
	char d2[10] = "000000000";
	void *r1 = ft_memcpy(d1, "hello", 6);
	memcpy(d2, "hello", 6);
	ASSERT_TRUE(memcmp(d1, d2, 10) == 0 && r1 == d1, "d1 == d2", (memcmp(d1, d2, 10) == 0 ? "Match" : "Mismatch"));
}
static void t2(void) {
	TEST_START(2, "ft_memcpy(dest, \"hello\", 0)", "dest, \"hello\", 0", "n = 0 leaves dest untouched", "man 3 memcpy");
	char d1[10] = "000000000";
	char d2[10] = "000000000";
	size_t zero = 0;
	ft_memcpy(d1, "hello", zero);
	memcpy(d2, "hello", zero);
	ASSERT_TRUE(memcmp(d1, d2, 10) == 0, "d1 == d2", (memcmp(d1, d2, 10) == 0 ? "Match" : "Mismatch"));
}
static void t3(void) {
	TEST_START(3, "ft_memcpy(dest, binary, 8)", "dest, binary, 8", "Copies binary data with nulls", "man 3 memcpy");
	char src[8] = {'a', '\0', 'b', '\0', 'c', '\0', 'd', '\0'};
	char d1[8], d2[8];
	ft_memcpy(d1, src, 8);
	memcpy(d2, src, 8);
	ASSERT_TRUE(memcmp(d1, d2, 8) == 0, "d1 == d2", (memcmp(d1, d2, 8) == 0 ? "Match" : "Mismatch"));
}

void	suite_memcpy(t_func_suite *s)
{
	s->func_name = "ft_memcpy";
	s->part = 1;
	s->test_count = 3;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3};
	run_suite_tests(s, tests);
}
