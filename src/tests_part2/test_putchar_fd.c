#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_putchar_fd('A', fd)", "'A', fd", "Writes 'A' to file descriptor", "Subject Section IV.3");
	int p[2];
	if (pipe(p) == -1) { record_test_ko("pipe ok", "pipe error"); return; }
	ft_putchar_fd('A', p[1]);
	close(p[1]);
	char buf[16] = {0};
	read(p[0], buf, sizeof(buf) - 1);
	close(p[0]);
	ASSERT_EQ_STR(buf, "A");
}
static void t2(void) {
	TEST_START(2, "ft_putchar_fd('\\n', fd)", "'\\n', fd", "Writes newline to file descriptor", "Subject Section IV.3");
	int p[2];
	if (pipe(p) == -1) { record_test_ko("pipe ok", "pipe error"); return; }
	ft_putchar_fd('\n', p[1]);
	close(p[1]);
	char buf[16] = {0};
	read(p[0], buf, sizeof(buf) - 1);
	close(p[0]);
	ASSERT_EQ_STR(buf, "\n");
}

void	suite_putchar_fd(t_func_suite *s)
{
	s->func_name = "ft_putchar_fd";
	s->part = 2;
	s->test_count = 2;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2};
	run_suite_tests(s, tests);
}
