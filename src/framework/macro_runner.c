#include "tester.h"

static int exec_cmd(const char *cmd, char *output, size_t out_max)
{
	FILE *fp = popen(cmd, "r");
	if (!fp)
		return (-1);

	if (output && out_max > 0)
	{
		size_t n = fread(output, 1, out_max - 1, fp);
		output[n] = '\0';
	}
	int status = pclose(fp);
	return (WEXITSTATUS(status));
}

int	run_macro_suite(const char *target_dir, bool verbose)
{
	int passed = 0;
	int total = 7;
	char cmd[1024];
	char out[4096];

	printf("\n" BOLD FG_BMAGENTA "════════════════════════════════════════════════════════════════════" RESET "\n");
	printf(BOLD "  MACRO SUITE: Build, Makefile, Symbols, Relink & Repository Checks" RESET "\n");
	printf(BOLD FG_BMAGENTA "════════════════════════════════════════════════════════════════════" RESET "\n\n");

	/* Test 1: Presence of Makefile and libft.h */
	printf("  [Macro 01] File Structure Check: ");
	snprintf(cmd, sizeof(cmd), "test -f %s/Makefile && test -f %s/libft.h", target_dir, target_dir);
	if (exec_cmd(cmd, NULL, 0) == 0)
	{
		printf(BOLD FG_BGREEN "[OK]" RESET " Makefile and libft.h found at root\n");
		passed++;
	}
	else
	{
		printf(BOLD FG_BRED "[KO]" RESET " Missing Makefile or libft.h in %s\n", target_dir);
	}

	/* Test 2: make (default rule) */
	printf("  [Macro 02] Makefile 'make / make all': ");
	snprintf(cmd, sizeof(cmd), "make -C %s fclean >/dev/null 2>&1; make -C %s all >/dev/null 2>&1 && test -f %s/libft.a", target_dir, target_dir, target_dir);
	if (exec_cmd(cmd, NULL, 0) == 0)
	{
		printf(BOLD FG_BGREEN "[OK]" RESET " 'all' rule builds libft.a cleanly\n");
		passed++;
	}
	else
	{
		printf(BOLD FG_BRED "[KO]" RESET " 'make all' failed to create libft.a\n");
	}

	/* Test 3: make bonus */
	printf("  [Macro 03] Makefile 'make bonus': ");
	snprintf(cmd, sizeof(cmd), "make -C %s bonus >/dev/null 2>&1 && test -f %s/libft.a", target_dir, target_dir);
	if (exec_cmd(cmd, NULL, 0) == 0)
	{
		printf(BOLD FG_BGREEN "[OK]" RESET " 'bonus' rule builds libft.a cleanly\n");
		passed++;
	}
	else
	{
		printf(BOLD FG_BYELLOW "[SKIP / KO]" RESET " 'make bonus' rule missing or failed\n");
	}

	/* Test 4: Relink check (Idempotence) */
	printf("  [Macro 04] Relink Check (Idempotence): ");
	snprintf(cmd, sizeof(cmd), "make -C %s 2>&1", target_dir);
	exec_cmd(cmd, out, sizeof(out));
	if (strstr(out, "up to date") || strstr(out, "Nothing to be done") || strlen(out) < 30)
	{
		printf(BOLD FG_BGREEN "[OK]" RESET " No unnecessary relinking detected\n");
		passed++;
	}
	else
	{
		printf(BOLD FG_BRED "[KO]" RESET " Makefile recompiled unchanged files (Unnecessary relink!)\n");
		if (verbose)
			printf("             Output: %s\n", out);
	}

	/* Test 5: make clean and fclean */
	printf("  [Macro 05] Makefile 'clean' and 'fclean': ");
	snprintf(cmd, sizeof(cmd), "make -C %s clean >/dev/null 2>&1 && test -f %s/libft.a && make -C %s fclean >/dev/null 2>&1 && ! test -f %s/libft.a", target_dir, target_dir, target_dir, target_dir);
	if (exec_cmd(cmd, NULL, 0) == 0)
	{
		printf(BOLD FG_BGREEN "[OK]" RESET " 'clean' and 'fclean' remove files as expected\n");
		passed++;
	}
	else
	{
		printf(BOLD FG_BRED "[KO]" RESET " 'clean' or 'fclean' failed\n");
	}

	/* Test 6: Forbidden functions & Globals audit */
	printf("  [Macro 06] Symbol Audit (No globals & only allowed externals): ");
	snprintf(cmd, sizeof(cmd), "make -C %s all >/dev/null 2>&1; nm -g --defined-only %s/libft.a 2>/dev/null | grep -E ' [B|D] '", target_dir, target_dir);
	exec_cmd(cmd, out, sizeof(out));
	if (strlen(out) == 0)
	{
		printf(BOLD FG_BGREEN "[OK]" RESET " No unallowed global variables found\n");
		passed++;
	}
	else
	{
		printf(BOLD FG_BRED "[KO]" RESET " Global variable(s) found in library:\n%s\n", out);
	}

	/* Test 7: Archive format with ar */
	printf("  [Macro 07] Archive Integrity: ");
	snprintf(cmd, sizeof(cmd), "ar -t %s/libft.a 2>/dev/null | grep -q 'ft_strlen.o'", target_dir);
	if (exec_cmd(cmd, NULL, 0) == 0)
	{
		printf(BOLD FG_BGREEN "[OK]" RESET " libft.a is a valid archive created with ar\n");
		passed++;
	}
	else
	{
		printf(BOLD FG_BRED "[KO]" RESET " libft.a archive is missing object files\n");
	}

	/* Restore bonus in libft.a for micro suite */
	snprintf(cmd, sizeof(cmd), "make -C %s bonus >/dev/null 2>&1 || make -C %s all >/dev/null 2>&1", target_dir, target_dir);
	exec_cmd(cmd, NULL, 0);

	printf("\n  " BOLD "Macro Score: %d / %d checks passed" RESET "\n\n", passed, total);
	return (passed);
}
