#include "tester.h"

void	launch_browser(const char *url_or_file)
{
	char cmd[4096];
	char full_path[1024];
	const char *base_dir = getenv("FT_BRO_DIR");
	if (!base_dir)
		base_dir = getenv("FT_COMPANION_DIR");
	if (!base_dir)
		base_dir = ".";

	if (url_or_file && (strstr(url_or_file, "http://") || strstr(url_or_file, "https://") || strstr(url_or_file, "file://")))
	{
		strncpy(full_path, url_or_file, sizeof(full_path) - 1);
		full_path[sizeof(full_path) - 1] = '\0';
	}
	else
	{
		snprintf(full_path, sizeof(full_path), "file://%s/web/index.html", base_dir);
	}

#if defined(__APPLE__)
	snprintf(cmd, sizeof(cmd), "open \"%s\" >/dev/null 2>&1 &", full_path);
#else
	snprintf(cmd, sizeof(cmd), "xdg-open \"%s\" >/dev/null 2>&1 || sensible-browser \"%s\" >/dev/null 2>&1 &", full_path, full_path);
#endif

	int ret = system(cmd);
	(void)ret;
}
