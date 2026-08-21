#include "tester.h"

static int g_clear_del_count = 0;
static void clear_del(void *content)
{
	(void)content;
	g_clear_del_count++;
}

static void t1(void) {
	TEST_START(1, "ft_lstclear clears all nodes", "&lst, del", "Frees all nodes and sets *lst = NULL", "Subject Section IV.4");
	t_list *n1 = test_create_node(strdup("1"));
	t_list *n2 = test_create_node(strdup("2"));
	t_list *n3 = test_create_node(strdup("3"));
	n1->next = n2;
	n2->next = n3;
	t_list *lst = n1;
	ft_lstclear(&lst, free);
	ASSERT_NULL(lst);
}
static void t2(void) {
	TEST_START(2, "ft_lstclear with NULL lst", "NULL, del", "Does not crash on NULL head", "Subject Section IV.4");
	t_list *lst = NULL;
	ft_lstclear(&lst, clear_del);
	ASSERT_NULL(lst);
}

void	suite_lstclear(t_func_suite *s)
{
	s->func_name = "ft_lstclear";
	s->part = 3;
	s->test_count = 2;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2};
	run_suite_tests(s, tests);
}
