/* ft_bro dashboard. Vanilla ES6, zero dependencies, no network access - it has
   to render on a cluster machine with no internet and be safe to hand to
   someone else (SPEC_FRONTEND, decision B10). */
"use strict";

const DATA = JSON.parse(document.getElementById("bro-data").textContent);
const BAD = new Set(["KO","SIGSEGV","SIGBUS","SIGABRT","TIMEOUT","LEAK","FAIL"]);
const UNSCORED = new Set(["UB","MISSING","SKIP"]);
const LEVELS = {1:"ASCII & classification",2:"Pointer traversal",3:"Copying & comparing",
  4:"Sized buffers & parsing",5:"Allocation",6:"Double & function pointers",
  7:"File descriptors",8:"Linked lists"};

const el = (t, cls, txt) => { const n = document.createElement(t);
  if (cls) n.className = cls; if (txt !== undefined) n.textContent = txt; return n; };
const allCases = () => DATA.suites.flatMap(s => s.cases.map(c => ({...c, suite: s})));

/* ---------------------------------------------------------------- header */
function header() {
  const s = DATA.summary;
  document.getElementById("target").textContent = DATA.target;
  const version = DATA.bro_version || "";
  document.querySelector(".brand").title = "ft_bro " + version;
  const ver = document.getElementById("version");
  if (ver) {
    ver.textContent = "v" + version;
    /* DATA.update_available is filled in by report.py from the cache that
       `bro --check-update` writes. The page never asks the network itself:
       it has to render on a cluster machine with no internet (A2, A8). */
    if (DATA.update_available) {
      ver.textContent = "v" + version + " → " + DATA.update_available;
      ver.classList.add("stale");
      ver.title = "ft_bro " + DATA.update_available
        + " is available. Update with: git pull && make";
    } else {
      ver.title = "ft_bro " + version;
    }
  }
  document.getElementById("c-cases").innerHTML =
    `<b>${s.passed}/${s.total_cases}</b> cases`;
  document.getElementById("c-funcs").innerHTML =
    `<b>${s.present_funcs}/${s.total_funcs}</b> written`;
  const extra = Object.entries(s.counts)
    .filter(([k]) => UNSCORED.has(k)).map(([k, v]) => `${v} ${k}`).join(" · ");
  document.getElementById("c-extra").textContent = extra || "nothing skipped";
  document.getElementById("theme").onclick = () => {
    const r = document.documentElement;
    r.dataset.theme = r.dataset.theme === "dark" ? "light" : "dark";
  };
  wireRerun();
}

/* bro --serve is the ONLY thing that makes these live (design/07_WEB_UX.md):
   the page detects its own protocol rather than being told, and over
   file:// the buttons are not drawn at all rather than drawn dead. */
function applyReport(fresh) {
  Object.keys(DATA).forEach(k => delete DATA[k]);
  Object.assign(DATA, fresh);
  header();
  render();
}
function wireRerun() {
  if (typeof location === "undefined" || location.protocol !== "http:") return;
  const run = (btn, url, body, busyLabel, idleLabel) => {
    btn.hidden = false;
    btn.onclick = async () => {
      btn.disabled = true; const was = btn.textContent; btn.textContent = busyLabel;
      try {
        const res = await fetch(url, {method: "POST",
          headers: body ? {"Content-Type": "application/json"} : undefined,
          body: body ? JSON.stringify(body) : undefined});
        if (res.ok) applyReport(await res.json());
      } catch (e) { /* server not reachable - leave the page as it was */ }
      finally { btn.disabled = false; btn.textContent = idleLabel || was; }
    };
  };
  run(document.getElementById("rerun"), "/api/run", null, "running…");
  run(document.getElementById("rerun-macro"), "/api/macro", null, "running…");
}

/* ------------------------------------------------------------- evidence */
function byteChar(b) {
  const map = {0:"\\0",10:"\\n",13:"\\r",9:"\\t"};
  if (map[b] !== undefined) return map[b];
  return (b >= 32 && b < 127) ? String.fromCharCode(b) : b.toString(16).padStart(2,"0");
}
function byteGrid(c) {
  if (!c.expected_bytes) return null;
  const n = Math.min(c.expected_bytes.length, 24), base = c.window_base || 0;
  const wrap = el("div", "scroll"), t = el("table", "bytes");
  const rows = [["idx", null], ["exp", c.expected_bytes], ["got", c.actual_bytes]];
  rows.forEach(([label, vals], ri) => {
    const tr = el("tr", ri === 1 ? "exp" : ri === 2 ? "got" : "");
    tr.appendChild(el("td", "lbl", label));
    for (let i = 0; i < n; i++) {
      const td = el("td", "", vals ? byteChar(vals[i]) : String(base + i));
      if (c.diverge >= 0 && base + i === c.diverge && vals) td.classList.add("diff");
      tr.appendChild(td);
    }
    t.appendChild(tr);
  });
  wrap.appendChild(t);
  if (c.diverge >= 0) wrap.appendChild(el("div", "small muted",
    `first difference at index ${c.diverge}`));
  return wrap;
}

