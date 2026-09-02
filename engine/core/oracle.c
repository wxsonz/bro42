#include "bro.h"
#include <string.h>

/*
** Reference implementations for the functions libc does not provide here
** (design/02_ENGINE.md, decision B4).
**
** Expectations are normally computed by calling real libc at runtime, so the
** tester is always right about the platform it is on. strnstr is BSD-only and
** glibc will never have it, so we ship one - and that makes it the one oracle
** that can be WRONG. If it is, correct students fail, which is the worst
** failure this project can have. It follows the documented BSD semantics
** exactly, including the two that surprise people:
**
**   - an empty needle returns big, and does so BEFORE len is consulted,
**     so it matches even when len is 0;
**   - the whole needle must fit within the first len characters, so a match
**     that starts inside the window but ends outside it does not count.
**
** strlcpy and strlcat get the same treatment: glibc only gained them in
** 2.38, and pinning the oracle to real libc would make the build's outcome
** depend on which glibc happens to be installed. Shipping our own keeps the
** tester right everywhere, on the same terms as strnstr above.
*/

char	*bro_ref_strnstr(const char *big, const char *little, size_t len)
{
	char	c;
	char	sc;
	size_t	needle_len;

	c = *little++;
	if (c == '\0')
		return ((char *)big);
	needle_len = strlen(little);
	while (1)
	{
		sc = 0;
		while (sc != c)
		{
			if (len-- < 1)
				return (NULL);
			sc = *big++;
			if (sc == '\0')
				return (NULL);
		}
		if (needle_len > len)
			return (NULL);
		if (strncmp(big, little, needle_len) == 0)
			break ;
	}
	return ((char *)(big - 1));
}

size_t	bro_ref_strlcpy(char *dst, const char *src, size_t size)
{
	size_t	srclen;
	size_t	n;

	srclen = strlen(src);
	if (size != 0)
	{
		n = srclen;
		if (n >= size)
			n = size - 1;
		if (n > 0)
			memcpy(dst, src, n);
		dst[n] = '\0';
	}
	return (srclen);
}

size_t	bro_ref_strlcat(char *dst, const char *src, size_t size)
{
	char	*d;
	const char	*s;
	size_t	n;
	size_t	dlen;

	d = dst;
	s = src;
	n = size;
	while (n-- != 0 && *d != '\0')
		d++;
	dlen = (size_t)(d - dst);
	n = size - dlen;
	if (n == 0)
		return (dlen + strlen(s));
	while (*s != '\0')
	{
		if (n != 1)
		{
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';
	return (dlen + (size_t)(s - src));
}
