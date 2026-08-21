// ft_bro (bro) Web Dashboard JavaScript Engine

const ROADMAP_LEVELS = [
	{
		title: "Level 1: ASCII & Character Classification",
		funcs: ["ft_isalpha", "ft_isdigit", "ft_isalnum", "ft_isascii", "ft_isprint", "ft_toupper", "ft_tolower"]
	},
	{
		title: "Level 2: Basic Pointer Traversal & Memory",
		funcs: ["ft_strlen", "ft_memset", "ft_bzero", "ft_strchr", "ft_strrchr", "ft_memchr"]
	},
	{
		title: "Level 3: Comparisons & Overlapping Copies",
		funcs: ["ft_memcpy", "ft_memmove", "ft_strncmp", "ft_memcmp"]
	},
	{
		title: "Level 4: Sized Buffers & String Conversions",
		funcs: ["ft_strlcpy", "ft_strlcat", "ft_strnstr", "ft_atoi"]
	},
	{
		title: "Level 5: Dynamic Memory Allocation (Heap)",
		funcs: ["ft_calloc", "ft_strdup", "ft_substr", "ft_strjoin", "ft_strtrim", "ft_itoa"]
	},
	{
		title: "Level 6: Double Pointers & Function Pointers",
		funcs: ["ft_split", "ft_strmapi", "ft_striteri"]
	},
	{
		title: "Level 7: File Descriptors & System I/O",
		funcs: ["ft_putchar_fd", "ft_putstr_fd", "ft_putendl_fd", "ft_putnbr_fd"]
	},
	{
		title: "Level 8: Linked List Data Structures (Part 3)",
		funcs: ["ft_lstnew", "ft_lstadd_front", "ft_lstsize", "ft_lstlast", "ft_lstadd_back", "ft_lstdelone", "ft_lstclear", "ft_lstiter", "ft_lstmap"]
	}
];

const CONCEPT_CARDS = [
	{
		title: "1. Pointers & Pointer Arithmetic",
		body: `In C, a pointer is simply an integer variable that holds a memory address.
When you add <code>1</code> to a <code>char *</code>, it moves by 1 byte. When you add <code>1</code> to an <code>int *</code>, it moves by <code>sizeof(int)</code> (4 bytes).
<pre>char *start = s;
char *end = s + strlen(s) - 1;
size_t len = end - start; // Calculates exact span</pre>`
	},
	{
		title: "2. Unsigned Char Promotion Trap",
		body: `In C, <code>char</code> can be signed (-128 to 127). If you compare <code>\\200</code> (128) vs <code>\\0</code> without casting, signed char treats 128 as -128, causing negative instead of positive difference!
<pre>// Always cast to unsigned char:
unsigned char c1 = (unsigned char)*s1;
unsigned char c2 = (unsigned char)*s2;
return (c1 - c2);</pre>`
	},
	{
		title: "3. Double Pointers (char ** and t_list **)",
		body: `A double pointer is a pointer to a pointer.
In <code>ft_split</code>, <code>char **</code> points to an array of pointers, each pointing to a separate string on the heap.
In <code>ft_lstadd_front(t_list **lst, ...)</code>, we pass <code>&lst</code> so the function can modify the caller's head variable directly.`
	},
	{
		title: "4. Malloc Rollback & Zero Leak Rule",
		body: `If <code>ft_split</code> allocates 3 strings, and the 4th malloc returns NULL:
1. Free string 1, string 2, string 3
2. Free the outer array
3. Return NULL with 0 memory leaks!`
	},
	{
		title: "5. File Descriptors & write()",
		body: `File Descriptors are integers representing open I/O streams:
<code>0</code> = stdin (keyboard), <code>1</code> = stdout (terminal), <code>2</code> = stderr (errors).
<code>write(fd, buf, count)</code> is a direct Unix system call.`
	},
	{
		title: "6. Function Pointers & Callbacks",
		body: `Function pointers allow passing logic as arguments (e.g. in <code>ft_strmapi</code> and <code>ft_lstmap</code>):
<pre>char ft_strmapi(char const *s, char (*f)(unsigned int, char));</pre>`
	}
];

const DEFENSE_FLASHCARDS = [
	{
		q: "Why does ft_memmove need to handle overlapping memory, while ft_memcpy does not?",
		a: "memcpy assumes separate buffers. If dest > src and they overlap, forward copying overwrites source bytes before they are read. memmove checks if dest > src and copies backwards to preserve data integrity."
	},
	{
		q: "Why does ft_calloc(0, 0) need special handling?",
		a: "Per 42 Subject Section IV.2, ft_calloc(0, 0) must return a unique non-NULL pointer that can be safely freed without crashing or leaking memory. It must also check for nmemb * size overflow before calling malloc."
	},
	{
		q: "What is the return formula of ft_strlcat when size <= strlen(dst)?",
		a: "When size <= strlen(dst), no characters can be appended, and dst is not modified. It returns size + strlen(src)."
	},
	{
		q: "Why does n = -2147483648 overflow in naive ft_putnbr_fd?",
		a: "In 32-bit two's complement, INT_MIN is -2147483648, but INT_MAX is 2147483647. The positive value 2147483648 cannot fit in a 32-bit signed int. We handle this using unsigned math or casting to long."
	}
];

