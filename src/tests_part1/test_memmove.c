#include "tester.h"

static void t1(void) {
	TEST_START(1, "Non-overlapping memmove", "dest, \"hello\", 6", "Standard non-overlapping copy", "man 3 memmove");
	char d1[10] = "000000000";
	char d2[10] = "000000000";
	ft_memmove(d1, "hello", 6);
	memmove(d2, "hello", 6);
	ASSERT_TRUE(memcmp(d1, d2, 10) == 0, "d1 == d2", (memcmp(d1, d2, 10) == 0 ? "Match" : "Mismatch"));
}
static void t2(void) {
	TEST_START(2, "Overlap dest > src (buf + 2, buf, 5)", "buf+2, buf, 5", "Must copy backwards to prevent source overwrite", "man 3 memmove");
	char b1[10] = "012345678";
	char b2[10] = "012345678";
	ft_memmove(b1 + 2, b1, 5);
	memmove(b2 + 2, b2, 5);
	ASSERT_TRUE(memcmp(b1, b2, 10) == 0, "b1 == b2", (memcmp(b1, b2, 10) == 0 ? "Match" : "Mismatch"));
}
static void t3(void) {
	TEST_START(3, "Overlap dest < src (buf, buf + 2, 5)", "buf, buf+2, 5", "Must copy forwards", "man 3 memmove");
	char b1[10] = "012345678";
	char b2[10] = "012345678";
	ft_memmove(b1, b1 + 2, 5);
	memmove(b2, b2 + 2, 5);
	ASSERT_TRUE(memcmp(b1, b2, 10) == 0, "b1 == b2", (memcmp(b1, b2, 10) == 0 ? "Match" : "Mismatch"));
}
static void t4(void) {
	TEST_START(4, "dest == src", "buf, buf, 5", "No change", "man 3 memmove");
	char b1[10] = "012345678";
	char b2[10] = "012345678";
	ft_memmove(b1, b1, 5);
	memmove(b2, b2, 5);
	ASSERT_TRUE(memcmp(b1, b2, 10) == 0, "b1 == b2", (memcmp(b1, b2, 10) == 0 ? "Match" : "Mismatch"));
}

void	suite_memmove(t_func_suite *s)
{
	s->func_name = "ft_memmove";
	s->part = 1;
	s->test_count = 4;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3, t4};
	run_suite_tests(s, tests);
}
