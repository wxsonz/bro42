#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_memchr(\"abcdef\", 'c', 6)", "\"abcdef\", 'c', 6", "Find character in buffer", "man 3 memchr");
	const char *s = "abcdef";
	ASSERT_PTR_EQ(ft_memchr(s, 'c', 6), memchr(s, 'c', 6));
}
static void t2(void) {
	TEST_START(2, "ft_memchr(\"abcdef\", 'z', 6)", "\"abcdef\", 'z', 6", "Not found returns NULL", "man 3 memchr");
	const char *s = "abcdef";
	ASSERT_PTR_EQ(ft_memchr(s, 'z', 6), memchr(s, 'z', 6));
}
static void t3(void) {
	TEST_START(3, "ft_memchr(\"abc\\0def\", 'd', 7)", "\"abc\\0def\", 'd', 7", "Searches past null byte", "man 3 memchr");
	const char *s = "abc\0def";
	ASSERT_PTR_EQ(ft_memchr(s, 'd', 7), memchr(s, 'd', 7));
}
static void t4(void) {
	TEST_START(4, "ft_memchr unsigned char 0x81", "\"\\200\\201\\202\", 0x81, 3", "Raw unsigned byte search", "man 3 memchr");
	const char *s = "\200\201\202";
	ASSERT_PTR_EQ(ft_memchr(s, 0x81, 3), memchr(s, 0x81, 3));
}

void	suite_memchr(t_func_suite *s)
{
	s->func_name = "ft_memchr";
	s->part = 1;
	s->test_count = 4;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3, t4};
	run_suite_tests(s, tests);
}
