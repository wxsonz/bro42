#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_lstadd_front to empty list", "&lst, node1", "Sets head to new node", "Subject Section IV.4");
	t_list *lst = NULL;
	t_list *node = test_create_node("first");
	ft_lstadd_front(&lst, node);
	ASSERT_PTR_EQ(lst, node);
	ASSERT_NULL(lst->next);
	free(node);
}
static void t2(void) {
	TEST_START(2, "ft_lstadd_front to non-empty list", "&lst, node2", "Prepends new node to head", "Subject Section IV.4");
	t_list *lst = NULL;
	t_list *node1 = test_create_node("first");
	t_list *node2 = test_create_node("second");
	ft_lstadd_front(&lst, node1);
	ft_lstadd_front(&lst, node2);
	ASSERT_PTR_EQ(lst, node2);
	ASSERT_PTR_EQ(lst->next, node1);
	free(node1);
	free(node2);
}

void	suite_lstadd_front(t_func_suite *s)
{
	s->func_name = "ft_lstadd_front";
	s->part = 3;
	s->test_count = 2;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2};
	run_suite_tests(s, tests);
}
