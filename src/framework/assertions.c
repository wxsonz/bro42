#include "tester.h"

t_test_ipc	g_current_ipc = {0};
bool		g_verbose = true;
bool		g_json_mode = false;

void	set_test_info(int num, const char *desc, const char *input, const char *hint, const char *ref)
{
	memset(&g_current_ipc, 0, sizeof(t_test_ipc));
	g_current_ipc.test_num = num;
	if (desc)
		strncpy(g_current_ipc.desc, desc, sizeof(g_current_ipc.desc) - 1);
	if (input)
		strncpy(g_current_ipc.input, input, sizeof(g_current_ipc.input) - 1);
	if (hint)
		strncpy(g_current_ipc.hint, hint, sizeof(g_current_ipc.hint) - 1);
	if (ref)
		strncpy(g_current_ipc.ref, ref, sizeof(g_current_ipc.ref) - 1);
}

void	record_test_ok(const char *actual_val)
{
	g_current_ipc.status = TEST_OK;
	if (actual_val)
		strncpy(g_current_ipc.actual, actual_val, sizeof(g_current_ipc.actual) - 1);
}

void	record_test_ko(const char *expected_val, const char *actual_val)
{
	g_current_ipc.status = TEST_KO;
	if (expected_val)
		strncpy(g_current_ipc.expected, expected_val, sizeof(g_current_ipc.expected) - 1);
	if (actual_val)
		strncpy(g_current_ipc.actual, actual_val, sizeof(g_current_ipc.actual) - 1);
}

void	record_test_crash(int sig)
{
	g_current_ipc.signal_num = sig;
	if (sig == SIGSEGV)
		g_current_ipc.status = TEST_SIGSEGV;
	else if (sig == SIGBUS)
		g_current_ipc.status = TEST_SIGBUS;
	else if (sig == SIGFPE)
		g_current_ipc.status = TEST_SIGFPE;
	else
		g_current_ipc.status = TEST_SIGABRT;
}

void	record_test_timeout(void)
{
	g_current_ipc.status = TEST_TIMEOUT;
}

void	record_test_leak(size_t leaked_bytes)
{
	char buf[64];
	snprintf(buf, sizeof(buf), "%zu bytes leaked", leaked_bytes);
	g_current_ipc.status = TEST_LEAK;
	strncpy(g_current_ipc.actual, buf, sizeof(g_current_ipc.actual) - 1);
}

void	print_suite_header(const char *func_name, int part)
{
	if (g_json_mode)
		return ;

	printf("\n" BOLD FG_BCYAN "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" RESET "\n");
	printf(BOLD "  %-25s " RESET DIM "(Part %d)" RESET "\n", func_name, part);
	printf(BOLD FG_BCYAN "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" RESET "\n");
}

void	print_not_found(const char *func_name)
{
	if (g_json_mode)
		return ;

	printf("  " BG_RED FG_BWHITE " %s NOT FOUND " RESET "\n\n", func_name);
}

