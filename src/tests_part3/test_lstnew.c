#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_lstnew(\"hello\")", "\"hello\"", "Creates new node with content", "Subject Section IV.4");
	t_list *node = ft_lstnew("hello");
	ASSERT_NOT_NULL(node);
	ASSERT_EQ_STR((char *)node->content, "hello");
	ASSERT_NULL(node->next);
	free(node);
}
static void t2(void) {
	TEST_START(2, "ft_lstnew(NULL)", "NULL", "Creates new node with NULL content", "Subject Section IV.4");
	t_list *node = ft_lstnew(NULL);
	ASSERT_NOT_NULL(node);
	ASSERT_NULL(node->content);
	ASSERT_NULL(node->next);
	free(node);
}

void	suite_lstnew(t_func_suite *s)
{
	s->func_name = "ft_lstnew";
	s->part = 3;
	s->test_count = 2;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2};
	run_suite_tests(s, tests);
}
