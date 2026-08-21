#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_isalpha('a')", "'a'", "Lowercase letter must return 1", "man 3 isalpha");
	ASSERT_EQ_INT(ft_isalpha('a'), (isalpha('a') != 0 ? 1 : 0));
}
static void t2(void) {
	TEST_START(2, "ft_isalpha('z')", "'z'", "Lowercase letter must return 1", "man 3 isalpha");
	ASSERT_EQ_INT(ft_isalpha('z'), (isalpha('z') != 0 ? 1 : 0));
}
static void t3(void) {
	TEST_START(3, "ft_isalpha('A')", "'A'", "Uppercase letter must return 1", "man 3 isalpha");
	ASSERT_EQ_INT(ft_isalpha('A'), (isalpha('A') != 0 ? 1 : 0));
}
static void t4(void) {
	TEST_START(4, "ft_isalpha('Z')", "'Z'", "Uppercase letter must return 1", "man 3 isalpha");
	ASSERT_EQ_INT(ft_isalpha('Z'), (isalpha('Z') != 0 ? 1 : 0));
}
static void t5(void) {
	TEST_START(5, "ft_isalpha('0')", "'0'", "Digit must return 0", "man 3 isalpha");
	ASSERT_EQ_INT(ft_isalpha('0'), 0);
}
static void t6(void) {
	TEST_START(6, "ft_isalpha('a' - 1)", "'`'", "Character right before 'a' must return 0", "man 3 isalpha");
	ASSERT_EQ_INT(ft_isalpha('a' - 1), 0);
}
static void t7(void) {
	TEST_START(7, "ft_isalpha('z' + 1)", "'{'", "Character right after 'z' must return 0", "man 3 isalpha");
	ASSERT_EQ_INT(ft_isalpha('z' + 1), 0);
}
static void t8(void) {
	TEST_START(8, "ft_isalpha(-1)", "-1", "Negative input must return 0", "man 3 isalpha");
	ASSERT_EQ_INT(ft_isalpha(-1), 0);
}

void	suite_isalpha(t_func_suite *s)
{
	s->func_name = "ft_isalpha";
	s->part = 1;
	s->test_count = 8;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3, t4, t5, t6, t7, t8};
	run_suite_tests(s, tests);
}