void	print_case_result(const t_test_case *tc)
{
	if (g_json_mode)
		return ;

	if (tc->status == TEST_NOT_FOUND)
	{
		printf("  " BG_RED FG_BWHITE " NOT FOUND " RESET "\n");
		return ;
	}

	printf("  Test %02d: ", tc->test_num);

	switch (tc->status)
	{
		case TEST_OK:
			printf(BOLD FG_BGREEN "[OK]" RESET);
			if (g_verbose && tc->desc)
				printf(" %s", tc->desc);
			printf("\n");
			break;

		case TEST_KO:
			printf(BOLD FG_BRED "[KO]" RESET);
			if (tc->desc)
				printf(" %s", tc->desc);
			printf("\n");
			if (tc->input && tc->input[0])
				printf("          " DIM "Input   : " RESET "%s\n", tc->input);
			if (tc->expected && tc->expected[0])
				printf("          " FG_GREEN "Expected: " RESET "%s\n", tc->expected);
			if (tc->actual && tc->actual[0])
				printf("          " FG_RED "Actual  : " RESET "%s\n", tc->actual);
			if (tc->hint && tc->hint[0])
				printf("          " FG_BYELLOW "💡 Hint  : " RESET "%s\n", tc->hint);
			if (tc->ref && tc->ref[0])
				printf("          " FG_CYAN "📖 Ref   : " RESET "%s\n", tc->ref);
			break;

		case TEST_SIGSEGV:
			printf(BOLD FG_BMAGENTA "[SIGSEGV: Segmentation Fault]" RESET);
			if (tc->desc)
				printf(" %s", tc->desc);
			printf("\n");
			if (tc->input && tc->input[0])
				printf("          " DIM "Input : " RESET "%s\n", tc->input);
			if (tc->hint && tc->hint[0])
				printf("          " FG_BYELLOW "💡 Hint: " RESET "%s\n", tc->hint);
			break;

		case TEST_SIGBUS:
			printf(BOLD FG_BMAGENTA "[SIGBUS: Bus Error]" RESET);
			if (tc->desc)
				printf(" %s", tc->desc);
			printf("\n");
			if (tc->hint && tc->hint[0])
				printf("          " FG_BYELLOW "💡 Hint: " RESET "%s\n", tc->hint);
			break;

		case TEST_TIMEOUT:
			printf(BOLD FG_BYELLOW "[TIMEOUT: Infinite Loop > 2s]" RESET);
			if (tc->desc)
				printf(" %s", tc->desc);
			printf("\n");
			if (tc->hint && tc->hint[0])
				printf("          " FG_BYELLOW "💡 Hint: " RESET "%s\n", tc->hint);
			break;

		case TEST_LEAK:
			printf(BOLD FG_BCYAN "[LEAK: Memory Leak Detected]" RESET);
			if (tc->actual && tc->actual[0])
				printf(" (%s)", tc->actual);
			printf("\n");
			if (tc->hint && tc->hint[0])
				printf("          " FG_BYELLOW "💡 Hint: " RESET "%s\n", tc->hint);
			break;

		default:
			printf(BOLD FG_BRED "[FAIL]" RESET "\n");
			break;
	}
}

void	print_suite_summary(const t_func_suite *suite)
{
	if (g_json_mode)
		return ;

	if (suite->not_found)
		return ;

	if (suite->fail_count == 0)
		printf("  " BOLD FG_BGREEN "Result: %d/%d tests passed (100%%)" RESET "\n", suite->pass_count, suite->test_count);
	else
		printf("  " BOLD FG_BRED "Result: %d/%d tests passed (%d failed)" RESET "\n", suite->pass_count, suite->test_count, suite->fail_count);
}

void	print_total_summary(const t_suite_summary *summary)
{
	if (g_json_mode)
		return ;

	printf("\n" BOLD "╔══════════════════════════════════════════════════════════════════╗" RESET "\n");
	printf(BOLD "║                     TEST SUITE SUMMARY                           ║" RESET "\n");
	printf(BOLD "╠══════════════════════════════════════════════════════════════════╣" RESET "\n");
	printf("  Functions Tested : %d / %d\n", summary->passed_funcs, summary->total_funcs);
	if (summary->not_found_funcs > 0)
		printf("  " BG_RED FG_BWHITE " Not Found (404)  : %d " RESET "\n", summary->not_found_funcs);
	printf("  Total Unit Tests : %d\n", summary->total_tests);
	printf("  " FG_BGREEN "Passed Tests     : %d" RESET "\n", summary->passed_tests);
	if (summary->failed_tests > 0)
		printf("  " FG_BRED "Failed (KO)      : %d" RESET "\n", summary->failed_tests);
	if (summary->crashed_tests > 0)
		printf("  " FG_BMAGENTA "Crashes (SEGV)   : %d" RESET "\n", summary->crashed_tests);
	if (summary->timed_out_tests > 0)
		printf("  " FG_BYELLOW "Timeouts (Loop)  : %d" RESET "\n", summary->timed_out_tests);

	if (summary->failed_tests == 0 && summary->crashed_tests == 0 && summary->timed_out_tests == 0 && summary->not_found_funcs == 0)
		printf("\n  " BOLD BG_GREEN FG_BWHITE "  ALL TESTS PASSED! CONGRATULATIONS! 🎉  " RESET "\n");
	else
		printf("\n  " BOLD BG_RED FG_BWHITE "  SOME TESTS FAILED. CHECK HINTS ABOVE! 🔍  " RESET "\n");
	printf(BOLD "╚══════════════════════════════════════════════════════════════════╝" RESET "\n\n");
}
