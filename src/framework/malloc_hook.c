#include "tester.h"

static bool		g_hook_enabled = false;
static int		g_fail_at = -1;
static int		g_alloc_count = 0;
static size_t	g_active_allocs = 0;
static size_t	g_total_bytes = 0;

void	malloc_hook_enable(void)
{
	g_hook_enabled = true;
	g_alloc_count = 0;
	g_active_allocs = 0;
	g_total_bytes = 0;
	g_fail_at = -1;
}

void	malloc_hook_disable(void)
{
	g_hook_enabled = false;
}

void	malloc_hook_set_fail_at(int count)
{
	g_fail_at = count;
}

void	malloc_hook_reset(void)
{
	g_alloc_count = 0;
	g_active_allocs = 0;
	g_total_bytes = 0;
	g_fail_at = -1;
}

size_t	malloc_hook_get_active_allocs(void)
{
	return (g_active_allocs);
}

size_t	malloc_hook_get_total_bytes(void)
{
	return (g_total_bytes);
}

void	init_canary(char *buf, size_t data_size)
{
	memset(buf, 0, data_size);
	memset(buf + data_size, CANARY_BYTE, CANARY_SIZE);
}

int		check_canary(const char *buf, size_t data_size)
{
	const unsigned char *canary = (const unsigned char *)(buf + data_size);
	for (size_t i = 0; i < CANARY_SIZE; i++)
	{
		if (canary[i] != CANARY_BYTE)
			return (0); /* Canary corrupted! */
	}
	return (1); /* Intact */
}
