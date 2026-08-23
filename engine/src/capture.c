#include "bro.h"
#include <string.h>
#include <unistd.h>

/*
** fd capture (T4). ft_putchar_fd and friends have no return value, so the only
** observable is what appeared on the descriptor - there is nothing to compare
** but the bytes themselves.
**
** Binding a real descriptor number rather than handing the case an arbitrary
** fd is deliberate: it is what lets a case assert "3 bytes on fd 2 and nothing
** on fd 1", which is how you catch a function that ignores its fd parameter
** and always writes to stdout.
*/

bool	bro_capture_bind(t_ctx *c, int slot, int fd)
{
	int	p[2];

	if (slot < 0 || slot > 1 || pipe(p) < 0)
		return (false);
	c->cap_saved[slot] = dup(fd);
	if (c->cap_saved[slot] < 0)
	{
		close(p[0]);
		close(p[1]);
		return (false);
	}
	if (dup2(p[1], fd) < 0)
	{
		close(p[0]);
		close(p[1]);
		return (false);
	}
	close(p[1]);
	c->cap_fd[slot] = p[0];
	return (true);
}

/*
** Restoring the descriptor closes the write end, which is what puts EOF on the
** read end. Without that the read below would block forever.
*/
size_t	bro_capture_take(t_ctx *c, int slot, char *out, size_t max)
{
	ssize_t	n;
	size_t	total;
	int	fd;

	if (slot < 0 || slot > 1 || c->cap_fd[slot] <= 0)
		return (0);
	fd = (slot == 0) ? STDOUT_FILENO : STDERR_FILENO;
	if (c->cap_saved[slot] >= 0)
	{
		dup2(c->cap_saved[slot], fd);
		close(c->cap_saved[slot]);
		c->cap_saved[slot] = -1;
	}
	total = 0;
	while (total < max)
	{
		n = read(c->cap_fd[slot], out + total, max - total);
		if (n <= 0)
			break ;
		total += (size_t)n;
	}
	close(c->cap_fd[slot]);
	c->cap_fd[slot] = -1;
	return (total);
}
