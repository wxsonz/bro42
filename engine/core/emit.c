#include "bro.h"
#include <stdio.h>
#include <string.h>

/*
** One NDJSON line per case. The engine emits FACTS only - no why, no fix, no
** concept tags, no colour. Python joins the prose by fn:id at render time
** (design/02_ENGINE.md), so rewording an explanation rebuilds nothing.
**
** NDJSON rather than one document so results can render as they arrive, and so
** a truncated stream still yields every completed case.
*/

static void	json_str(const char *s)
{
	putchar('"');
	while (s && *s)
	{
		if (*s == '"' || *s == '\\')
			printf("\\%c", *s);
		else if ((unsigned char)*s < 0x20)
			printf("\\u%04x", (unsigned char)*s);
		else
			putchar(*s);
		s++;
	}
	putchar('"');
}

/*
** Same escaping as json_str, but length-bounded rather than NUL-terminated:
** r->captured is a student's raw buffer (test_ft_putnbr_fd.c and friends fill
** it via capture.c), not a C string - it may hold embedded NULs and is not
** guaranteed to be valid UTF-8. json_str would stop at the first NUL and
** silently truncate the evidence, so control bytes are escaped the same way
** but the walk is bounded by `n` instead of a terminator.
*/
static void	json_str_n(const char *s, size_t n)
{
	size_t	i;

	putchar('"');
	i = 0;
	while (i < n)
	{
		if (s[i] == '"' || s[i] == '\\')
			printf("\\%c", s[i]);
		else if ((unsigned char)s[i] < 0x20)
			printf("\\u%04x", (unsigned char)s[i]);
		else
			putchar(s[i]);
		i++;
	}
	putchar('"');
}

static void	json_bytes(const char *key, const unsigned char *b, size_t n)
{
	size_t	i;

	printf(",\"%s\":[", key);
	i = 0;
	while (i < n)
	{
		printf("%s%u", i ? "," : "", b[i]);
		i++;
	}
	printf("]");
}

static void	emit_evidence(const t_result *r)
{
	if (r->kind == EV_SCALAR || r->kind == EV_OFFSET)
		printf(",\"expected\":%lld,\"actual\":%lld",
			r->expected_num, r->actual_num);
	if (r->kind == EV_BYTES)
	{
		json_bytes("expected_bytes", r->expected_b, r->nbytes);
		json_bytes("actual_bytes", r->actual_b, r->nbytes);
		printf(",\"window_base\":%zu,\"diverge\":%ld",
			r->window_base, r->diverge);
	}
	if (r->write_extent >= 0)
		printf(",\"write_extent\":%ld", r->write_extent);
}

static void	emit_alloc(const t_result *r)
{
	size_t	i;

	if (r->alloc.calls == 0 && r->rollback_n == 0)
		return ;
	/* When a sweep ran, its width is the true number of allocations the
	** function under test made. r->alloc is the SNAPSHOT OF THE FAILING RUN,
	** which stopped early, so recomputing from it under-reports. */
	printf(",\"alloc\":{\"calls\":%zu,\"under_test\":%zu,"
		"\"live_bytes\":%zu,\"live_blocks\":%zu,\"fail_after\":%ld}",
		r->alloc.calls,
		r->rollback_n ? r->rollback_n : r->alloc.calls - r->alloc.base,
		r->alloc.live_bytes, r->alloc.live_blocks, r->fail_after);
	if (r->rollback_n == 0)
		return ;
	printf(",\"rollback\":[");
	i = 0;
	while (i < r->rollback_n)
	{
		printf("%s%s", i ? "," : "", r->rollback[i] ? "true" : "false");
		i++;
	}
	printf("]");
}

static void	emit_sweep(const t_result *r)
{
	size_t	i;

	if (r->sweep_n == 0)
		return ;
	printf(",\"sweep\":{\"scored\":%s,\"values\":[",
		r->sweep_scored ? "true" : "false");
	i = 0;
	while (i < r->sweep_n)
	{
		printf("%s%ld", i ? "," : "", r->sweep_val[i]);
		i++;
	}
	printf("],\"got\":[");
	i = 0;
	while (i < r->sweep_n)
	{
		printf("%s%s", i ? "," : "", r->sweep[i] ? "true" : "false");
		i++;
	}
	printf("],\"want\":[");
	i = 0;
	while (i < r->sweep_n)
	{
		printf("%s%s", i ? "," : "", r->sweep_ref[i] ? "true" : "false");
		i++;
	}
	printf("]}");
}

/*
** t_result carries whatever a case captured off a file descriptor
** (capture.c), but nothing emitted it - test_ft_putnbr_fd.c fills
** r->captured/captured_len and the report had no way to show it.
*/
static void	emit_captured(const t_result *r)
{
	if (r->captured_len == 0)
		return ;
	printf(",\"captured\":");
	json_str_n(r->captured, r->captured_len);
}

static const char	*kind_name(t_evidence k)
{
	static const char	*names[] = {
		"none", "scalar", "offset", "bytes", "guard"
	};

	return (names[k]);
}

void	bro_emit(const t_suite *s, const t_case *c, const t_result *r,
		double ms)
{
	printf("{\"fn\":");
	json_str(s->fn);
	printf(",\"id\":%d,\"part\":%d,\"level\":%d,\"tier\":%d",
		c->id, s->part, s->level, s->tier);
	printf(",\"status\":");
	json_str(bro_status_name(r->status));
	printf(",\"kind\":");
	json_str(kind_name(r->kind));
	printf(",\"ms\":%.3f", ms);
	printf(",\"msg\":");
	json_str(r->msg);
	emit_evidence(r);
	emit_alloc(r);
	emit_sweep(r);
	emit_captured(r);
	printf("}\n");
	fflush(stdout);
}
