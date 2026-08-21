#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_bzero(buf, 5)", "buf, 5", "Zero out first 5 bytes", "man 3 bzero");
	char b1[10] = "123456789";
	char b2[10] = "123456789";
	ft_bzero(b1, 5);
	bzero(b2, 5);
	ASSERT_TRUE(memcmp(b1, b2, 10) == 0, "b1 == b2", (memcmp(b1, b2, 10) == 0 ? "Match" : "Mismatch"));
}
static void t2(void) {
	TEST_START(2, "ft_bzero(buf, 0)", "buf, 0", "n = 0 leaves buffer untouched", "man 3 bzero");
	char b1[10] = "123456789";
	char b2[10] = "123456789";
	ft_bzero(b1, 0);
	bzero(b2, 0);
	ASSERT_TRUE(memcmp(b1, b2, 10) == 0, "b1 == b2", (memcmp(b1, b2, 10) == 0 ? "Match" : "Mismatch"));
}

void	suite_bzero(t_func_suite *s)
{
	s->func_name = "ft_bzero";
	s->part = 1;
	s->test_count = 2;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2};
	run_suite_tests(s, tests);
}
