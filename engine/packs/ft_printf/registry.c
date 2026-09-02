#include "bro.h"

/*
** The suites, statically listed - same discipline as engine/packs/libft/
** registry.c: no dynamic registration, adding a suite is adding a line
** (design/02_ENGINE.md non-goals).
**
** Six of fifteen suites so far (_dev/plan/rank01/ftprintf-01-cases.md
** Part 1 #1-6: pf_literal, pf_pct, pf_c, pf_s, pf_d, pf_i). To add the next
** one (#7 pf_u):
**   1. write engine/packs/ft_printf/tests/mandatory/test_pf_u.c, following
**      one of the six files here as a template - every suite in this pack is
**      Tier T4, so pf_begin/pf_check (printf_assert.h) apply unchanged;
**   2. `extern const t_suite g_suite_pf_u;` below, in spec order;
**   3. add `&g_suite_pf_u,` to g_suites[] below, in the same order;
**   4. add `mandatory/test_pf_u` to FT_PRINTF_TESTS in the root Makefile.
** Case ids must match bro42/packs/ft_printf/data/cases.json's `pf_u:N`
** exactly - that file is generated from the spec and is not this agent's to
** edit.
*/

extern const t_suite	g_suite_pf_literal;
extern const t_suite	g_suite_pf_pct;
extern const t_suite	g_suite_pf_c;
extern const t_suite	g_suite_pf_s;
extern const t_suite	g_suite_pf_d;
extern const t_suite	g_suite_pf_i;

const t_suite	*g_suites[] = {
	&g_suite_pf_literal,	/* T4, level 1 */
	&g_suite_pf_pct,	/* T4, level 1 */
	&g_suite_pf_c,		/* T4, level 2 */
	&g_suite_pf_s,		/* T4, level 2 */
	&g_suite_pf_d,		/* T4, level 3 */
	&g_suite_pf_i,		/* T4, level 3 */
};

const size_t	g_suite_count = sizeof(g_suites) / sizeof(*g_suites);

/*
** Nothing to reset between cases: an ft_printf case's entire state is the
** descriptor capture, which the harness sets up and tears down itself.
*/
void	bro_pack_before_case(void)
{
}
