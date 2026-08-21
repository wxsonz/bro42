#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_memset(buf, 'A', 5)", "buf, 'A', 5", "Fill 5 bytes with 'A'", "man 3 memset");
	char b1[10] = "012345678";
	char b2[10] = "012345678";
	void *r1 = ft_memset(b1, 'A', 5);
	memset(b2, 'A', 5);
	ASSERT_TRUE(memcmp(b1, b2, 10) == 0 && r1 == b1, "b1 == b2", (memcmp(b1, b2, 10) == 0 ? "Match" : "Mismatch"));
}
static void t2(void) {
	TEST_START(2, "ft_memset(buf, 0, 10)", "buf, 0, 10", "Zero out buffer", "man 3 memset");
	char b1[10] = "012345678";
	char b2[10] = "012345678";
	ft_memset(b1, 0, 10);
	memset(b2, 0, 10);
	ASSERT_TRUE(memcmp(b1, b2, 10) == 0, "b1 == b2", (memcmp(b1, b2, 10) == 0 ? "Match" : "Mismatch"));
}
static void t3(void) {
	TEST_START(3, "ft_memset(buf, 'X', 0)", "buf, 'X', 0", "n = 0 leaves buffer untouched", "man 3 memset");
	char b1[10] = "012345678";
	char b2[10] = "012345678";
	size_t zero = 0;
	ft_memset(b1, 'X', zero);
	memset(b2, 'X', zero);
	ASSERT_TRUE(memcmp(b1, b2, 10) == 0, "b1 == b2", (memcmp(b1, b2, 10) == 0 ? "Match" : "Mismatch"));
}
static void t4(void) {
	TEST_START(4, "ft_memset(buf, 0x123441, 5)", "buf, 0x123441, 5", "c cast to unsigned char (0x41 = 'A')", "man 3 memset");
	char b1[10] = "012345678";
	char b2[10] = "012345678";
	ft_memset(b1, 0x123441, 5);
	memset(b2, 0x123441, 5);
	ASSERT_TRUE(memcmp(b1, b2, 10) == 0, "b1 == b2", (memcmp(b1, b2, 10) == 0 ? "Match" : "Mismatch"));
}

void	suite_memset(t_func_suite *s)
{
	s->func_name = "ft_memset";
	s->part = 1;
	s->test_count = 4;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3, t4};
	run_suite_tests(s, tests);
}
