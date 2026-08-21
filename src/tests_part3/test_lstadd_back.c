#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_lstadd_back to empty list", "&lst, node", "Sets head to new node", "Subject Section IV.4");
	t_list *lst = NULL;
	t_list *node = test_create_node("first");
	ft_lstadd_back(&lst, node);
	ASSERT_PTR_EQ(lst, node);
	ASSERT_NULL(lst->next);
	free(node);
}
static void t2(void) {
	TEST_START(2, "ft_lstadd_back to non-empty list", "&lst, node2", "Appends new node to end", "Subject Section IV.4");
	t_list *lst = NULL;
	t_list *node1 = test_create_node("first");
	t_list *node2 = test_create_node("second");
	ft_lstadd_back(&lst, node1);
	ft_lstadd_back(&lst, node2);
	ASSERT_PTR_EQ(lst, node1);
	ASSERT_PTR_EQ(node1->next, node2);
	ASSERT_NULL(node2->next);
	free(node1);
	free(node2);
}

void	suite_lstadd_back(t_func_suite *s)
{
	s->func_name = "ft_lstadd_back";
	s->part = 3;
	s->test_count = 2;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2};
	run_suite_tests(s, tests);
}
