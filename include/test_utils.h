#ifndef TEST_UTILS_H
# define TEST_UTILS_H

# include <stddef.h>
# include <unistd.h>
# include <stdlib.h>
# include <string.h>

/* Canary buffer protection */
# define CANARY_SIZE 16
# define CANARY_BYTE 0xDE

void	init_canary(char *buf, size_t data_size);
int		check_canary(const char *buf, size_t data_size);

/* Pipe redirection */
int		capture_fd_start(int target_fd);
char	*capture_fd_end(int target_fd, int pipe_read_fd, size_t *out_size);

/* Malloc failure injection & tracking */
void	malloc_hook_enable(void);
void	malloc_hook_disable(void);
void	malloc_hook_set_fail_at(int count);
void	malloc_hook_reset(void);
size_t	malloc_hook_get_active_allocs(void);
size_t	malloc_hook_get_total_bytes(void);

#endif