/* ---------------------------------------------------------------- memory
   SPEC_FRONTEND #5. Two halves that must never be confused for each other:
     - the BUFFER VIEW is MEASURED - bytes the engine actually captured.
     - the STEP REPLAY is SIMULATED - the report has no per-step trace, only
       the call and (for byte-comparison cases) the final buffer, so the
       walk-through below is derived here, in the browser, from the case's
       own input text. Every place that draws it says so; it is never
       presented as something the engine recorded. */
const STEP_FUNCS = ["ft_memmove", "ft_split", "ft_strtrim"];

function unescapeC(s) {
  const MAP = {"0": "\0", "n": "\n", "t": "\t", "r": "\r",
    "\\": "\\", "\"": "\"", "'": "'"};
  return s.replace(/\\(.)/g, (_, c) => (c in MAP ? MAP[c] : c));
}

/* The four-row layout SPEC_FRONTEND #5 asks for: idx / chr / hex / ptr.
   Reuses byteChar() rather than re-deriving how a byte prints. */
function bufferView(bytes, base, markers) {
  const wrap = el("div", "scroll"), t = el("table", "bytes");
  const rows = {idx: el("tr"), chr: el("tr"), hex: el("tr"), ptr: el("tr")};
  rows.idx.appendChild(el("td", "lbl", "idx"));
  rows.chr.appendChild(el("td", "lbl", "chr"));
  rows.hex.appendChild(el("td", "lbl", "hex"));
  rows.ptr.appendChild(el("td", "lbl", "ptr"));
  bytes.forEach((b, i) => {
    rows.idx.appendChild(el("td", "", String(base + i)));
    rows.chr.appendChild(el("td", "", byteChar(b)));
    rows.hex.appendChild(el("td", "mono", "0x" + b.toString(16).padStart(2, "0").toUpperCase()));
    const here = markers.filter(m => m.i === base + i);
    rows.ptr.appendChild(el("td", here.length ? "mark" : "", here.map(m => m.label).join(" ")));
  });
  [rows.idx, rows.chr, rows.hex, rows.ptr].forEach(r => t.appendChild(r));
  wrap.appendChild(t);
  return wrap;
}

/* Same shape, for a plain JS string being walked in the browser (split /
   strtrim simulations) rather than a captured byte array. */
function charStrip(str, markers) {
  const wrap = el("div", "scroll"), t = el("table", "bytes");
  const idxRow = el("tr"), chrRow = el("tr"), ptrRow = el("tr");
  idxRow.appendChild(el("td", "lbl", "idx"));
  chrRow.appendChild(el("td", "lbl", "chr"));
  ptrRow.appendChild(el("td", "lbl", "ptr"));
  for (let i = 0; i < str.length; i++) {
    idxRow.appendChild(el("td", "", String(i)));
    chrRow.appendChild(el("td", "", byteChar(str.charCodeAt(i))));
    const here = markers.filter(m => m.i === i);
    ptrRow.appendChild(el("td", here.length ? "mark" : "", here.map(m => m.label).join(" ")));
  }
  [idxRow, chrRow, ptrRow].forEach(r => t.appendChild(r));
  wrap.appendChild(t);
  return wrap;
}

/* Plays one memmove call forward or backward across n bytes, mutating a
   working copy exactly the way a real (mis)implementation would - so
   "clobbered" falls out of the simulation itself rather than being asserted:
   a direction is unsafe if and only if it ever reads a cell it already wrote. */
function simulateDirection(seed, doff, soff, n, dir) {
  const work = seed.slice(), written = new Set(), steps = [];
  for (let k = 0; k < n; k++) {
    const kk = dir === "forward" ? k : n - 1 - k;
    const s = soff + kk, d = doff + kk;
    const clobbered = written.has(s);
    const value = work[s];
    work[d] = value;
    written.add(d);
    steps.push({k: kk, srcIdx: s, dstIdx: d, value, clobbered});
  }
  return steps;
}

/* ft_memmove(dest, "literal", n)  -> non-overlapping copy, source known.
   ft_memmove(buf [+ off], buf [+ off], n) -> overlap, shared buffer. */
