#include "tester.h"

static void *map_strdup(void *content)
{
	return strdup((char *)content);
}

static void t1(void) {
	TEST_START(1, "ft_lstmap creates new mapped list", "lst, f, del", "Transforms contents and preserves original", "Subject Section IV.4");
	t_list *n1 = test_create_node("1");
	t_list *n2 = test_create_node("2");
	n1->next = n2;

	t_list *new_list = ft_lstmap(n1, map_strdup, free);
	ASSERT_NOT_NULL(new_list);
	ASSERT_EQ_STR((char *)new_list->content, "1");
	ASSERT_EQ_STR((char *)new_list->next->content, "2");
	ASSERT_NULL(new_list->next->next);

	test_free_list_with_del(new_list, free);
	free(n1);
	free(n2);
}
static void t2(void) {
	TEST_START(2, "ft_lstmap with NULL list", "NULL, f, del", "Returns NULL", "Subject Section IV.4");
	t_list *res = ft_lstmap(NULL, map_strdup, free);
	ASSERT_NULL(res);
}

void	suite_lstmap(t_func_suite *s)
{
	s->func_name = "ft_lstmap";
	s->part = 3;
	s->test_count = 2;
	s->pass_count = 0;
	s->fail_count = 0;

	void (*tests[])(void) = {t1, t2};
	run_suite_tests(s, tests);
}
