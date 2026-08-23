/* Headless smoke test for the dashboard.
 *
 * The page is self-contained and every DATA.* field app.js reads has been
 * checked to exist - but nobody has ever WATCHED it render, and the failure
 * that check cannot see is the likeliest one: a JS exception on load leaving a
 * blank page. Structurally perfect data, nothing drawn.
 *
 * So: a minimal DOM shim, enough to execute app.js for real, and assertions
 * that each view actually produces content. No npm dependencies - a hand-rolled
 * shim keeps the toolchain as dependency-free as the page itself.
 *
 *   node tools/render_check.js <report.html>
 */
"use strict";
const fs = require("fs");

const file = process.argv[2];
if (!file) { console.error("usage: render_check.js <report.html>"); process.exit(2); }
const html = fs.readFileSync(file, "utf8");

const dataMatch = html.match(
  /<script id="bro-data" type="application\/json">([\s\S]*?)<\/script>/);
const appMatch = html.match(/<script>([\s\S]*?)<\/script>\s*<\/body>/);
if (!dataMatch) { console.error("no inlined data block"); process.exit(1); }
if (!appMatch) { console.error("no app script block"); process.exit(1); }

/* ------------------------------------------------------------- DOM shim */
let created = 0;
function elem(tag) {
  created++;
  return {
    tagName: (tag || "div").toUpperCase(),
    children: [], _text: "", className: "", id: "", title: "",
    style: {}, dataset: {}, open: false, value: "",
    classList: { add() {}, remove() {}, contains() { return false; } },
    appendChild(c) { this.children.push(c); return c; },
    addEventListener() {}, scrollIntoView() {}, focus() {},
    querySelector() { return elem("div"); },
    set textContent(v) { this._text = String(v); },
    get textContent() {
      return this._text + this.children.map(c => c.textContent).join("");
    },
    set innerHTML(v) { this._text = String(v).replace(/<[^>]*>/g, ""); },
    get innerHTML() { return this._text; },
  };
}

const byId = {};
["target", "c-cases", "c-funcs", "c-extra", "theme", "search", "nav", "main",
 "version"]
  .forEach(id => { byId[id] = elem("div"); byId[id].id = id; });
byId["bro-data"] = elem("script");
byId["bro-data"]._text = dataMatch[1];

global.document = {
  getElementById: id => byId[id] || null,
  createElement: elem,
  querySelector: sel => byId[sel.replace(/^[.#]/, "")] || elem("div"),
  addEventListener() {},
  documentElement: { dataset: {} },
  activeElement: { id: "" },
};
global.location = { hash: "" };
global.window = { addEventListener() {} };

/* --------------------------------------------------------------- run it */
let app;
try {
  app = new Function(appMatch[1] + "\n;return {go:typeof go==='function'?go:null,"
    + "VIEWS:typeof VIEWS!=='undefined'?VIEWS:null};");
} catch (e) {
  console.error("app.js failed to PARSE: " + e.message); process.exit(1);
}

let handle;
try { handle = app(); }
catch (e) {
  console.error("app.js THREW on load: " + e.stack.split("\n").slice(0, 3).join(" | "));
  process.exit(1);
}

const problems = [];
const nav = byId.nav, main = byId.main;
if (!nav.children.length) problems.push("nav rendered no entries");
if (!main.children.length) problems.push("default view rendered nothing");
if (!byId["c-cases"].textContent.match(/\d+\/\d+/))
  problems.push("case counter is empty: " + JSON.stringify(byId["c-cases"].textContent));

/* The version is provenance: a report with no version on it cannot be placed
   in time. When the cache says something newer exists, the notice has to be
   visible in the page too, not only in the terminal. */
const ver = byId["version"];
const payload = JSON.parse(dataMatch[1]);
if (!ver || !ver.textContent.includes(payload.bro_version))
  problems.push("version is missing from the header: "
    + JSON.stringify(ver && ver.textContent));
if (payload.update_available && !ver.textContent.includes(payload.update_available))
  problems.push("report carries update_available=" + payload.update_available
    + " but the header does not show it: " + JSON.stringify(ver.textContent));

/* every view must render without throwing and produce content */
if (handle.VIEWS && handle.go) {
  for (const name of Object.keys(handle.VIEWS)) {
    main.children.length = 0;
    try { handle.go(name); }
    catch (e) { problems.push(`view "${name}" threw: ${e.message}`); continue; }
    if (!main.children.length) problems.push(`view "${name}" rendered nothing`);
  }
} else {
  problems.push("could not reach VIEWS/go - the shim needs updating");
}

if (problems.length) {
  console.error("render check FAILED:");
  problems.forEach(p => console.error("  " + p));
  process.exit(1);
}
console.log(`  ok   render              ${Object.keys(handle.VIEWS).length} views, `
  + `${created} elements, no exceptions`);