function deriveMemmoveSteps(c) {
  const m = (c.input || "").match(/^ft_memmove\(\s*([^,]+?)\s*,\s*([^,]+?)\s*,\s*(\d+)\s*\)$/);
  if (!m) return {ok: false, reason: "could not parse this call's arguments"};
  const n = parseInt(m[3], 10);
  const lit = m[2].match(/^"(.*)"$/);
  if (lit) {
    const src = unescapeC(lit[1]);
    const srcBytes = Array.from(src, ch => ch.charCodeAt(0));
    // A C string literal's storage includes its own trailing '\0'; when n
    // covers more than the literal's characters that terminator (and, in
    // this suite, only that) fills the rest.
    while (srcBytes.length < n) srcBytes.push(0);
    return {ok: true, overlap: false, n, srcBytes,
      seedNote: "the destination's prior bytes are guard filler, not shown - " +
        "this call does not overlap, so write order cannot corrupt anything"};
  }
  const dm = m[1].match(/^(\w+)(?:\s*\+\s*(\d+))?$/);
  const sm = m[2].match(/^(\w+)(?:\s*\+\s*(\d+))?$/);
  if (!dm || !sm || dm[1] !== sm[1])
    return {ok: false, reason: "unrecognised argument shape: " + m[1] + ", " + m[2]};
  const doff = dm[2] ? parseInt(dm[2], 10) : 0;
  const soff = sm[2] ? parseInt(sm[2], 10) : 0;
  // The report holds only the post-move result, not a pre-move snapshot, so
  // the seed below is NOT read from this case's data - it is this suite's
  // fixed overlap fixture (engine/tests/part1/test_ft_memmove.c,
  // overlap_case: memcpy(buf, "abcde", 6) into a 16-byte guarded buffer).
  const seed = [97, 98, 99, 100, 101, 0, 170, 170, 170, 170, 170, 170, 170, 170, 170, 170];
  return {ok: true, overlap: true, n, doff, soff, seed,
    seedNote: "seeded from this suite's fixed overlap fixture (" + dm[1] +
      " = \"abcde\"), not from a captured pre-move snapshot"};
}

/* ft_split("literal", 'c') - delimiter skipping, word sizing, one allocation
   event per completed word. The 100-word generated case has no literal to
   replay from, so it is honestly refused rather than faked. */
function deriveSplitSteps(c) {
  const m = (c.input || "").match(/^ft_split\("((?:[^"\\]|\\.)*)",\s*'((?:\\.|[^'])+)'\)$/);
  if (!m) return {ok: false, reason:
    "this call's string is generated at run time and is not embedded in the " +
    "report - pick a different case"};
  const s = unescapeC(m[1]);
  const sep = unescapeC(m[2]);
  const sepCh = sep.length ? sep[0] : "\0";
  let inWord = false, wordStart = -1;
  const words = [], steps = [];
  for (let i = 0; i <= s.length; i++) {
    const ch = i < s.length ? s[i] : null;
    const isSep = ch === null || ch === sepCh;
    let note;
    if (!isSep && !inWord) {
      inWord = true; wordStart = i;
      note = `index ${i}: '${ch}' begins a new word`;
    } else if (isSep && inWord) {
      inWord = false;
      const w = s.slice(wordStart, i); words.push(w);
      note = `index ${i}: delimiter - word #${words.length} "${w}" is complete and gets allocated`;
    } else if (isSep) {
      note = ch === null ? "end of string" : `index ${i}: delimiter, not inside a word - skipped`;
    } else {
      note = `index ${i}: '${ch}' - still inside word #${words.length + 1}`;
    }
    steps.push({i, note, wordsSoFar: words.slice()});
  }
  steps[steps.length - 1].note +=
    ` - done: ${words.length} word(s) allocated, NULL-terminated array returned`;
  return {ok: true, s, steps};
}

/* ft_strtrim("literal", "set") - start advances while s[start] is in set,
   end retreats while s[end-1] is in set, and what happens when they cross. */
function deriveTrimSteps(c) {
  const m = (c.input || "").match(/^ft_strtrim\("((?:[^"\\]|\\.)*)",\s*"((?:[^"\\]|\\.)*)"\)$/);
  if (!m) return {ok: false, reason: "could not parse this call's arguments"};
  const s = unescapeC(m[1]), set = unescapeC(m[2]);
  const inSet = ch => set.length > 0 && set.includes(ch);
  const steps = [];
  let start = 0;
  while (start < s.length && inSet(s[start])) {
    steps.push({phase: "start", i: start, note: `index ${start}: '${s[start]}' is in the set - advance`});
    start++;
  }
  steps.push({phase: "start", i: Math.min(start, s.length - 1),
    note: start >= s.length ? "reached the end - nothing survives"
      : `index ${start}: '${s[start]}' is not in the set - start stops here`});
  let end = s.length;
  while (end > start && inSet(s[end - 1])) {
    steps.push({phase: "end", i: end - 1, note: `index ${end - 1}: '${s[end - 1]}' is in the set - retreat`});
    end--;
  }
  const result = start < end ? s.slice(start, end) : "";
  steps.push({phase: "end", i: Math.max(end - 1, 0),
    note: (end <= start ? "start and end crossed - " : `end stops at index ${end - 1} - `) +
      `result is "${result}"`});
  return {ok: true, s, steps, start, end, result};
}

