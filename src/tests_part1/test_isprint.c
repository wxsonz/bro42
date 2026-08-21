#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_isprint(32)", "32 (' ')", "Space is lowest printable ASCII", "man 3 isprint");
	ASSERT_EQ_INT(ft_isprint(32), 1);
}
static void t2(void) {
	TEST_START(2, "ft_isprint(126)", "126 ('~')", "Tilde is highest printable ASCII", "man 3 isprint");
	ASSERT_EQ_INT(ft_isprint(126), 1);
}
static void t3(void) {
	TEST_START(3, "ft_isprint(31)", "31", "Control character is non-printable", "man 3 isprint");
	ASSERT_EQ_INT(ft_isprint(31), 0);
}
static void t4(void) {
	TEST_START(4, "ft_isprint(127)", "127", "DEL is non-printable", "man 3 isprint");
	ASSERT_EQ_INT(ft_isprint(127), 0);
}
static void t5(void) {
	TEST_START(5, "ft_isprint('\n')", "'\\n'", "Newline is non-printable", "man 3 isprint");
	ASSERT_EQ_INT(ft_isprint('\n'), 0);
}

void	suite_isprint(t_func_suite *s)
{
	s->func_name = "ft_isprint";
	s->part = 1;
	s->test_count = 5;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3, t4, t5};
	run_suite_tests(s, tests);
}