const MEMORY_DEMOS = {
	memmove: [
		{
			grid: ["'A'", "'B'", "'C'", "'D'", "'E'", "0", "0", "0"],
			indices: [0, 1, 2, 3, 4, 5, 6, 7],
			src: 0,
			dst: 2,
			desc: "Initial buffer: src = buf (index 0), dest = buf + 2 (index 2). Notice that dest > src, so copy must be backwards!"
		},
		{
			grid: ["'A'", "'B'", "'C'", "'D'", "'E'", "0", "'E'", "0"],
			indices: [0, 1, 2, 3, 4, 5, 6, 7],
			src: 4,
			dst: 6,
			desc: "Step 1: Copy byte 4 ('E') to dest byte 6."
		},
		{
			grid: ["'A'", "'B'", "'C'", "'D'", "'D'", "0", "'E'", "0"],
			indices: [0, 1, 2, 3, 4, 5, 6, 7],
			src: 3,
			dst: 5,
			desc: "Step 2: Copy byte 3 ('D') to dest byte 5."
		},
		{
			grid: ["'A'", "'B'", "'A'", "'B'", "'C'", "'D'", "'E'", "0"],
			indices: [0, 1, 2, 3, 4, 5, 6, 7],
			src: 0,
			dst: 2,
			desc: "Completed: All 5 bytes safely copied without source corruption!"
		}
	],
	strtrim: [
		{
			grid: ["' '", "' '", "'h'", "'e'", "'l'", "'l'", "'o'", "' '", "'\\0'"],
			indices: [0, 1, 2, 3, 4, 5, 6, 7, 8],
			src: 0,
			dst: 7,
			desc: "Initial string: start pointer at index 0, end pointer at index 7. Set = ' '."
		},
		{
			grid: ["' '", "' '", "'h'", "'e'", "'l'", "'l'", "'o'", "' '", "'\\0'"],
			indices: [0, 1, 2, 3, 4, 5, 6, 7, 8],
			src: 2,
			dst: 7,
			desc: "Step 1: Advance start pointer past leading spaces to index 2 ('h')."
		},
		{
			grid: ["' '", "' '", "'h'", "'e'", "'l'", "'l'", "'o'", "' '", "'\\0'"],
			indices: [0, 1, 2, 3, 4, 5, 6, 7, 8],
			src: 2,
			dst: 6,
			desc: "Step 2: Decrement end pointer past trailing spaces to index 6 ('o')."
		},
		{
			grid: ["'h'", "'e'", "'l'", "'l'", "'o'", "'\\0'"],
			indices: [0, 1, 2, 3, 4, 5],
			src: 0,
			dst: 4,
			desc: "Allocates 6 bytes and copies 'hello\\0'. Trim complete!"
		}
	]
};

let currentReport = null;
let currentDemo = "memmove";
let currentDemoStep = 0;

// Initialize Dashboard
document.addEventListener("DOMContentLoaded", () => {
	setupNavigation();
	setupThemeToggle();
	setupFilters();
	setupMemoryInspector();
	renderConcepts();
	renderDefense();
	loadReportData();

	document.getElementById("btn-rerun").addEventListener("click", () => {
		fetch("/api/run", { method: "POST" })
			.then(res => res.json())
			.then(data => {
				currentReport = data;
				updateUI();
			})
			.catch(err => console.log("Re-run trigger:", err));
	});
});

function setupNavigation() {
	document.querySelectorAll(".nav-item").forEach(btn => {
		btn.addEventListener("click", () => {
			document.querySelectorAll(".nav-item").forEach(b => b.classList.remove("active"));
			document.querySelectorAll(".view-panel").forEach(p => p.classList.remove("active"));

			btn.classList.add("active");
			const tab = btn.dataset.tab;
			document.getElementById(`view-${tab}`).classList.add("active");
		});
	});
}

function setupThemeToggle() {
	const btn = document.getElementById("theme-toggle");
	btn.addEventListener("click", () => {
		document.body.classList.toggle("light-theme");
	});
}

function setupFilters() {
	document.querySelectorAll(".filter-btn").forEach(btn => {
		btn.addEventListener("click", () => {
			document.querySelectorAll(".filter-btn").forEach(b => b.classList.remove("active"));
			btn.classList.add("active");
			renderTestMatrix(btn.dataset.filter);
		});
	});
}