function stepSlider(n, onchange) {
  const range = el("input");
  range.type = "range"; range.min = "0"; range.max = String(Math.max(n - 1, 0)); range.value = "0";
  range.oninput = () => onchange(parseInt(range.value, 10) || 0);
  return range;
}

function stepperMemmove(model) {
  const wrap = el("div", "");
  wrap.appendChild(el("div", "small muted", model.seedNote));
  if (model.n === 0) {
    wrap.appendChild(el("p", "muted", "n = 0 - nothing is moved for this call."));
    return wrap;
  }
  if (!model.overlap) {
    const frame = el("div", ""), label = el("div", "small");
    const srcStr = model.srcBytes.map(b => String.fromCharCode(b)).join("");
    const update = i => {
      frame.innerHTML = "";
      frame.appendChild(charStrip(srcStr, [{i, label: "▲"}]));
      label.textContent = `step ${i + 1}/${model.n}: write dst[${i}] = src[${i}] ` +
        `(${byteChar(model.srcBytes[i])}) - no overlap, so order cannot corrupt anything`;
    };
    wrap.appendChild(stepSlider(model.n, update));
    wrap.appendChild(frame); wrap.appendChild(label);
    update(0);
    return wrap;
  }
  const n = model.n;
  const fSim = simulateDirection(model.seed, model.doff, model.soff, n, "forward");
  const bSim = simulateDirection(model.seed, model.doff, model.soff, n, "backward");
  const fSafe = !fSim.some(x => x.clobbered), bSafe = !bSim.some(x => x.clobbered);
  const frame = el("div", "stepwrap"), label = el("div", "small");
  const side = (name, sim, safe, k) => {
    const st = sim[Math.min(k, sim.length - 1)];
    const box = el("div", "stepbox");
    box.appendChild(el("div", "small " + (safe ? "s-OK" : "s-KO"),
      `${name}  ${safe ? "✓ safe for this call" : "✗ corrupts this call"}`));
    box.appendChild(el("div", "mono small",
      `move #${st.k + 1}: dst[${st.dstIdx}] ← src[${st.srcIdx}]  (${byteChar(st.value)})` +
      (st.clobbered ? "  — already overwritten by an earlier move this direction" : "")));
    return box;
  };
  const update = k => {
    frame.innerHTML = "";
    frame.appendChild(side("forward (low→high)", fSim, fSafe, k));
    frame.appendChild(side("backward (high→low)", bSim, bSafe, k));
    label.textContent = `move ${k + 1} of ${n}`;
  };
  wrap.appendChild(stepSlider(n, update));
  wrap.appendChild(label); wrap.appendChild(frame);
  update(0);
  return wrap;
}

function stepperSplit(model) {
  const wrap = el("div", ""), frame = el("div", "");
  const label = el("div", "small"), words = el("div", "small mono");
  const update = i => {
    const st = model.steps[i];
    frame.innerHTML = "";
    frame.appendChild(charStrip(model.s, st.i < model.s.length ? [{i: st.i, label: "▲"}] : []));
    label.textContent = `step ${i + 1}/${model.steps.length} — ${st.note}`;
    words.textContent = "words so far: " + (st.wordsSoFar.length
      ? st.wordsSoFar.map(w => JSON.stringify(w)).join(", ") : "(none yet)");
  };
  wrap.appendChild(stepSlider(model.steps.length, update));
  wrap.appendChild(frame); wrap.appendChild(label); wrap.appendChild(words);
  update(0);
  return wrap;
}

function stepperTrim(model) {
  const wrap = el("div", ""), frame = el("div", ""), label = el("div", "small");
  const update = i => {
    const st = model.steps[i];
    frame.innerHTML = "";
    const marker = st.i >= 0 && st.i < model.s.length
      ? [{i: st.i, label: st.phase === "start" ? "▲start" : "▲end"}] : [];
    frame.appendChild(charStrip(model.s, marker));
    label.textContent = `step ${i + 1}/${model.steps.length} [${st.phase}] — ${st.note}`;
  };
  wrap.appendChild(stepSlider(model.steps.length, update));
  wrap.appendChild(frame); wrap.appendChild(label);
  wrap.appendChild(el("div", "small muted", `result: "${model.result}"`));
  update(0);
  return wrap;
}

function memoryCandidates() {
  return DATA.suites.filter(s => STEP_FUNCS.includes(s.func_name) ||
    s.cases.some(c => c.expected_bytes && c.expected_bytes.length));
}

