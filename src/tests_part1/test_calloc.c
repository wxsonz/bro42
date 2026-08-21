#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_calloc(5, sizeof(int))", "5, sizeof(int)", "Allocates and zeroes 20 bytes", "man 3 calloc");
	int *p = (int *)ft_calloc(5, sizeof(int));
	ASSERT_NOT_NULL(p);
	for (int i = 0; i < 5; i++)
	{
		if (p[i] != 0) {
			free(p);
			record_test_ko("All elements 0", "Non-zero element found");
			return ;
		}
	}
	free(p);
	record_test_ok("Zero-filled and freed");
}
static void t2(void) {
	TEST_START(2, "ft_calloc(0, 0)", "0, 0", "Returns unique freeable pointer (Subject note)", "Subject Section IV.2");
	void *p = ft_calloc(0, 0);
	ASSERT_NOT_NULL(p);
	free(p);
	record_test_ok("Valid pointer freed");
}
static void t3(void) {
	TEST_START(3, "ft_calloc(SIZE_MAX, 2)", "SIZE_MAX, 2", "Multiplication overflow check returns NULL", "man 3 calloc");
	void *p = ft_calloc(SIZE_MAX, 2);
	ASSERT_NULL(p);
}

void	suite_calloc(t_func_suite *s)
{
	s->func_name = "ft_calloc";
	s->part = 1;
	s->test_count = 3;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3};
	run_suite_tests(s, tests);
}
