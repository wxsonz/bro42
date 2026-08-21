#include "tester.h"

static int g_saved_fd = -1;

int	capture_fd_start(int target_fd)
{
	int pipe_fds[2];

	if (pipe(pipe_fds) == -1)
		return (-1);

	g_saved_fd = dup(target_fd);
	if (g_saved_fd == -1)
	{
		close(pipe_fds[0]);
		close(pipe_fds[1]);
		return (-1);
	}

	if (dup2(pipe_fds[1], target_fd) == -1)
	{
		close(pipe_fds[0]);
		close(pipe_fds[1]);
		close(g_saved_fd);
		return (-1);
	}

	close(pipe_fds[1]); /* Writing end is now duplicated to target_fd */
	return (pipe_fds[0]); /* Return read end */
}

char	*capture_fd_end(int target_fd, int pipe_read_fd, size_t *out_size)
{
	char	buffer[4096];
	char	*result = NULL;
	size_t	total_read = 0;
	ssize_t	bytes_read;

	/* Flush target fd */
	if (target_fd == 1)
		fflush(stdout);
	else if (target_fd == 2)
		fflush(stderr);

	/* Restore original file descriptor */
	if (g_saved_fd != -1)
	{
		dup2(g_saved_fd, target_fd);
		close(g_saved_fd);
		g_saved_fd = -1;
	}

	/* Set read end to non-blocking to prevent hang */
	int flags = fcntl(pipe_read_fd, F_GETFL, 0);
	fcntl(pipe_read_fd, F_SETFL, flags | O_NONBLOCK);

	while ((bytes_read = read(pipe_read_fd, buffer, sizeof(buffer) - 1)) > 0)
	{
		char *new_res = realloc(result, total_read + bytes_read + 1);
		if (!new_res)
		{
			free(result);
			close(pipe_read_fd);
			return (NULL);
		}
		result = new_res;
		memcpy(result + total_read, buffer, bytes_read);
		total_read += bytes_read;
		result[total_read] = '\0';
	}

	close(pipe_read_fd);

	if (!result)
	{
		result = strdup("");
		total_read = 0;
	}

	if (out_size)
		*out_size = total_read;

	return (result);
}

t_list	*test_create_node(void *content)
{
	t_list	*node;

	node = (t_list *)malloc(sizeof(t_list));
	if (!node)
		return (NULL);
	node->content = content;
	node->next = NULL;
	return (node);
}

void	test_free_list(t_list *lst)
{
	t_list	*tmp;

	while (lst)
	{
		tmp = lst->next;
		free(lst);
		lst = tmp;
	}
}

void	test_free_list_with_del(t_list *lst, void (*del)(void *))
{
	t_list	*tmp;

	while (lst)
	{
		tmp = lst->next;
		if (del && lst->content)
			del(lst->content);
		free(lst);
		lst = tmp;
	}
}