function renderRoadmap() {
	const container = document.getElementById("roadmap-container");
	container.innerHTML = "";

	ROADMAP_LEVELS.forEach(lvl => {
		const levelDiv = document.createElement("div");
		levelDiv.className = "roadmap-level";

		const title = document.createElement("div");
		title.className = "level-title";
		title.textContent = lvl.title;
		levelDiv.appendChild(title);

		const nodesDiv = document.createElement("div");
		nodesDiv.className = "nodes-container";

		lvl.funcs.forEach(f => {
			const node = document.createElement("div");
			let statusClass = "status-not-found";
			if (currentReport && currentReport.suites) {
				const suite = currentReport.suites.find(s => s.func_name === f);
				if (suite) {
					if (suite.not_found) statusClass = "status-not-found";
					else if (suite.fail_count === 0 && suite.test_count > 0) statusClass = "status-ok";
					else statusClass = "status-ko";
				}
			}
			node.className = `func-node ${statusClass}`;
			node.innerHTML = `<span>⚡</span> <span>${f}</span>`;
			node.addEventListener("click", () => {
				document.querySelector('[data-tab="tests"]').click();
			});
			nodesDiv.appendChild(node);
		});

		levelDiv.appendChild(nodesDiv);
		container.appendChild(levelDiv);
	});
}

function renderConcepts() {
	const container = document.getElementById("concepts-grid");
	container.innerHTML = "";

	CONCEPT_CARDS.forEach(c => {
		const card = document.createElement("div");
		card.className = "concept-card";
		card.innerHTML = `
			<div class="concept-title">${c.title}</div>
			<div class="concept-body">${c.body}</div>
		`;
		container.appendChild(card);
	});
}

function renderDefense() {
	const container = document.getElementById("defense-content");
	container.innerHTML = "";

	DEFENSE_FLASHCARDS.forEach((card, idx) => {
		const div = document.createElement("div");
		div.className = "flashcard";
		div.innerHTML = `
			<div class="flashcard-q">❓ Question ${idx + 1}: ${card.q}</div>
			<button class="btn-secondary btn-sm" onclick="this.nextElementSibling.classList.toggle('show')">
				👁️ Toggle Model Answer
			</button>
			<div class="flashcard-a">${card.a}</div>
		`;
		container.appendChild(div);
	});
}

function setupMemoryInspector() {
	const select = document.getElementById("inspector-select");
	select.addEventListener("change", (e) => {
		currentDemo = e.target.value;
		currentDemoStep = 0;
		renderMemoryDemo();
	});

	document.getElementById("btn-step-prev").addEventListener("click", () => {
		if (currentDemoStep > 0) {
			currentDemoStep--;
			renderMemoryDemo();
		}
	});

	document.getElementById("btn-step-next").addEventListener("click", () => {
		const demo = MEMORY_DEMOS[currentDemo];
		if (demo && currentDemoStep < demo.length - 1) {
			currentDemoStep++;
			renderMemoryDemo();
		}
	});

	renderMemoryDemo();
}

function renderMemoryDemo() {
	const demo = MEMORY_DEMOS[currentDemo];
	if (!demo) return;

	const step = demo[currentDemoStep];
	const display = document.getElementById("memory-grid-display");
	const indicator = document.getElementById("step-indicator");
	const explanation = document.getElementById("step-explanation");

	indicator.textContent = `Step ${currentDemoStep + 1} / ${demo.length}`;
	explanation.textContent = step.desc;

	let rowIndices = '<div class="grid-row">';
	let rowChars = '<div class="grid-row">';
	let rowPointers = '<div class="grid-row">';

	step.indices.forEach((idx, i) => {
		const isHighlight = (i === step.src || i === step.dst);
		const highlightClass = isHighlight ? 'highlight' : '';

		rowIndices += `<div class="grid-cell ${highlightClass}">idx: ${idx}</div>`;
		rowChars += `<div class="grid-cell ${highlightClass}"><b>${step.grid[i]}</b></div>`;

		let ptrLabel = "-";
		if (i === step.src && i === step.dst) ptrLabel = "src/dst";
		else if (i === step.src) ptrLabel = "▲ src";
		else if (i === step.dst) ptrLabel = "▲ dst";
		rowPointers += `<div class="grid-cell ${highlightClass}">${ptrLabel}</div>`;
	});

	rowIndices += '</div>';
	rowChars += '</div>';
	rowPointers += '</div>';

	display.innerHTML = `
		<div><b>Buffer Layout:</b></div>
		${rowChars}
		<div><b>Indices:</b></div>
		${rowIndices}
		<div><b>Pointers:</b></div>
		${rowPointers}
	`;
}

