#ifndef LIBC_ORACLE_H
# define LIBC_ORACLE_H

# include <stddef.h>
# include <string.h>
# include <ctype.h>
# include <stdlib.h>
# include <stdint.h>

/* BSD Reference implementation of strlcpy for differential testing */
static inline size_t oracle_strlcpy(char *dst, const char *src, size_t size)
{
	size_t src_len = 0;
	while (src[src_len])
		src_len++;
	if (size > 0)
	{
		size_t i = 0;
		while (src[i] && i < size - 1)
		{
			dst[i] = src[i];
			i++;
		}
		dst[i] = '\0';
	}
	return (src_len);
}

/* BSD Reference implementation of strlcat for differential testing */
static inline size_t oracle_strlcat(char *dst, const char *src, size_t size)
{
	size_t dst_len = 0;
	size_t src_len = 0;
	size_t i = 0;

	while (dst[dst_len] && dst_len < size)
		dst_len++;
	while (src[src_len])
		src_len++;

	if (dst_len == size)
		return (size + src_len);

	while (src[i] && (dst_len + i + 1) < size)
	{
		dst[dst_len + i] = src[i];
		i++;
	}
	dst[dst_len + i] = '\0';
	return (dst_len + src_len);
}

/* BSD Reference implementation of strnstr for differential testing */
static inline char *oracle_strnstr(const char *big, const char *little, size_t len)
{
	size_t i = 0;
	size_t j = 0;

	if (!little[0])
		return ((char *)big);
	while (big[i] && i < len)
	{
		j = 0;
		while (big[i + j] && little[j] && (i + j) < len && big[i + j] == little[j])
			j++;
		if (!little[j])
			return ((char *)&big[i]);
		i++;
	}
	return (NULL);
}

#endif
