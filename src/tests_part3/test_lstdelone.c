#include "tester.h"

static int g_del_called = 0;
static void custom_del(void *content)
{
	(void)content;
	g_del_called++;
}

static void t1(void) {
	TEST_START(1, "ft_lstdelone frees content and node", "node, del", "Calls del on content and frees node", "Subject Section IV.4");
	t_list *node = test_create_node(strdup("hello"));
	g_del_called = 0;
	ft_lstdelone(node, free);
	ASSERT_EQ_INT(1, 1);
}
static void t2(void) {
	TEST_START(2, "ft_lstdelone does not free next", "node, del", "Preserves next pointer logic", "Subject Section IV.4");
	t_list *n1 = test_create_node("1");
	t_list *n2 = test_create_node("2");
	n1->next = n2;
	g_del_called = 0;
	ft_lstdelone(n1, custom_del);
	ASSERT_EQ_INT(g_del_called, 1);
	free(n2);
}

void	suite_lstdelone(t_func_suite *s)
{
	s->func_name = "ft_lstdelone";
	s->part = 3;
	s->test_count = 2;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2};
	run_suite_tests(s, tests);
}
