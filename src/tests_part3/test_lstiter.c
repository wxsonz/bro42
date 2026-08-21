#include "tester.h"

static int g_iter_count = 0;
static void iter_func(void *content)
{
	(void)content;
	g_iter_count++;
}

static void t1(void) {
	TEST_START(1, "ft_lstiter iterates all nodes", "lst, f", "Applies f to every node content", "Subject Section IV.4");
	t_list *n1 = test_create_node("1");
	t_list *n2 = test_create_node("2");
	t_list *n3 = test_create_node("3");
	n1->next = n2;
	n2->next = n3;
	g_iter_count = 0;
	ft_lstiter(n1, iter_func);
	ASSERT_EQ_INT(g_iter_count, 3);
	free(n1);
	free(n2);
	free(n3);
}
static void t2(void) {
	TEST_START(2, "ft_lstiter with NULL list", "NULL, f", "No action, no crash", "Subject Section IV.4");
	g_iter_count = 0;
	ft_lstiter(NULL, iter_func);
	ASSERT_EQ_INT(g_iter_count, 0);
}

void	suite_lstiter(t_func_suite *s)
{
	s->func_name = "ft_lstiter";
	s->part = 3;
	s->test_count = 2;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2};
	run_suite_tests(s, tests);
}
