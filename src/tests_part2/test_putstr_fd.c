#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_putstr_fd(\"Hello 42\", fd)", "\"Hello 42\", fd", "Writes string to fd", "Subject Section IV.3");
	int p[2];
	if (pipe(p) == -1) { record_test_ko("pipe ok", "pipe error"); return; }
	ft_putstr_fd("Hello 42", p[1]);
	close(p[1]);
	char buf[32] = {0};
	read(p[0], buf, sizeof(buf) - 1);
	close(p[0]);
	ASSERT_EQ_STR(buf, "Hello 42");
}
static void t2(void) {
	TEST_START(2, "ft_putstr_fd(\"\", fd)", "\"\", fd", "Writes empty string to fd", "Subject Section IV.3");
	int p[2];
	if (pipe(p) == -1) { record_test_ko("pipe ok", "pipe error"); return; }
	ft_putstr_fd("", p[1]);
	close(p[1]);
	char buf[32] = {0};
	read(p[0], buf, sizeof(buf) - 1);
	close(p[0]);
	ASSERT_EQ_STR(buf, "");
}

void	suite_putstr_fd(t_func_suite *s)
{
	s->func_name = "ft_putstr_fd";
	s->part = 2;
	s->test_count = 2;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2};
	run_suite_tests(s, tests);
}
