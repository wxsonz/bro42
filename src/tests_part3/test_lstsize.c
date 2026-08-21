#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_lstsize(NULL)", "NULL", "Empty list returns 0", "Subject Section IV.4");
	ASSERT_EQ_INT(ft_lstsize(NULL), 0);
}
static void t2(void) {
	TEST_START(2, "ft_lstsize single node", "node", "Single node returns 1", "Subject Section IV.4");
	t_list *node = test_create_node("first");
	ASSERT_EQ_INT(ft_lstsize(node), 1);
	free(node);
}
static void t3(void) {
	TEST_START(3, "ft_lstsize 3 nodes", "n1->n2->n3", "3 nodes return 3", "Subject Section IV.4");
	t_list *n1 = test_create_node("1");
	t_list *n2 = test_create_node("2");
	t_list *n3 = test_create_node("3");
	n1->next = n2;
	n2->next = n3;
	ASSERT_EQ_INT(ft_lstsize(n1), 3);
	free(n1);
	free(n2);
	free(n3);
}

void	suite_lstsize(t_func_suite *s)
{
	s->func_name = "ft_lstsize";
	s->part = 3;
	s->test_count = 3;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3};
	run_suite_tests(s, tests);
}
