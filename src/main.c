#include "tester.h"

typedef struct s_suite_entry
{
	const char	*name;
	int			part;
	void		(*suite_fn)(t_func_suite *s);
}	t_suite_entry;

static t_suite_entry g_all_suites[] = {
	/* Part 1: Libc functions */
	{"ft_isalpha", 1, suite_isalpha},
	{"ft_isdigit", 1, suite_isdigit},
	{"ft_isalnum", 1, suite_isalnum},
	{"ft_isascii", 1, suite_isascii},
	{"ft_isprint", 1, suite_isprint},
	{"ft_strlen", 1, suite_strlen},
	{"ft_memset", 1, suite_memset},
	{"ft_bzero", 1, suite_bzero},
	{"ft_memcpy", 1, suite_memcpy},
	{"ft_memmove", 1, suite_memmove},
	{"ft_strlcpy", 1, suite_strlcpy},
	{"ft_strlcat", 1, suite_strlcat},
	{"ft_toupper", 1, suite_toupper},
	{"ft_tolower", 1, suite_tolower},
	{"ft_strchr", 1, suite_strchr},
	{"ft_strrchr", 1, suite_strrchr},
	{"ft_strncmp", 1, suite_strncmp},
	{"ft_memchr", 1, suite_memchr},
	{"ft_memcmp", 1, suite_memcmp},
	{"ft_strnstr", 1, suite_strnstr},
	{"ft_atoi", 1, suite_atoi},
	{"ft_calloc", 1, suite_calloc},
	{"ft_strdup", 1, suite_strdup},

	/* Part 2: Additional functions */
	{"ft_substr", 2, suite_substr},
	{"ft_strjoin", 2, suite_strjoin},
	{"ft_strtrim", 2, suite_strtrim},
	{"ft_split", 2, suite_split},
	{"ft_itoa", 2, suite_itoa},
	{"ft_strmapi", 2, suite_strmapi},
	{"ft_striteri", 2, suite_striteri},
	{"ft_putchar_fd", 2, suite_putchar_fd},
	{"ft_putstr_fd", 2, suite_putstr_fd},
	{"ft_putendl_fd", 2, suite_putendl_fd},
	{"ft_putnbr_fd", 2, suite_putnbr_fd},

	/* Part 3: Linked list functions */
	{"ft_lstnew", 3, suite_lstnew},
	{"ft_lstadd_front", 3, suite_lstadd_front},
	{"ft_lstsize", 3, suite_lstsize},
	{"ft_lstlast", 3, suite_lstlast},
	{"ft_lstadd_back", 3, suite_lstadd_back},
	{"ft_lstdelone", 3, suite_lstdelone},
	{"ft_lstclear", 3, suite_lstclear},
	{"ft_lstiter", 3, suite_lstiter},
	{"ft_lstmap", 3, suite_lstmap},

	{NULL, 0, NULL}
};

static void print_banner(const char *target_dir)
{
	printf("\n" BOLD FG_BCYAN "╔══════════════════════════════════════════════════════════════════╗" RESET "\n");
	printf(BOLD FG_BCYAN "║" RESET BOLD FG_BWHITE "             🚀 ft_bro - Libft Testing & Guidance                 " RESET BOLD FG_BCYAN "║" RESET "\n");
	printf(BOLD FG_BCYAN "╚══════════════════════════════════════════════════════════════════╝" RESET "\n");
	printf("  " DIM "Target Directory : " RESET BOLD "%s" RESET "\n", target_dir);
}

int	main(int argc, char **argv)
{
	const char	*target_dir = ".";
	const char	*filter = NULL;
	bool		run_macro = true;
	bool		run_micro = true;
	bool		open_web = true;
	int			macro_score = 7;

	/* Parse arguments */
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "--no-web") == 0 || strcmp(argv[i], "--cli-only") == 0)
			open_web = false;
		else if (strcmp(argv[i], "--web-only") == 0)
		{
			launch_browser(NULL);
			return (0);
		}
		else if (strcmp(argv[i], "--json") == 0)
			g_json_mode = true;
		else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0)
			g_verbose = true;
		else if (strcmp(argv[i], "macro") == 0)
		{
			run_micro = false;
			run_macro = true;
		}
		else if (strcmp(argv[i], "micro") == 0)
		{
			run_macro = false;
			run_micro = true;
		}
		else if (strcmp(argv[i], "part1") == 0 || strcmp(argv[i], "part2") == 0 || strcmp(argv[i], "part3") == 0)
		{
			run_macro = false;
			filter = argv[i];
		}
		else if (strncmp(argv[i], "ft_", 3) == 0)
		{
			run_macro = false;
			filter = argv[i];
		}
		else if (argv[i][0] != '-')
		{
			target_dir = argv[i];
		}
	}

	if (!g_json_mode)
		print_banner(target_dir);

	/* Run Macro Suite */
	if (run_macro)
		macro_score = run_macro_suite(target_dir, g_verbose);

	/* Run Micro Suite */
	t_func_suite suites[MAX_FUNCS];
	int suite_count = 0;
	t_suite_summary summary = {0};

	if (run_micro)
	{
		for (int i = 0; g_all_suites[i].name != NULL; i++)
		{
			if (filter)
			{
				if (strcmp(filter, "part1") == 0 && g_all_suites[i].part != 1) continue;
				if (strcmp(filter, "part2") == 0 && g_all_suites[i].part != 2) continue;
				if (strcmp(filter, "part3") == 0 && g_all_suites[i].part != 3) continue;
				if (strncmp(filter, "ft_", 3) == 0 && strcmp(filter, g_all_suites[i].name) != 0) continue;
			}

			memset(&suites[suite_count], 0, sizeof(t_func_suite));
			g_all_suites[i].suite_fn(&suites[suite_count]);

			summary.total_funcs++;
			if (suites[suite_count].not_found)
			{
				summary.not_found_funcs++;
			}
			else
			{
				if (suites[suite_count].fail_count == 0)
					summary.passed_funcs++;
				summary.total_tests += suites[suite_count].test_count;
				summary.passed_tests += suites[suite_count].pass_count;
				summary.failed_tests += suites[suite_count].fail_count;

				for (int j = 0; j < suites[suite_count].test_count; j++)
				{
					if (suites[suite_count].cases[j].status == TEST_SIGSEGV || suites[suite_count].cases[j].status == TEST_SIGBUS)
						summary.crashed_tests++;
					else if (suites[suite_count].cases[j].status == TEST_TIMEOUT)
						summary.timed_out_tests++;
					else if (suites[suite_count].cases[j].status == TEST_LEAK)
						summary.leaked_tests++;
				}
			}
			suite_count++;
		}
		print_total_summary(&summary);
	}

	/* Generate JSON and JS report for Web Dashboard */
	char report_path[2048];
	const char *base_dir = getenv("FT_BRO_DIR");
	if (!base_dir)
		base_dir = getenv("FT_COMPANION_DIR");
	if (!base_dir)
		base_dir = ".";
	snprintf(report_path, sizeof(report_path), "%s/web/report.json", base_dir);
	generate_json_report(report_path, suites, suite_count, macro_score);

	/* Launch Web Dashboard if enabled */
	if (open_web && !g_json_mode)
	{
		printf("  " BOLD FG_BCYAN "🌐 Web Dashboard: " FG_BWHITE UNDERLINE "file://%s/web/index.html" RESET "\n\n", base_dir);
		launch_browser(NULL);
	}

	return (summary.failed_tests == 0 && summary.not_found_funcs == 0 ? 0 : 1);
}
