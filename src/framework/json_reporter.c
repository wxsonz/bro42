#include "tester.h"

static void escape_json_str(FILE *fp, const char *str)
{
	if (!str)
	{
		fprintf(fp, "null");
		return ;
	}
	fputc('"', fp);
	while (*str)
	{
		if (*str == '"')
			fprintf(fp, "\\\"");
		else if (*str == '\\')
			fprintf(fp, "\\\\");
		else if (*str == '\n')
			fprintf(fp, "\\n");
		else if (*str == '\r')
			fprintf(fp, "\\r");
		else if (*str == '\t')
			fprintf(fp, "\\t");
		else if ((unsigned char)*str < 32)
			fprintf(fp, "\\u%04x", (unsigned char)*str);
		else
			fputc(*str, fp);
		str++;
	}
	fputc('"', fp);
}

static void write_report_stream(FILE *fp, t_func_suite *suites, int suite_count, int macro_score)
{
	int total_tests = 0;
	int passed_tests = 0;
	int failed_tests = 0;
	int crashed_tests = 0;
	int timed_out_tests = 0;
	int leaked_tests = 0;
	int passed_funcs = 0;
	int not_found_funcs = 0;

	for (int i = 0; i < suite_count; i++)
	{
		if (suites[i].not_found)
		{
			not_found_funcs++;
			continue ;
		}
		total_tests += suites[i].test_count;
		passed_tests += suites[i].pass_count;
		failed_tests += suites[i].fail_count;
		if (suites[i].fail_count == 0 && suites[i].test_count > 0)
			passed_funcs++;

		for (int j = 0; j < suites[i].test_count; j++)
		{
			if (suites[i].cases[j].status == TEST_SIGSEGV || suites[i].cases[j].status == TEST_SIGBUS || suites[i].cases[j].status == TEST_SIGABRT || suites[i].cases[j].status == TEST_SIGFPE)
				crashed_tests++;
			else if (suites[i].cases[j].status == TEST_TIMEOUT)
				timed_out_tests++;
			else if (suites[i].cases[j].status == TEST_LEAK)
				leaked_tests++;
		}
	}

	fprintf(fp, "{\n");
	fprintf(fp, "  \"summary\": {\n");
	fprintf(fp, "    \"total_funcs\": %d,\n", suite_count);
	fprintf(fp, "    \"passed_funcs\": %d,\n", passed_funcs);
	fprintf(fp, "    \"not_found_funcs\": %d,\n", not_found_funcs);
	fprintf(fp, "    \"total_tests\": %d,\n", total_tests);
	fprintf(fp, "    \"passed_tests\": %d,\n", passed_tests);
	fprintf(fp, "    \"failed_tests\": %d,\n", failed_tests);
	fprintf(fp, "    \"crashed_tests\": %d,\n", crashed_tests);
	fprintf(fp, "    \"timed_out_tests\": %d,\n", timed_out_tests);
	fprintf(fp, "    \"leaked_tests\": %d,\n", leaked_tests);
	fprintf(fp, "    \"macro_score\": %d,\n", macro_score);
	fprintf(fp, "    \"macro_total\": 7\n");
	fprintf(fp, "  },\n");

	fprintf(fp, "  \"suites\": [\n");
	for (int i = 0; i < suite_count; i++)
	{
		fprintf(fp, "    {\n");
		fprintf(fp, "      \"func_name\": \"%s\",\n", suites[i].func_name);
		fprintf(fp, "      \"part\": %d,\n", suites[i].part);
		fprintf(fp, "      \"not_found\": %s,\n", suites[i].not_found ? "true" : "false");
		fprintf(fp, "      \"test_count\": %d,\n", suites[i].test_count);
		fprintf(fp, "      \"pass_count\": %d,\n", suites[i].pass_count);
		fprintf(fp, "      \"fail_count\": %d,\n", suites[i].fail_count);
		fprintf(fp, "      \"cases\": [\n");

		int limit = suites[i].not_found ? 0 : suites[i].test_count;
		for (int j = 0; j < limit; j++)
		{
			t_test_case *tc = &suites[i].cases[j];
			const char *status_str = "OK";
			if (tc->status == TEST_NOT_FOUND) status_str = "NOT_FOUND";
			else if (tc->status == TEST_KO) status_str = "KO";
			else if (tc->status == TEST_SIGSEGV) status_str = "SIGSEGV";
			else if (tc->status == TEST_SIGBUS) status_str = "SIGBUS";
			else if (tc->status == TEST_SIGABRT) status_str = "SIGABRT";
			else if (tc->status == TEST_SIGFPE) status_str = "SIGFPE";
			else if (tc->status == TEST_TIMEOUT) status_str = "TIMEOUT";
			else if (tc->status == TEST_LEAK) status_str = "LEAK";

			fprintf(fp, "        {\n");
			fprintf(fp, "          \"test_num\": %d,\n", tc->test_num);
			fprintf(fp, "          \"desc\": "); escape_json_str(fp, tc->desc); fprintf(fp, ",\n");
			fprintf(fp, "          \"input\": "); escape_json_str(fp, tc->input); fprintf(fp, ",\n");
			fprintf(fp, "          \"expected\": "); escape_json_str(fp, tc->expected); fprintf(fp, ",\n");
			fprintf(fp, "          \"actual\": "); escape_json_str(fp, tc->actual); fprintf(fp, ",\n");
			fprintf(fp, "          \"status\": \"%s\",\n", status_str);
			fprintf(fp, "          \"hint\": "); escape_json_str(fp, tc->hint); fprintf(fp, ",\n");
			fprintf(fp, "          \"ref\": "); escape_json_str(fp, tc->ref); fprintf(fp, "\n");
			fprintf(fp, "        }%s\n", (j + 1 < limit) ? "," : "");
		}

		fprintf(fp, "      ]\n");
		fprintf(fp, "    }%s\n", (i + 1 < suite_count) ? "," : "");
	}
	fprintf(fp, "  ]\n");
	fprintf(fp, "}\n");
}

void	generate_json_report(const char *filepath, t_func_suite *suites, int suite_count, int macro_score)
{
	/* 1. Write report.json */
	FILE *fp_json = fopen(filepath, "w");
	if (fp_json)
	{
		write_report_stream(fp_json, suites, suite_count, macro_score);
		fclose(fp_json);
	}

	/* 2. Write report.js */
	char js_path[2048];
	strncpy(js_path, filepath, sizeof(js_path) - 1);
	char *dot = strrchr(js_path, '.');
	if (dot && strcmp(dot, ".json") == 0)
		strcpy(dot, ".js");
	else
		strncat(js_path, ".js", sizeof(js_path) - strlen(js_path) - 1);

	FILE *fp_js = fopen(js_path, "w");
	if (fp_js)
	{
		fprintf(fp_js, "window.REPORT_DATA = ");
		write_report_stream(fp_js, suites, suite_count, macro_score);
		fprintf(fp_js, ";\n");
		fclose(fp_js);
	}
}
