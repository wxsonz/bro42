#ifndef BRO_H
# define BRO_H

/*
** ft_bro micro engine - core interfaces.
**
** STATUS: stage B. All six tiers now use t_ctx, so it is frozen: T0 and T1 in
** the alpha, then T2 (allocation), T3 (structure of allocations), T4 (fd
** capture) and T5 (caller-owned lists). Adding a field after this point means
** a tier was missed, not that a case needs a convenience.
**
** Design references:
**   design/04_TESTDESIGN.md  the case record, statuses, tiers
**   design/02_ENGINE.md      isolation, guards, oracle, allocation
*/

# include <stddef.h>
# include <stdbool.h>
# include <stdint.h>

/*
** ---------------------------------------------------------------- statuses
** UB, MISSING and SKIP are never scored - see design/04_TESTDESIGN.md.
*/
typedef enum e_status
{
	BRO_OK = 0,
	BRO_KO,
	BRO_SIGSEGV,
	BRO_SIGBUS,
	BRO_SIGABRT,
	BRO_TIMEOUT,
	BRO_LEAK,
	BRO_UB,
	BRO_MISSING,
	BRO_SKIP
}	t_status;

const char	*bro_status_name(t_status s);
bool		bro_status_is_scored(t_status s);

/*
** ------------------------------------------------------------------- flags
** Mirror the `flags:` line in _dev/SPEC_MICRO.md.
*/
# define BRO_ORACLE		(1u << 0)
# define BRO_UB_CASE		(1u << 1)
# define BRO_INJECT		(1u << 2)
# define BRO_GUARDED		(1u << 3)
# define BRO_CAPTURES_FD	(1u << 4)

/*
** ------------------------------------------------------------------ buffers
** A guarded buffer. `mem` is the whole allocation; `ptr` is what the student's
** function is handed. Canaries sit on both sides of `ptr` and are verified
** after the call, which is the only way to see a write past the end when the
** return value is still correct (design/02_ENGINE.md).
**
** Guarded buffers are ENGINE-owned: allocated with the real allocator, never
** counted as student allocation.
*/
# define BRO_CANARY		0x5A
# define BRO_POISON		0xAA
# define BRO_CANARY_LEN		16

typedef struct s_buf
{
	unsigned char	*mem;
	unsigned char	*ptr;
	size_t		len;
	bool		guarded;
}	t_buf;

/*
** ---------------------------------------------------------------- evidence
** What the renderer draws. The engine reports bytes and offsets; it never
** formats, colours or explains - prose is joined in Python by fn:id.
*/
# define BRO_EVID_MAX		64
# define BRO_MSG_MAX		256
# define BRO_INJECT_MAX		128
# define BRO_CAPTURE_MAX	512

typedef enum e_evidence
{
	EV_NONE = 0,
	EV_SCALAR,	/* expected/actual as integers  */
	EV_OFFSET,	/* pointer results, as offsets into the input */
	EV_BYTES,	/* two byte windows plus a divergence index */
	EV_GUARD	/* a canary was overwritten */
}	t_evidence;

/*
** Allocation accounting for one case (stage B, alloc.c).
**
** `owned` is memory the STUDENT is expected to free - list fixtures handed to
** ft_lstclear, for instance - and is deliberately allocated through the
** wrapped allocator so that failing to free it shows up as a leak. Engine
** scratch (guards, oracle reference buffers) goes through __real_malloc and
** never appears here at all.
*/
typedef struct s_alloc
{
	bool		armed;
	long		fail_after;	/* -1 = never fail                */
	size_t		calls;		/* wrapped malloc calls this case */
	size_t		base;		/* calls made before bro_ready()  */
	size_t		live_blocks;
	size_t		live_bytes;
	size_t		peak_bytes;
}	t_alloc;

typedef struct s_result
{
	t_status	status;
	t_evidence	kind;
	long long	expected_num;
	long long	actual_num;
	unsigned char	expected_b[BRO_EVID_MAX];
	unsigned char	actual_b[BRO_EVID_MAX];
	size_t		nbytes;		/* bytes valid in the two windows       */
	size_t		window_base;	/* index of byte 0 of the window        */
	long		diverge;	/* first differing index, -1 if none    */
	long		write_extent;	/* bytes actually written, -1 if unknown */
	char		msg[BRO_MSG_MAX];
	t_alloc		alloc;
	long		fail_after;	/* the k this run used, -1 if none  */
	unsigned char	rollback[BRO_INJECT_MAX];	/* 1 = clean at k */
	size_t		rollback_n;
	char		captured[BRO_CAPTURE_MAX];
	size_t		captured_len;
}	t_result;

void	bro_fail(t_result *r, const char *fmt, ...);
void	bro_expect_num(t_result *r, long long expected, long long actual);
void	bro_expect_sign(t_result *r, long long expected, long long actual);
void	bro_expect_bytes(t_result *r, const void *expected, const void *actual,
			size_t n);
