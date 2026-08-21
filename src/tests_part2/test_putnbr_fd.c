#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_putnbr_fd(0, fd)", "0, fd", "Prints \"0\"", "Subject Section IV.3");
	int p[2];
	if (pipe(p) == -1) { record_test_ko("pipe ok", "pipe error"); return; }
	ft_putnbr_fd(0, p[1]);
	close(p[1]);
	char buf[32] = {0};
	read(p[0], buf, sizeof(buf) - 1);
	close(p[0]);
	ASSERT_EQ_STR(buf, "0");
}
static void t2(void) {
	TEST_START(2, "ft_putnbr_fd(42, fd)", "42, fd", "Prints \"42\"", "Subject Section IV.3");
	int p[2];
	if (pipe(p) == -1) { record_test_ko("pipe ok", "pipe error"); return; }
	ft_putnbr_fd(42, p[1]);
	close(p[1]);
	char buf[32] = {0};
	read(p[0], buf, sizeof(buf) - 1);
	close(p[0]);
	ASSERT_EQ_STR(buf, "42");
}
static void t3(void) {
	TEST_START(3, "ft_putnbr_fd(-42, fd)", "-42, fd", "Prints \"-42\"", "Subject Section IV.3");
	int p[2];
	if (pipe(p) == -1) { record_test_ko("pipe ok", "pipe error"); return; }
	ft_putnbr_fd(-42, p[1]);
	close(p[1]);
	char buf[32] = {0};
	read(p[0], buf, sizeof(buf) - 1);
	close(p[0]);
	ASSERT_EQ_STR(buf, "-42");
}
static void t4(void) {
	TEST_START(4, "ft_putnbr_fd(2147483647, fd)", "2147483647, fd", "Prints INT_MAX", "Subject Section IV.3");
	int p[2];
	if (pipe(p) == -1) { record_test_ko("pipe ok", "pipe error"); return; }
	ft_putnbr_fd(2147483647, p[1]);
	close(p[1]);
	char buf[32] = {0};
	read(p[0], buf, sizeof(buf) - 1);
	close(p[0]);
	ASSERT_EQ_STR(buf, "2147483647");
}
static void t5(void) {
	TEST_START(5, "ft_putnbr_fd(-2147483648, fd)", "-2147483648, fd", "Prints INT_MIN without arithmetic overflow", "Subject Section IV.3");
	int p[2];
	if (pipe(p) == -1) { record_test_ko("pipe ok", "pipe error"); return; }
	ft_putnbr_fd(-2147483648, p[1]);
	close(p[1]);
	char buf[32] = {0};
	read(p[0], buf, sizeof(buf) - 1);
	close(p[0]);
	ASSERT_EQ_STR(buf, "-2147483648");
}

void	suite_putnbr_fd(t_func_suite *s)
{
	s->func_name = "ft_putnbr_fd";
	s->part = 2;
	s->test_count = 5;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3, t4, t5};
	run_suite_tests(s, tests);
}