function viewMemory(main, focus) {
  const cands = memoryCandidates();
  if (!cands.length) {
    main.appendChild(el("p", "muted",
      "No case in this report carries byte-level evidence to inspect."));
    return;
  }
  const [ffn, fidRaw] = (focus || "").split(":");
  const suite = cands.find(s => s.func_name === ffn) || cands[0];
  const fid = fidRaw !== undefined ? parseInt(fidRaw, 10) : null;

  main.appendChild(el("p", "small muted",
    "The visual inspector for spatial bugs. Buffer view below is measured " +
    "— bytes the engine actually captured. Step replay is simulated in " +
    "the browser from the case's own input, not recorded per-step by the engine."));

  const picker = el("div", "grid");
  cands.forEach(s => {
    const n = el("div", "node" + (s === suite ? " ok" : ""), s.func_name.replace("ft_", ""));
    n.onclick = () => go("memory", s.func_name);
    picker.appendChild(n);
  });
  main.appendChild(picker);

  const sel = el("select", "mono small");
  suite.cases.forEach(c => {
    const opt = el("option");
    opt.value = String(c.id);
    opt.textContent = `${String(c.id).padStart(2, "0")}  ${c.status}  ${c.input || ""}`;
    if (fid === c.id) opt.selected = true;
    sel.appendChild(opt);
  });
  sel.onchange = () => go("memory", `${suite.func_name}:${sel.value}`);
  main.appendChild(sel);

  const chosen = (fid !== null && suite.cases.find(c => c.id === fid)) ||
    suite.cases.find(c => c.expected_bytes && c.expected_bytes.length) ||
    suite.cases[0];

  main.appendChild(el("h2", "", "Buffer view — measured"));
  if (chosen.expected_bytes && chosen.expected_bytes.length) {
    const base = chosen.window_base || 0;
    const bytes = chosen.actual_bytes || chosen.expected_bytes;
    const zero = bytes.indexOf(0);
    const markers = [{i: base, label: "▲start"},
      {i: base + (zero >= 0 ? zero : bytes.length - 1), label: "▲end"}];
    main.appendChild(bufferView(bytes, base, markers));
    if (chosen.write_extent !== undefined)
      main.appendChild(el("div", "small muted",
        `guard: this call wrote ${chosen.write_extent} byte(s)`));
    main.appendChild(el("div", "small muted", "expected vs. actual (captured):"));
    main.appendChild(byteGrid(chosen));
  } else {
    main.appendChild(el("p", "muted",
      "No byte data was captured for this case — its evidence is structural " +
      "(a word list or an allocation trail), not a byte buffer."));
  }

  main.appendChild(el("h2", "", "Step replay — simulated"));
  if (!STEP_FUNCS.includes(suite.func_name)) {
    main.appendChild(el("p", "muted",
      "Step replay is only available for ft_memmove, ft_split and ft_strtrim."));
    return;
  }
  const model = suite.func_name === "ft_memmove" ? deriveMemmoveSteps(chosen)
    : suite.func_name === "ft_split" ? deriveSplitSteps(chosen)
    : deriveTrimSteps(chosen);
  if (!model.ok) {
    main.appendChild(el("p", "muted", "Can't replay this case: " + model.reason));
    return;
  }
  main.appendChild(suite.func_name === "ft_memmove" ? stepperMemmove(model)
    : suite.func_name === "ft_split" ? stepperSplit(model) : stepperTrim(model));
}

function rollback(c) {
  if (!c.rollback) return null;
  const d = el("div", "roll");
  d.innerHTML = `<span class="muted small">${c.alloc.under_test} allocation(s), each forced to fail in turn: </span>` +
    c.rollback.map((ok, k) => `<span class="${ok ? "y" : "n"}">${ok ? "✓" : "✗"}#${k+1}</span>`).join(" ");
  return d;
}

/* A range sweep: one block per integer tested, laid out in rows of 32 so the
   ASCII structure is legible - control bytes, then printable, then the high
   half each occupy whole rows. The shape of a failure is the point: a solid
   red band reads as "a whole range is wrong", scattered red as "a table is
   wrong", and neither is visible from a first-divergence message alone. */
function sweep(c) {
  if (!c.sweep) return null;
  const d = el("div", "sweep");
  const { base, bits, scored } = c.sweep;
  const head = el("div", "muted small");
  const bad = bits.filter(b => !b).length;
  head.textContent = scored
    ? `${bits.length} values from ${base} to ${base + bits.length - 1}` +
      (bad ? ` · ${bad} disagree` : " · all agree")
    : `${bits.length} values from ${base} to ${base + bits.length - 1}` +
      ` · not graded · ${bits.filter(Boolean).length} returned true`;
  d.appendChild(head);

  const grid = el("div", "blocks");
  bits.forEach((ok, i) => {
    const v = base + i;
    const b = el("span", "blk " + (scored ? (ok ? "y" : "n") : (ok ? "t" : "f")));
    b.title = scored
      ? `${v}${printable(v)} — ${ok ? "matches" : "DIFFERS"}`
      : `${v}${printable(v)} — returned ${ok ? "true" : "false"}`;
    grid.appendChild(b);
  });
  d.appendChild(grid);
  return d;
}

