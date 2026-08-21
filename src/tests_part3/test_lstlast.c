#include "tester.h"

static void t1(void) {
	TEST_START(1, "ft_lstlast(NULL)", "NULL", "Empty list returns NULL", "Subject Section IV.4");
	ASSERT_NULL(ft_lstlast(NULL));
}
static void t2(void) {
	TEST_START(2, "ft_lstlast single node", "node", "Single node returns itself", "Subject Section IV.4");
	t_list *node = test_create_node("first");
	ASSERT_PTR_EQ(ft_lstlast(node), node);
	free(node);
}
static void t3(void) {
	TEST_START(3, "ft_lstlast 3 nodes", "n1->n2->n3", "Returns last node n3", "Subject Section IV.4");
	t_list *n1 = test_create_node("1");
	t_list *n2 = test_create_node("2");
	t_list *n3 = test_create_node("3");
	n1->next = n2;
	n2->next = n3;
	ASSERT_PTR_EQ(ft_lstlast(n1), n3);
	free(n1);
	free(n2);
	free(n3);
}

void	suite_lstlast(t_func_suite *s)
{
	s->func_name = "ft_lstlast";
	s->part = 3;
	s->test_count = 3;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2, t3};
	run_suite_tests(s, tests);
}
