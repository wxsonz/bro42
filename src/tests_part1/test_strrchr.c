#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_strrchr(\"tripouille\", 'l')", "\"tripouille\", 'l'", "Find last occurrence of 'l'", "man 3 strrchr");
	const char *s = "tripouille";
	ASSERT_PTR_EQ(ft_strrchr(s, 'l'), strrchr(s, 'l'));
}
static void t2(void) {
	TEST_START(2, "ft_strrchr(\"tripouille\", 'z')", "\"tripouille\", 'z'", "Not found returns NULL", "man 3 strrchr");
	const char *s = "tripouille";
	ASSERT_PTR_EQ(ft_strrchr(s, 'z'), strrchr(s, 'z'));
}
static void t3(void) {
	TEST_START(3, "ft_strrchr(\"tripouille\", '\\0')", "\"tripouille\", '\\0'", "Find null-terminator", "man 3 strrchr");
	const char *s = "tripouille";
	ASSERT_PTR_EQ(ft_strrchr(s, '\0'), strrchr(s, '\0'));
}
static void t4(void) {
	TEST_START(4, "ft_strrchr(\"tripouille\", 'l' + 256)", "\"tripouille\", 'l'+256", "c cast to char", "man 3 strrchr");
	const char *s = "tripouille";
	ASSERT_PTR_EQ(ft_strrchr(s, 'l' + 256), strrchr(s, 'l' + 256));
}

void	suite_strrchr(t_func_suite *s)
{
	s->func_name = "ft_strrchr";
	s->part = 1;
	s->test_count = 4;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3, t4};
	run_suite_tests(s, tests);
}