void	bro_expect_offset(t_result *r, const void *base, const void *expected,
			const void *actual);
void	bro_mark_ub(t_result *r, const char *fmt, ...);

/*
** --------------------------------------------------------------- t_ctx (A1)
** Handed to every case function. It owns the scratch a case needs so that no
** case allocates for itself - which keeps student allocation counts clean once
** alloc.c lands in stage B.
**
**   dst  destination buffer, guarded, for T1 cases that write
**   ref  the ORACLE's own destination - never the same memory as dst, or the
**        oracle and the student would write to one buffer and every T1 case
**        would pass (design/02_ENGINE.md, gap 4)
**   src  read-only input scratch
**
** Stage B will add: allocation counters and the injection index (T2),
** captured-fd handles (T4), and list fixture ownership tracking (T5).
*/
typedef struct s_ctx
{
	t_buf		dst;
	t_buf		ref;
	t_buf		src;
	t_result	*out;
	unsigned	flags;
	void		*returned;	/* what the function under test gave back */
	int		cap_fd[2];	/* captured descriptors, -1 when unused   */
	int		cap_saved[2];
}	t_ctx;

/*
** Cases that allocate hand their result here. Under injection the harness
** requires it to be NULL: returning a partly-built structure after a failed
** malloc is a bug the leak count alone would not catch.
*/
void	bro_track(t_ctx *c, void *p);
bool	bro_injecting(const t_ctx *c);

/*
** Called after a case has built its fixtures and immediately before it invokes
** the function under test. Everything allocated before this point is still
** counted for leaks but can never be the injected failure - otherwise the
** sweep spends its budget breaking the test's own setup and reports the
** fixture's memory as the student's leak.
**
** This is enforced by the harness sweeping ABSOLUTE allocation indices from
** base upward, not by anything this function does at allocation time. An
** earlier version subtracted base inside the allocator, which did not work:
** base is only known once the fixture is already built, so at the moment a
** fixture allocated it was still zero and a low failure index broke the
** setup anyway.
**
** Cases with no setup need not call it; the baseline is then zero.
*/
void	bro_ready(t_ctx *c);

t_buf	*bro_ctx_dst(t_ctx *c, size_t len);
t_buf	*bro_ctx_ref(t_ctx *c, size_t len);
void	*bro_ctx_src(t_ctx *c, const void *data, size_t len);
void	bro_ctx_reset(t_ctx *c);

/*
** ------------------------------------------------------------ case & suite
** `id` is a PERMANENT address: bro debug, web deep links, run history and the
** self-test all use it. Append only; never renumber (design/04 B7).
*/
typedef struct s_case
{
	int		id;
	const char	*input;
	unsigned	flags;
	void		(*run)(t_ctx *);
}	t_case;

typedef struct s_suite
{
	const char	*fn;
	int		part;
	int		level;
	int		tier;
	const t_case	*cases;
	size_t		count;
}	t_suite;

extern const t_suite	*g_suites[];
extern const size_t	g_suite_count;

/*
** ----------------------------------------------------------------- harness
*/
typedef struct s_limits
{
	unsigned	timeout_ms;
	size_t		address_space;
	bool		fork;
}	t_limits;

int	bro_run_case(const t_suite *s, const t_case *c, const t_limits *lim,
		t_result *out);

/* guard.c */
bool	bro_guard_alloc(t_buf *b, size_t len);
void	bro_guard_free(t_buf *b);
bool	bro_guard_check(const t_buf *b, t_result *r);
long	bro_guard_extent(const t_buf *b);

/* alloc.c - allocation accounting and failure injection */
void	bro_alloc_reset(void);
void	bro_alloc_arm(long fail_after);
void	bro_alloc_ready(void);
void	bro_alloc_disarm(void);
void	bro_alloc_snapshot(t_alloc *out);
bool	bro_alloc_available(void);

/* capture.c - fd capture for functions whose only observable is what they wrote */
bool	bro_capture_bind(t_ctx *c, int slot, int fd);
size_t	bro_capture_take(t_ctx *c, int slot, char *out, size_t max);

/* list.c - t_list fixtures, cycle-safe traversal, ownership tracking */
typedef struct s_list
{
	void		*content;
	struct s_list	*next;
}	t_list;

# define BRO_LIST_MAX	4096

t_list	*bro_list_build(size_t n);
void	bro_list_free(t_list *lst);
long	bro_list_len(const t_list *lst);
void	bro_del_counting(void *content);
size_t	bro_del_calls(void);
void	bro_del_reset(void);

/* oracle.c - reference implementations libc does not provide */
char	*bro_ref_strnstr(const char *big, const char *little, size_t len);

/* emit.c */
void	bro_emit(const t_suite *s, const t_case *c, const t_result *r,
		double ms);

/* utils */
void	*bro_alloc(size_t n);
void	bro_free(void *p);

#endif