function printable(v) {
  return (v >= 32 && v < 127) ? ` '${String.fromCharCode(v)}'` : "";
}

/* ------------------------------------------------------------ case card */
function caseCard(c) {
  const d = el("details", "card");
  d.id = `${c.fn}:${c.id}`;
  const sum = el("summary");
  sum.appendChild(el("span", "pill s-" + c.status, c.status));
  sum.appendChild(el("span", "mono small", `${String(c.id).padStart(2,"0")}`));
  sum.appendChild(el("span", "mono", c.input || ""));
  if (c.msg) { const m = el("span", "small muted", c.msg); m.style.marginLeft = "auto"; sum.appendChild(m); }
  d.appendChild(sum);

  const b = el("div", "body");
  const g = byteGrid(c); if (g) b.appendChild(g);
  const r = rollback(c); if (r) b.appendChild(r);
  const sw = sweep(c); if (sw) b.appendChild(sw);
  if (c.expected !== undefined && !c.expected_bytes)
    b.appendChild(el("div", "mono small", `expected ${c.expected}   got ${c.actual}`));
  const kv = el("div", "kv");
  const add = (k, v) => { if (!v) return; kv.appendChild(el("div","k",k));
    kv.appendChild(el("div","",v)); };
  add("why", c.why);
  add(c.status === "UB" ? "note" : "fix", c.status === "UB" ? c.note : c.fix);
  b.appendChild(kv);
  const foot = el("div", "small");
  (c.kw || []).forEach(k => {
    const t = el("span", "tag", k);
    t.onclick = ev => { ev.preventDefault(); go("concepts", k); };
    foot.appendChild(t);
  });
  if (c.expected_bytes && c.expected_bytes.length) {
    const m = el("span", "tag", "🔍 inspect in Memory");
    m.onclick = ev => { ev.preventDefault(); go("memory", `${c.fn}:${c.id}`); };
    foot.appendChild(m);
  }
  if (c.ref) foot.appendChild(el("span", "muted", " " + c.ref));
  b.appendChild(foot);
  d.appendChild(b);
  return d;
}

/* --------------------------------------------------------------- views */
/* SPEC_FRONTEND #2: full graph layout is not the point - the point is that a
   red node whose prerequisite is also red should sit visibly downstream of
   it, so a student fixes the root cause first. So the dependent (not the
   root cause) is the one that gets subordinated: dimmed, and labelled with
   which prerequisite to fix first - a word, not just a colour/opacity. */
function viewRoadmap(main) {
  const levels = {};
  DATA.suites.forEach(s => (levels[s.level] = levels[s.level] || []).push(s));
  const byName = {};
  DATA.suites.forEach(s => { byName[s.func_name] = s; });
  const statusOf = s => !s.present ? "none" : s.pass_count === s.test_count ? "ok" : "bad";
  Object.keys(levels).sort((a, b) => a - b).forEach(lv => {
    main.appendChild(el("h2", "", `Level ${lv} — ${LEVELS[lv] || ""}`));
    const g = el("div", "grid");
    levels[lv].forEach(s => {
      const cls = statusOf(s);
      const prereqs = s.prereqs || [];
      const unmet = prereqs.filter(p => byName[p] && statusOf(byName[p]) !== "ok");
      const subordinate = cls === "bad" && unmet.length > 0;
      const n = el("div", "node " + cls + (subordinate ? " subordinate" : ""));
      n.appendChild(el("span", "", s.func_name.replace("ft_", "")));
      if (unmet.length) n.appendChild(el("div", "small prereq",
        "needs " + unmet.map(p => p.replace("ft_", "")).join(", ")));
      n.title = (s.present ? `${s.pass_count}/${s.test_count}` : "not written") +
        (prereqs.length ? `  ·  prerequisites: ${prereqs.join(", ")}` : "");
      n.onclick = () => go("tests", s.func_name);
      g.appendChild(n);
    });
    main.appendChild(g);
  });
}

function viewTests(main, focus) {
  DATA.suites.forEach(s => {
    const d = el("details", "card");
    d.id = s.func_name;
    if (focus === s.func_name) d.open = true;
    const sum = el("summary");
    sum.appendChild(el("b", "", s.func_name));
    sum.appendChild(el("span", "small muted", `part ${s.part} · level ${s.level} · T${s.tier}`));
    const bar = el("div", "bar" + (s.pass_count === s.test_count ? "" : " bad"));
    const fill = el("i"); fill.style.width =
      (s.test_count ? 100 * s.pass_count / s.test_count : 0) + "%";
    bar.appendChild(fill);
    const sp = el("span", "spacer"); sum.appendChild(sp); sum.appendChild(bar);
    sum.appendChild(el("span", "small mono",
      s.present ? `${s.pass_count}/${s.test_count}` : "not written"));
    d.appendChild(sum);
    const b = el("div", "body");
    if (s.prototype) b.appendChild(el("pre", "", s.prototype));
    s.cases.forEach(c => b.appendChild(caseCard(c)));
    d.appendChild(b);
    main.appendChild(d);
  });
}

