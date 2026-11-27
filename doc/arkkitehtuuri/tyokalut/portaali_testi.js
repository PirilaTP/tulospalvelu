// Smoke test of the knowledge-base portal without a browser (phase K-4/K-5).
// Usage (from the repository root): node doc/arkkitehtuuri/tyokalut/portaali_testi.js
// Loads doc/tietopohja/portaali/data/data.js and app.js into a minimal DOM stub, visits every
// route, and fails if a view throws, renders nothing, or the page refers to a network resource.
"use strict";
const fs = require("fs");
const path = require("path");
const vm = require("vm");
const dir = path.join("doc", "tietopohja", "portaali");
const failures = [];

for (const f of ["index.html", "app.js", "tyyli.css"]) {
  const t = fs.readFileSync(path.join(dir, f), "utf8");
  const net = t.match(/(?:src|href)\s*=\s*["'](?:https?:)?\/\/[^"']+/gi) || [];
  const imp = t.match(/@import|url\(\s*["']?https?:/gi) || [];
  const fetchCalls = t.match(/\b(fetch|XMLHttpRequest|WebSocket|EventSource)\b/g) || [];
  if (net.length || imp.length || fetchCalls.length) failures.push(f + ": network reference " + net.concat(imp, fetchCalls).join(" "));
}

function el() {
  return { innerHTML: "", value: "", textContent: "", classList: { toggle() {} }, style: {},
    addEventListener() {}, getAttribute() { return null; }, querySelectorAll() { return []; }, appendChild() {}, removeChild() {}, select() {} };
}
const els = {};
let listeners = {};
const location = { hash: "" };
const document = {
  getElementById(id) { return els[id] || (els[id] = el()); },
  querySelectorAll() { return []; },
  createElement() { return el(); },
  body: el(),
  execCommand() { return true; },
};
const ctx = { window: {}, document, location, navigator: {}, setTimeout() {}, console };
ctx.window.addEventListener = (n, f) => { listeners[n] = f; };
ctx.window.scrollTo = () => {};
ctx.window.print = () => {};
ctx.window.prompt = () => {};
vm.createContext(ctx);
vm.runInContext(fs.readFileSync(path.join(dir, "data", "data.js"), "utf8"), ctx);
const D = ctx.window.TIETOPOHJA;
if (!D) { console.log("FAIL: data.js did not define window.TIETOPOHJA"); process.exit(1); }
vm.runInContext(fs.readFileSync(path.join(dir, "app.js"), "utf8"), ctx);

const routes = ["#/", "#/havainnot", "#/havainnot?prioriteetti=P1", "#/havainnot?ohjelma=ViestiWin&tila=avoin",
  "#/havainnot?vaikutus=ajanotto&arvo=vm", "#/havainnot?vaikutus=tulokset&arvo=", "#/havainnot?teksti=emit",
  "#/juurisyyt", "#/kokonaisuudet", "#/paatokset", "#/paatokset/OP-1", "#/kysymykset", "#/eteneminen",
  "#/testaus", "#/kattavuus", "#/kasikirja", "#/sanasto", "#/haku?q=emit", "#/haku?q=H-033"];
D.kokonaisuudet.forEach(p => routes.push("#/kokonaisuus/" + p.kk));
D.havainnot.forEach(h => { routes.push("#/kortti/" + h.id); if (h.kortti) routes.push("#/kortti/" + h.id + "?nakyma=tekninen"); });
D.kasikirja.forEach(c => routes.push("#/kasikirja/" + c.id));

let n = 0;
for (const r of routes) {
  location.hash = r;
  try {
    listeners.hashchange();
    const html = els.sisalto.innerHTML;
    if (!html || html.length < 20) failures.push(r + ": empty view");
    if (/>undefined<|"undefined"|undefined<\/|\[object Object\]|NaN</.test(html)) failures.push(r + ": 'undefined' or [object Object] in output");
    n++;
  } catch (e) {
    failures.push(r + ": " + e.message);
  }
}
// cards: user view must not be the full technical card, technical view must contain the sources table
for (const h of D.havainnot.filter(x => x.kortti && x.laji === "H" && x.tila === "avoin")) {
  if (h.kayttaja.indexOf("Tekniset tiedot ja lähteet") >= 0) failures.push(h.id + ": technical section in user view");
  if (h.tekninen.indexOf("Lähdeviitteet") < 0) failures.push(h.id + ": technical view lacks source references");
}
console.log("routes visited: " + n + ", findings: " + D.havainnot.length + ", cards: " + D.havainnot.filter(x => x.kortti).length +
  ", chapters: " + D.kasikirja.length);
failures.slice(0, 40).forEach(f => console.log("FAIL " + f));
console.log(failures.length ? failures.length + " failures" : "OK");
process.exit(failures.length ? 1 : 0);
