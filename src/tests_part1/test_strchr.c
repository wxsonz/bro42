#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_strchr(\"tripouille\", 't')", "\"tripouille\", 't'", "Find first char", "man 3 strchr");
	const char *s = "tripouille";
	ASSERT_PTR_EQ(ft_strchr(s, 't'), strchr(s, 't'));
}
static void t2(void) {
	TEST_START(2, "ft_strchr(\"tripouille\", 'l')", "\"tripouille\", 'l'", "Find first occurrence of 'l'", "man 3 strchr");
	const char *s = "tripouille";
	ASSERT_PTR_EQ(ft_strchr(s, 'l'), strchr(s, 'l'));
}
static void t3(void) {
	TEST_START(3, "ft_strchr(\"tripouille\", 'z')", "\"tripouille\", 'z'", "Not found returns NULL", "man 3 strchr");
	const char *s = "tripouille";
	ASSERT_PTR_EQ(ft_strchr(s, 'z'), strchr(s, 'z'));
}
static void t4(void) {
	TEST_START(4, "ft_strchr(\"tripouille\", '\\0')", "\"tripouille\", '\\0'", "Find null-terminator", "man 3 strchr");
	const char *s = "tripouille";
	ASSERT_PTR_EQ(ft_strchr(s, '\0'), strchr(s, '\0'));
}
static void t5(void) {
	TEST_START(5, "ft_strchr(\"tripouille\", 't' + 256)", "\"tripouille\", 't'+256", "c cast to char", "man 3 strchr");
	const char *s = "tripouille";
	ASSERT_PTR_EQ(ft_strchr(s, 't' + 256), strchr(s, 't' + 256));
}

void	suite_strchr(t_func_suite *s)
{
	s->func_name = "ft_strchr";
	s->part = 1;
	s->test_count = 5;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3, t4, t5};
	run_suite_tests(s, tests);
}