/* The cross-cutting index: which functions test this idea, and am I getting
   it. The live roll-up is what no static card can do. */
function viewConcepts(main, focus) {
  const cases = allCases();
  Object.values(DATA.concepts).forEach(card => {
    const d = el("details", "card");
    d.id = "concept/" + card.slug;
    if (focus === card.slug) d.open = true;
    const tagged = cases.filter(c => (c.kw || []).includes(card.slug));
    const ok = tagged.filter(c => c.status === "OK").length;
    const sum = el("summary");
    sum.appendChild(el("b", "", card.slug));
    sum.appendChild(el("span", "small muted", card.title));
    sum.appendChild(el("span", "spacer"));
    if (tagged.length) sum.appendChild(el("span",
      "pill " + (ok === tagged.length ? "s-OK" : "s-KO"), `${ok}/${tagged.length}`));
    d.appendChild(sum);
    const b = el("div", "body");
    b.appendChild(el("pre", "", card.body));
    if (tagged.length) {
      b.appendChild(el("div", "small muted",
        `tested in ${tagged.length} case(s) across ` +
        `${new Set(tagged.map(c => c.fn)).size} function(s)`));
      const g = el("div", "grid");
      tagged.forEach(c => {
        const n = el("div", "node " + (c.status === "OK" ? "ok" :
          UNSCORED.has(c.status) ? "none" : "bad"),
          `${c.fn.replace("ft_","")} ${String(c.id).padStart(2,"0")}`);
        n.onclick = () => go("tests", c.fn, `${c.fn}:${c.id}`);
        g.appendChild(n);
      });
      b.appendChild(g);
    }
    if (card.refs.length) b.appendChild(el("div", "small muted",
      "reference: " + card.refs.join(" · ")));
    card.defense.forEach(q => b.appendChild(el("div", "small", "defense: “" + q + "”")));
    d.appendChild(b);
    main.appendChild(d);
  });
}

function viewMacro(main) {
  if (!DATA.macro.length) {
    main.appendChild(el("p", "muted", "No build checks in this report (run without --no-macro)."));
    return;
  }
  const names = {1:"Structure",2:"Makefile rules",3:"Relinking",4:"Compiler & flags",
    5:"Symbols",6:"Archive",7:"Norm",8:"README"};
  const bySec = {};
  DATA.macro.forEach(c => (bySec[c.section] = bySec[c.section] || []).push(c));
  Object.keys(bySec).sort().forEach(sec => {
    main.appendChild(el("h2", "", `${sec}. ${names[sec] || ""}`));
    bySec[sec].forEach(c => {
      const d = el("details", "card");
      d.id = "macro/" + c.name.replace(/\s+/g, "-");
      const sum = el("summary");
      sum.appendChild(el("span", "pill s-" + c.status, c.status));
      sum.appendChild(el("b", "", c.name));
      sum.appendChild(el("span", "small muted", c.desc));
      d.appendChild(sum);
      const b = el("div", "body");
      if (c.hint) b.appendChild(el("div", "small", c.hint));
      /* A check that cannot show its work is indistinguishable from a guess,
         and these are the findings students most often dispute. */
      if (c.command) b.appendChild(el("div", "small muted mono", "ran: " + c.command));
      if (c.output) b.appendChild(el("pre", "", c.output));
      d.appendChild(b);
      main.appendChild(d);
    });
  });
}

function viewDefense(main) {
  DATA.defense.forEach(q => {
    const d = el("details", "card");
    const sum = el("summary");
    sum.appendChild(el("b", "", "Q" + q.n));
    sum.appendChild(el("span", "", q.question));
    d.appendChild(sum);
    const b = el("div", "body");
    b.appendChild(el("div", "", q.answer));
    const f = el("div", "small");
    q.concepts.forEach(k => { const t = el("span", "tag", k);
      t.onclick = ev => { ev.preventDefault(); go("concepts", k); }; f.appendChild(t); });
    b.appendChild(f);
    d.appendChild(b);
    main.appendChild(d);
  });
}