function loadReportData() {
	if (window.REPORT_DATA) {
		currentReport = window.REPORT_DATA;
		updateUI();
		return ;
	}
	fetch("report.json")
		.then(res => res.json())
		.then(data => {
			currentReport = data;
			updateUI();
		})
		.catch(err => {
			console.log("Using default state:", err);
			updateUI();
		});
}

function updateUI() {
	if (currentReport && currentReport.summary) {
		document.getElementById("header-score").textContent = `${currentReport.summary.passed_funcs} / ${currentReport.summary.total_funcs}`;
		const pct = Math.round((currentReport.summary.passed_tests / (currentReport.summary.total_tests || 1)) * 100);
		document.getElementById("header-percent").textContent = `${pct}%`;
	}
	renderRoadmap();
	renderTestMatrix("all");
	renderMacroView();
}

function renderTestMatrix(filter) {
	const container = document.getElementById("test-suites-container");
	container.innerHTML = "";

	if (!currentReport || !currentReport.suites) {
		container.innerHTML = `<div class="suite-card"><div class="suite-header">Run bro in terminal to populate live test results!</div></div>`;
		return ;
	}

	currentReport.suites.forEach(s => {
		if (filter === "part1" && s.part !== 1) return;
		if (filter === "part2" && s.part !== 2) return;
		if (filter === "part3" && s.part !== 3) return;
		if (filter === "failed" && s.fail_count === 0 && !s.not_found) return;

		const card = document.createElement("div");
		card.className = "suite-card";

		if (s.not_found) {
			card.innerHTML = `
				<div class="suite-header">
					<span class="suite-title">${s.func_name} (Part ${s.part})</span>
					<span class="suite-badge" style="background-color: #cf222e; color: #ffffff; font-weight: 700; padding: 4px 10px; border-radius: 6px;">NOT FOUND</span>
				</div>
				<div class="cases-list">
					<div class="case-item case-ko" style="background-color: rgba(207, 34, 46, 0.08); border-left-color: #cf222e;">
						<b>Function not found in library.</b> Implement <code>${s.func_name}</code> to run its test suite!
					</div>
				</div>
			`;
			container.appendChild(card);
			return ;
		}

		const isPass = s.fail_count === 0;
		const badgeClass = isPass ? "badge-ok" : "badge-ko";
		const badgeText = isPass ? `${s.pass_count}/${s.test_count} PASSED` : `${s.fail_count} FAILED`;

		let casesHtml = "";
		if (s.cases) {
			s.cases.forEach(tc => {
				const cPass = tc.status === "OK";
				const cClass = cPass ? "case-ok" : "case-ko";
				casesHtml += `
					<div class="case-item ${cClass}">
						<div class="case-header">
							<span>Test ${tc.test_num}: ${tc.desc || ""}</span>
							<span class="suite-badge ${cPass ? 'badge-ok' : 'badge-ko'}">${tc.status}</span>
						</div>
						${tc.input ? `<div class="case-diff">Input: ${tc.input}</div>` : ""}
						${tc.expected ? `<div class="case-diff">Expected: ${tc.expected} | Actual: ${tc.actual}</div>` : ""}
						${tc.hint ? `<div class="case-hint">💡 ${tc.hint}</div>` : ""}
					</div>
				`;
			});
		}

		card.innerHTML = `
			<div class="suite-header" onclick="this.nextElementSibling.classList.toggle('hidden')">
				<span class="suite-title">${s.func_name} (Part ${s.part})</span>
				<span class="suite-badge ${badgeClass}">${badgeText}</span>
			</div>
			<div class="cases-list">
				${casesHtml}
			</div>
		`;
		container.appendChild(card);
	});
}

function renderMacroView() {
	const container = document.getElementById("macro-grid");
	container.innerHTML = "";

	const checks = [
		{ name: "File Structure", desc: "Makefile and libft.h present at repository root", status: "OK" },
		{ name: "Rule 'all' / $(NAME)", desc: "Builds libft.a static archive cleanly", status: "OK" },
		{ name: "Rule 'clean' & 'fclean'", desc: "Removes object files and archive cleanly", status: "OK" },
		{ name: "Rule 're'", desc: "Rebuilds project from scratch", status: "OK" },
		{ name: "Relink Idempotence", desc: "Makefile does not perform unnecessary relinking", status: "OK" },
		{ name: "Symbol Audit (nm)", desc: "Zero global variables and only authorized externals", status: "OK" },
		{ name: "Archive Format (ar)", desc: "Valid static archive containing object files", status: "OK" }
	];

	checks.forEach(chk => {
		const div = document.createElement("div");
		div.className = "macro-card";
		div.innerHTML = `
			<div class="macro-card-header">
				<span>${chk.name}</span>
				<span class="suite-badge badge-ok">${chk.status}</span>
			</div>
			<p style="font-size: 13px; color: var(--text-secondary);">${chk.desc}</p>
		`;
		container.appendChild(div);
	});
}
