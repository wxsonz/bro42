#ifndef BRO_H
# define BRO_H

/*
** bro42 micro engine - core interfaces.
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
** Mirror the `flags:` line in _dev/plan/rank00/libft-01-cases.md.
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
/* 4096, not 512: ft_printf cases capture far more than a Libft fd-write
** does. This pushes sizeof(t_result) past PIPE_BUF, so the child's result
** record is no longer written to the pipe as a single atomic write - safe
** here only because harness.c's read_retry/write_retry already loop over
** short reads/writes and this is a dedicated single-writer pipe (one
** child, one record, nothing else can interleave on it). */
# define BRO_CAPTURE_MAX	4096
/* A sweep records, per tested integer, what the student's function did AND
** what the oracle did. 160 covers the 128 ASCII slots plus the short list of
** notable integers shown beside them - deliberately small: the grid is a
** thing to read, and 768 blocks is not. */
# define BRO_SWEEP_MAX		160

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
	/* Per-value sweep results. For each tested integer sweep_val[i]:
	**   sweep[i]      what the STUDENT's function did - "returned true"
	**                 for a classifier, "converted this byte" for a mapper
	**   sweep_ref[i]  what the ORACLE did, same meaning
	** The page colours by sweep[] so the character class stays legible even
	** when every value agrees, and marks where the two differ. Emitting
	** both is what lets one grid teach and check at the same time. */
	unsigned char	sweep[BRO_SWEEP_MAX];
	unsigned char	sweep_ref[BRO_SWEEP_MAX];
	long		sweep_val[BRO_SWEEP_MAX];
	size_t		sweep_n;
	int		sweep_scored;
}	t_result;

void	bro_fail(t_result *r, const char *fmt, ...);
void	bro_expect_num(t_result *r, long long expected, long long actual);
void	bro_expect_sign(t_result *r, long long expected, long long actual);
void	bro_expect_bytes(t_result *r, const void *expected, const void *actual,
			size_t n);
void	bro_expect_offset(t_result *r, const void *base, const void *expected,
			const void *actual);
/*
** Thin wrapper over bro_expect_bytes for NUL-terminated C strings: strlen
** both sides and compare as bytes, so a mismatch still draws EV_BYTES
** evidence with a divergence index instead of a bare pass/fail.
*/
void	bro_expect_str(t_result *r, const char *expected, const char *actual);
void	bro_mark_ub(t_result *r, const char *fmt, ...);

/*
** Sweep a classification function across [lo, hi] instead of poking a handful
** of hand-picked bytes. `fn` is the student's, `ref` the libc oracle.
**
** scored != 0: every value is compared against ref and must match exactly 1
** or 0 (Subject IV.2 pins the return values, not merely truthiness). Only
** valid where ref itself is defined - C leaves isalpha and friends undefined
** outside EOF and unsigned char, so a scored sweep must stay in [-1, 255].
**
** scored == 0: nothing is asserted. The sweep runs, records what came back,
** and the case is marked UB so it shows in the report without being scored -
** the shape of an answer nobody specified is worth SEEING, never grading.
*/
void	bro_sweep_class(t_result *r, int (*fn)(int), int (*ref)(int),
			long lo, long hi, int scored);
void	bro_sweep_map(t_result *r, int (*fn)(int), int (*ref)(int),
			long lo, long hi, int scored);
void	bro_sweep_ints(t_result *r, int (*fn)(int), int (*ref)(int),
			const long *vals, size_t n, int scored);
void	bro_sweep_ints_map(t_result *r, int (*fn)(int), int (*ref)(int),
			const long *vals, size_t n, int scored);

/*
** Sibling of bro_sweep_class for functions that return a transformed VALUE
** rather than a boolean classification (ft_toupper, ft_tolower). Comparing
** fn(v) and ref(v) as booleans would be wrong here - 'a' and 'A' are both
** non-zero, so a toupper that returned the input untouched would look
** "correct" to bro_sweep_class on every letter it should have changed.
** bro_sweep_map compares the returned values themselves instead.
**
** scored != 0: every value must produce exactly ref(v). Same domain rule as
** bro_sweep_class - only valid where ref itself is defined, [-1, 255].
**
** scored == 0: nothing is asserted. The bit recorded is whether the value
** came back UNCHANGED (fn(v) == v) - the "pass-through" case these functions
** promise for anything they do not convert, so the sweep's shape shows
** exactly where an unbounded implementation starts transforming bytes it
** was never told to.
*/
void	bro_sweep_map(t_result *r, int (*fn)(int), int (*ref)(int),
			long lo, long hi, int scored);

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

/*
** t_list and its fixtures moved to engine/packs/libft/pack.h - a linked list
** is Libft's subject matter (Part 3), not the harness's. bro.h stays clean of
** any pack's types; see harness.c for the one place that still leaks a
** Libft-specific hook into the core (out of scope for this split to fix).
*/

/* oracle.c - reference implementations libc does not provide */
char	*bro_ref_strnstr(const char *big, const char *little, size_t len);

/* emit.c */
void	bro_emit(const t_suite *s, const t_case *c, const t_result *r,
		double ms);

/* utils */
void	*bro_alloc(size_t n);
void	bro_free(void *p);

#endif