function viewProgress(main) {
  const h = DATA.history;
  if (!h.length) { main.appendChild(el("p","muted","No history yet - run bro again tomorrow."));
    return; }
  main.appendChild(el("h2", "", "Cases passing, per run"));
  const s = el("div", "spark");
  const max = Math.max(...h.map(e => e.total || 1), 1);
  h.forEach(e => { const i = el("i");
    i.style.height = Math.max(2, 60 * (e.pass || 0) / max) + "px";
    i.title = `${e.ts}  ${e.pass}/${e.total}`; s.appendChild(i); });
  main.appendChild(s);
  if (DATA.delta) {
    main.appendChild(el("h2", "", "Since " + DATA.delta.since));
    const d = DATA.delta;
    const line = (label, arr, cls) => { if (!arr || !arr.length) return;
      const p = el("div", "small"); p.innerHTML =
        `<span class="${cls}">${label}</span> ` + arr.join(", "); main.appendChild(p); };
    line("fixed", d.fixed, "s-OK");
    line("broke", d.broke, "s-KO");
    line("wrote", d.new_funcs, "");
    line("gone", d.gone_funcs, "s-WARN");
  }
  main.appendChild(el("h2", "", "Runs"));
  h.slice().reverse().forEach(e => {
    const row = el("div", "hit small");
    row.textContent = `${e.ts}   ${e.pass}/${e.total}` +
      (e.fails && e.fails.length ? `   failing: ${e.fails.join(", ")}` : "");
    main.appendChild(row);
  });
}

/* --------------------------------------------------------------- search */
function search(q) {
  q = q.toLowerCase();
  const hits = [];
  allCases().forEach(c => {
    const hay = [c.input, c.why, c.fix, c.note, c.fn, (c.kw||[]).join(" ")]
      .filter(Boolean).join(" ").toLowerCase();
    if (hay.includes(q)) hits.push({kind:"case", label:`${c.fn} ${String(c.id).padStart(2,"0")}`,
      sub: c.input, go: () => go("tests", c.fn, `${c.fn}:${c.id}`)});
  });
  Object.values(DATA.concepts).forEach(k => {
    if ((k.slug + " " + k.title + " " + k.body).toLowerCase().includes(q))
      hits.push({kind:"concept", label:k.slug, sub:k.title, go:() => go("concepts", k.slug)});
  });
  DATA.macro.forEach(c => {
    if ((c.name + " " + c.desc + " " + c.hint).toLowerCase().includes(q))
      hits.push({kind:"check", label:c.name, sub:c.desc, go:() => go("macro")});
  });
  DATA.defense.forEach(d => {
    if ((d.question + " " + d.answer).toLowerCase().includes(q))
      hits.push({kind:"defense", label:"Q" + d.n, sub:d.question, go:() => go("defense")});
  });
  return hits;
}

function viewSearch(main, q) {
  const hits = search(q);
  main.appendChild(el("h2", "", `${hits.length} result(s) for “${q}”`));
  hits.slice(0, 120).forEach(h => {
    const row = el("div", "hit");
    row.innerHTML = `<span class="chip">${h.kind}</span> <b>${h.label}</b> ` +
      `<span class="muted small">${(h.sub || "").slice(0, 90)}</span>`;
    row.onclick = h.go;
    main.appendChild(row);
  });
}

/* ----------------------------------------------------------------- shell */
const VIEWS = {roadmap:["Roadmap",viewRoadmap], tests:["Tests",viewTests],
  concepts:["Concepts",viewConcepts], memory:["Memory",viewMemory],
  defense:["Defense",viewDefense], macro:["Build",viewMacro],
  progress:["Progress",viewProgress]};
let current = "roadmap";

function go(view, focus, anchor) {
  current = view;
  location.hash = anchor || (focus ? `${view}/${focus}` : view);
  render(focus, anchor);
}

function render(focus, anchor) {
  const nav = document.getElementById("nav"); nav.innerHTML = "";
  Object.entries(VIEWS).forEach(([k, [label]]) => {
    const a = el("a", k === current ? "on" : "", label);
    a.onclick = () => go(k);
    nav.appendChild(a);
  });
  const main = document.getElementById("main"); main.innerHTML = "";
  const q = document.getElementById("search").value.trim();
  if (q) { viewSearch(main, q); return; }
  VIEWS[current][1](main, focus);
  if (anchor) {
    const t = document.getElementById(anchor);
    if (t) { t.open = true; t.scrollIntoView({block:"center"});
      let p = t.parentElement;
      while (p) { if (p.tagName === "DETAILS") p.open = true; p = p.parentElement; } }
  }
}

function fromHash() {
  const h = decodeURIComponent(location.hash.slice(1));
  if (!h) return render();
  // A bare "fn:id" (no "/") is always a case deep link into Tests. A
  // "view/fn:id" (memory's own deep links) must fall through to the
  // view/focus split below instead - the colon there is not a case id.
  if (!h.includes("/") && h.includes(":")) {
    const fn = h.split(":")[0]; current = "tests"; return render(fn, h);
  }
  const [view, focus] = h.split("/");
  if (VIEWS[view]) { current = view; return render(focus); }
  render();
}

header();
document.getElementById("search").addEventListener("input", () => render());
document.addEventListener("keydown", e => {
  if (e.key === "/" && document.activeElement.id !== "search") {
    e.preventDefault(); document.getElementById("search").focus();
  }
});
window.addEventListener("hashchange", fromHash);
fromHash();
