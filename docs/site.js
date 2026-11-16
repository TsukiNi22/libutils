window.DOC_FR = { "Toggle theme": "Changer de thème", "Contents": "Sommaire", "Language": "Langue", "Copy": "Copier", "Copied": "Copié", "Select & copy": "Sélectionner et copier", "Link to this section": "Lien vers cette section", "Open the navigation": "Ouvrir la navigation", "Guide": "Guide", "Technical": "Technique", "Graph": "Graphe", "Pages": "Pages", "Previous": "Précédent", "Next": "Suivant", "GitHub repository": "Dépôt GitHub", "Skip to the content": "Aller au contenu" };
// Behaviour shared by the user's HTML pages (from html-doc): theme switch, optional EN / FR switch, copy buttons
// on the code blocks, heading anchors, content tabs, mobile menu. Inlined in each page by scripts/new_page.py.

// Language (only when the page has a .lang-switch): [data-i18n] elements and [data-i18n-<attr>] attributes are
// translated with window.DOC_FR (English text -> French text), [data-lang] blocks are shown for their language only.
(function () {
  var root = document.documentElement;
  var key = (root.getAttribute('data-storage-key') || 'page') + '-lang';
  var FR = window.DOC_FR || {};
  var listeners = [];
  function tr(s) { return root.lang === 'fr' && Object.prototype.hasOwnProperty.call(FR, s) ? FR[s] : s; }
  function apply() {
    document.querySelectorAll('[data-i18n]').forEach(function (el) {
      if (el.dataset.i18nEn === undefined) el.dataset.i18nEn = el.innerHTML.trim();
      el.innerHTML = tr(el.dataset.i18nEn);
    });
    ['aria-label', 'title', 'placeholder'].forEach(function (attr) {
      document.querySelectorAll('[data-i18n-' + attr + ']').forEach(function (el) {
        var store = 'i18nEn' + attr.replace(/-(\w)/g, function (m, c) { return c.toUpperCase(); });
        if (el.dataset[store] === undefined) el.dataset[store] = el.getAttribute(attr) || '';
        el.setAttribute(attr, tr(el.dataset[store]));
      });
    });
    document.querySelectorAll('[data-set-lang]').forEach(function (b) {
      b.classList.toggle('on', b.dataset.setLang === root.lang);
      b.setAttribute('aria-pressed', b.dataset.setLang === root.lang ? 'true' : 'false');
    });
  }
  function set(lang) {
    root.lang = lang === 'fr' ? 'fr' : 'en';
    try { localStorage.setItem(key, root.lang); } catch (e) { /* storage unavailable */ }
    apply();
    listeners.forEach(function (f) { f(root.lang); });
  }
  var saved = null;
  if (document.querySelector('.lang-switch')) {
    try { saved = localStorage.getItem(key); } catch (e) { /* storage unavailable */ }
  }
  // Choice carried by a link of the site (?lang=): pages opened from file:// don't share their storage
  var askedLang = new URLSearchParams(location.search).get('lang');
  if (askedLang === 'en' || askedLang === 'fr') { saved = askedLang; try { localStorage.setItem(key, askedLang); } catch (e) { /* storage unavailable */ } }
  root.lang = saved === 'fr' ? 'fr' : (root.lang || 'en');
  window.docI18n = { tr: tr, set: set, apply: apply, onChange: function (f) { listeners.push(f); }, lang: function () { return root.lang; } };
  document.querySelectorAll('[data-set-lang]').forEach(function (b) {
    b.addEventListener('click', function () { set(b.dataset.setLang); });
  });
  apply();
})();

// Links between the pages of the site carry the language, theme and level (?lang=fr&theme=dark&level=technical):
// the choice applies to every page, also from file:// where each page has its own storage
(function () {
  var root = document.documentElement;
  function carry(e) {
    var a = e.target.closest && e.target.closest('a[href]');
    if (!a || a.target === '_blank') return;
    var href = a.getAttribute('href');
    if (!/^[^:?#\/]+\.html(#.*)?$/.test(href) && !/^\.\/[^:?#\/]+\.html(#.*)?$/.test(href)) return; // pages of the same folder only
    var u = new URL(a.href);
    u.searchParams.set('lang', root.lang || 'en');
    if (root.getAttribute('data-theme')) u.searchParams.set('theme', root.getAttribute('data-theme'));
    if (root.getAttribute('data-level')) u.searchParams.set('level', root.getAttribute('data-level'));
    a.href = u.href;
  }
  document.addEventListener('click', carry, true);
  document.addEventListener('auxclick', carry, true);
})();

// Theme: sun / moon switch, choice saved, default = system preference
(function () {
  var root = document.documentElement;
  var key = (root.getAttribute('data-storage-key') || 'page') + '-theme';
  var btn = document.getElementById('themeToggle');
  try { var saved = localStorage.getItem(key); if (saved) root.setAttribute('data-theme', saved); } catch (e) { /* storage unavailable */ }
  var askedTheme = new URLSearchParams(location.search).get('theme'); // carried by a link of the site
  if (askedTheme === 'dark' || askedTheme === 'light') { root.setAttribute('data-theme', askedTheme); try { localStorage.setItem(key, askedTheme); } catch (e) { /* storage unavailable */ } }
  function isDark() {
    return root.getAttribute('data-theme') === 'dark' ||
      (!root.getAttribute('data-theme') && window.matchMedia('(prefers-color-scheme: dark)').matches);
  }
  if (!btn) return;
  btn.setAttribute('aria-checked', isDark() ? 'true' : 'false');
  btn.addEventListener('click', function () {
    var next = isDark() ? 'light' : 'dark';
    root.setAttribute('data-theme', next);
    btn.setAttribute('aria-checked', next === 'dark' ? 'true' : 'false');
    try { localStorage.setItem(key, next); } catch (e) { /* storage unavailable */ }
    document.dispatchEvent(new CustomEvent('themechange', { detail: next }));
  });
})();

// Figures: an SVG is never drawn larger than its viewBox (its text would grow with the screen)
document.querySelectorAll('figure svg[viewBox]').forEach(function (svg) {
  var w = +svg.getAttribute('viewBox').split(/[\s,]+/)[2];
  if (w && !svg.style.maxWidth) { svg.style.maxWidth = w + 'px'; svg.style.marginInline = 'auto'; }
});

// A drawing on a small screen: never under 72 % of its size (labels of 13 px stay readable), it scrolls in a box of its own, its caption stays.
// Pages drawing their figures themselves are left alone (explain-doc: data-page="explain", protocol-doc: a min-width of their own)
if (document.documentElement.getAttribute('data-page') !== 'explain') document.querySelectorAll('figure > svg[viewBox]').forEach(function (svg) {
  var width = +svg.getAttribute('viewBox').split(/[\s,]+/)[2];
  if (!(width > 400) || svg.style.minWidth) return;
  var box = document.createElement('div');
  box.className = 'figure-scroll';
  svg.parentNode.insertBefore(box, svg);
  box.appendChild(svg);
  svg.style.minWidth = Math.round(width * 0.72) + 'px';
  function mark() { // a block that scrolls is reached and scrolled with the keyboard
    if (box.scrollWidth > box.clientWidth + 1) {
      box.setAttribute('tabindex', '0');
      box.setAttribute('role', 'region');
      box.setAttribute('aria-label', svg.getAttribute('aria-label') || 'Figure');
    } else ['tabindex', 'role', 'aria-label'].forEach(function (name) { box.removeAttribute(name); });
  }
  mark();
  if ('ResizeObserver' in window) new ResizeObserver(mark).observe(box);
  else window.addEventListener('resize', mark);
});

// Diagram links: an arrow is computed from the shapes it joins instead of typed coordinates, so it always touches
// them. <path class="edge" data-from="a" data-to="b" marker-end="url(#arr)"/>: a / b = id of a rect, circle, ellipse
// (or a <g> holding one). Options: data-from="a:right" (side: top, bottom, left, right), data-bend="0.2" (curve, +/-
// fraction of the length), data-gap="2" (space before the shape). A label <text data-label-for="pathId"
// data-offset="-8"> is placed on the middle of the path. Geometry is read from the attributes (works while hidden).
window.docDiagram = (function () {
  function translateOf(el, svg) {
    var x = 0, y = 0;
    for (var n = el; n && n !== svg; n = n.parentNode) {
      var t = n.getAttribute && n.getAttribute('transform');
      var m = t && t.match(/translate\(\s*(-?[\d.]+)[\s,]*(-?[\d.]+)?\s*\)/);
      if (m) { x += +m[1]; y += +(m[2] || 0); }
    }
    return [x, y];
  }
  function shape(svg, id) {
    var el = svg.querySelector('[id="' + id + '"]');
    if (el && el.tagName.toLowerCase() === 'g') el = el.querySelector('rect, circle, ellipse');
    if (!el) return null;
    var o = translateOf(el, svg), num = function (a) { return +(el.getAttribute(a) || 0); };
    var tag = el.tagName.toLowerCase();
    if (tag === 'rect') return { kind: 'rect', x: num('x') + o[0], y: num('y') + o[1], w: num('width'), h: num('height') };
    if (tag === 'circle') return { kind: 'ellipse', cx: num('cx') + o[0], cy: num('cy') + o[1], rx: num('r'), ry: num('r') };
    return { kind: 'ellipse', cx: num('cx') + o[0], cy: num('cy') + o[1], rx: num('rx'), ry: num('ry') };
  }
  function center(s) { return s.kind === 'rect' ? [s.x + s.w / 2, s.y + s.h / 2] : [s.cx, s.cy]; }
  function side(s, name) {
    var c = center(s), hw = s.kind === 'rect' ? s.w / 2 : s.rx, hh = s.kind === 'rect' ? s.h / 2 : s.ry;
    return { top: [c[0], c[1] - hh], bottom: [c[0], c[1] + hh], left: [c[0] - hw, c[1]], right: [c[0] + hw, c[1]] }[name];
  }
  // Point of the border of s in the direction of p, moved back by gap
  function border(s, p, gap) {
    var c = center(s), dx = p[0] - c[0], dy = p[1] - c[1], len = Math.hypot(dx, dy) || 1, k;
    if (s.kind === 'rect') k = Math.min(dx ? (s.w / 2) / Math.abs(dx) : Infinity, dy ? (s.h / 2) / Math.abs(dy) : Infinity);
    else k = 1 / Math.sqrt((dx * dx) / (s.rx * s.rx) + (dy * dy) / (s.ry * s.ry));
    return [c[0] + dx * k + dx / len * gap, c[1] + dy * k + dy / len * gap];
  }
  // Point of the border of s for an arrow coming from / going to toward; off spreads several arrows of the same
  // shape along its side (instead of all reaching the same point)
  function end(s, spec, toward, gap, off) {
    var name = spec.split(':')[1], p;
    if (name) p = side(s, name);
    else if (off && s.kind === 'rect') {
        var c = center(s), dx = toward[0] - c[0], dy = toward[1] - c[1];
        if (Math.abs(dy) * s.w >= Math.abs(dx) * s.h) p = [c[0] + off * s.w, c[1] + (dy < 0 ? -1 : 1) * s.h / 2];
        else p = [c[0] + (dx < 0 ? -1 : 1) * s.w / 2, c[1] + off * s.h];
    } else p = border(s, toward, 0);
    var vx = toward[0] - p[0], vy = toward[1] - p[1], len = Math.hypot(vx, vy) || 1;
    return [p[0] + vx / len * gap, p[1] + vy / len * gap];
  }
  // Slot (-0.3 .. 0.3 of the side) of each arrow end sharing a shape, ordered by where the other end is
  function slots(items) {
    var groups = {};
    items.forEach(function (it) { if (!it.spec.split(':')[1]) (groups[it.id] = groups[it.id] || []).push(it); });
    Object.keys(groups).forEach(function (id) {
      var g = groups[id], s = g[0].shape, c = center(s);
      if (g.length < 2) return;
      g.forEach(function (it) {
        var dx = it.toward[0] - c[0], dy = it.toward[1] - c[1];
        it.vertical = s.kind === 'rect' && Math.abs(dy) * s.w >= Math.abs(dx) * s.h;
        it.key = it.vertical ? dx : dy;
      });
      [true, false].forEach(function (v) {
        var sub = g.filter(function (it) { return it.vertical === v; }).sort(function (a, b) { return a.key - b.key; });
        if (sub.length > 1) sub.forEach(function (it, i) { it.off = ((i + 1) / (sub.length + 1) - 0.5) * 0.6; });
      });
    });
  }
  function layout(svg) {
    var items = [], links = [];
    svg.querySelectorAll('[data-from][data-to]').forEach(function (path) {
      var fa = path.dataset.from, ta = path.dataset.to;
      var a = shape(svg, fa.split(':')[0]), b = shape(svg, ta.split(':')[0]);
      if (!a || !b) { console.warn('diagram link: unknown shape', fa, ta); return; }
      var gap = +(path.dataset.gap || 2), bend = +(path.dataset.bend || 0);
      var ca = fa.indexOf(':') > 0 ? side(a, fa.split(':')[1]) : center(a);
      var cb = ta.indexOf(':') > 0 ? side(b, ta.split(':')[1]) : center(b);
      var mx = (ca[0] + cb[0]) / 2, my = (ca[1] + cb[1]) / 2, len = Math.hypot(cb[0] - ca[0], cb[1] - ca[1]) || 1;
      var ctrl = [mx - (cb[1] - ca[1]) / len * bend * len, my + (cb[0] - ca[0]) / len * bend * len];
      var ea = { spec: fa, id: fa.split(':')[0], shape: a, toward: bend ? ctrl : cb, off: 0 };
      var eb = { spec: ta, id: ta.split(':')[0], shape: b, toward: bend ? ctrl : ca, off: 0 };
      items.push(ea, eb);
      links.push({ path: path, gap: gap, bend: bend, ctrl: ctrl, ea: ea, eb: eb });
    });
    slots(items);
    links.forEach(function (l) {
      var path = l.path, gap = l.gap, bend = l.bend, ctrl = l.ctrl;
      var p1 = end(l.ea.shape, l.ea.spec, l.ea.toward, gap, l.ea.off), p2 = end(l.eb.shape, l.eb.spec, l.eb.toward, gap, l.eb.off);
      var f = function (p) { return p[0].toFixed(1) + ' ' + p[1].toFixed(1); };
      path.setAttribute('d', bend ? 'M' + f(p1) + ' Q' + f(ctrl) + ' ' + f(p2) : 'M' + f(p1) + ' L' + f(p2));
      var mid = bend ? [(p1[0] + 2 * ctrl[0] + p2[0]) / 4, (p1[1] + 2 * ctrl[1] + p2[1]) / 4] : [(p1[0] + p2[0]) / 2, (p1[1] + p2[1]) / 2];
      if (path.id) svg.querySelectorAll('[data-label-for="' + path.id + '"]').forEach(function (t) {
        var off = +(t.dataset.offset || -8), dx = p2[0] - p1[0], dy = p2[1] - p1[1], l = Math.hypot(dx, dy) || 1;
        var nx = -dy / l, ny = dx / l;
        if (ny > 0) { nx = -nx; ny = -ny; } // a negative offset is always above the path
        t.setAttribute('x', (mid[0] - nx * off).toFixed(1));
        t.setAttribute('y', (mid[1] - ny * off).toFixed(1));
        if (!t.getAttribute('text-anchor')) t.setAttribute('text-anchor', 'middle');
      });
    });
  }
  document.querySelectorAll('svg').forEach(layout);
  return { layout: layout };
})();

// Tables: each one in a box of its own (it scrolls there, never the page); a short cell stays on one line next to a long sentence
// (never a word per line) as long as the table fits, a word too long for any column may break. Measured again when its box changes
// (the width of the screen, a tab or a <details> opened) and with the language.
window.docTables = (function () {
  var LIMITS = [40, 28, 20, 14], WORD_KEPT = 32, CELLS_MAX = 3000;
  var tables = [];
  function crushed(item) { // a cell of three words or more, one word per line
    if (item.size === Infinity) return false;
    var words = (item.cell.innerText || '').trim().split(/\s+/).length;
    if (words < 3 || item.cell.offsetWidth >= 150) return false; // wider: long names, one a line, nothing to pair them
    var range = document.createRange(), lines = 0, last = -1e9;
    range.selectNodeContents(item.cell);
    Array.prototype.slice.call(range.getClientRects()).sort(function (a, b) { return a.top - b.top; }).forEach(function (rect) {
      var middle = (rect.top + rect.bottom) / 2;
      if (middle - last > Math.max(4, rect.height * 0.6)) { lines++; last = middle; }
    });
    return lines >= words || (lines >= 10 && words / lines < 3); // or a sentence squeezed: better a table that scrolls
  }
  function fit(table) {
    var box = table.closest('.table-wrap');
    if (!box || !box.clientWidth) return; // hidden: done when it is shown
    var cells = table.querySelectorAll('th, td');
    if (cells.length > CELLS_MAX) return;
    var items = [];
    cells.forEach(function (cell) {
      var blocks = cell.querySelector('p, ul, ol, pre, table, div, dl, blockquote');
      var text = (cell.innerText || '').trim();
      var longest = text.split(/\s+/).reduce(function (most, word) { return Math.max(most, word.length); }, 0);
      cell.style.overflowWrap = longest > WORD_KEPT ? 'anywhere' : '';
      cell.style.minWidth = longest > WORD_KEPT ? '18ch' : ''; // broken where it must, not four letters a line
      items.push({ cell: cell, size: blocks || text.indexOf('\n') >= 0 ? Infinity : text.length });
    });
    var keep = function (limit) { items.forEach(function (item) { item.cell.style.whiteSpace = item.size <= limit ? 'nowrap' : ''; }); };
    var fits = false;
    for (var at = 0; at < LIMITS.length && !fits; at++) {
      keep(LIMITS[at]);
      fits = table.offsetWidth <= box.clientWidth + 1 && !items.some(crushed); // three words on three lines is not fitting
    }
    if (!fits) { // it scrolls whatever is done: its short cells on one line, its sentences with some width
      keep(LIMITS[1]);
      items.forEach(function (item) { if (item.size > LIMITS[1]) item.cell.style.minWidth = '16em'; });
    }
    var scrolls = box.scrollWidth > box.clientWidth + 1;
    if (scrolls && !box.hasAttribute('tabindex')) { // reached and scrolled with the keyboard
      box.setAttribute('tabindex', '0');
      box.setAttribute('role', 'region');
      if (!box.getAttribute('aria-label')) box.setAttribute('aria-label', (table.caption && table.caption.textContent.trim()) || 'Table');
    }
  }
  function fitAll() { tables.forEach(fit); }
  function watch(table) {
    if (table.closest('pre, svg, .no-fit')) return;
    var box = table.closest('.table-wrap');
    if (!box) {
      box = document.createElement('div');
      box.className = 'table-wrap';
      table.parentNode.insertBefore(box, table);
      box.appendChild(table);
    }
    tables.push(table);
    if ('ResizeObserver' in window) {
      var width = -1;
      new ResizeObserver(function () {
        if (box.clientWidth === width) return;
        width = box.clientWidth;
        fit(table);
      }).observe(box);
    }
  }
  document.querySelectorAll('main table').forEach(watch);
  fitAll();
  if (!('ResizeObserver' in window)) window.addEventListener('resize', fitAll);
  if (document.fonts && document.fonts.ready) document.fonts.ready.then(fitAll);
  if (window.docI18n) window.docI18n.onChange(fitAll);
  return { fit: fitAll, watch: function (table) { watch(table); fit(table); } };
})();

// Sidebar (mobile) + visible section, copy buttons, heading anchors, content tabs
(function () {
  var tr = function (s) { return window.docI18n ? window.docI18n.tr(s) : s; };
  var menu = document.getElementById('menuBtn'), side = document.getElementById('sidebar');
  if (menu && side) {
    menu.addEventListener('click', function () { side.classList.toggle('open'); });
    side.addEventListener('click', function (e) { if (e.target.closest('a')) side.classList.remove('open'); }); // a tapped entry closes the menu
    document.addEventListener('keydown', function (e) { if (e.key === 'Escape') side.classList.remove('open'); });
    document.addEventListener('click', function (e) { if (!side.contains(e.target) && !menu.contains(e.target)) side.classList.remove('open'); });
  }
  var links = document.querySelectorAll('.sidebar a[href^="#"]');
  if (links.length && 'IntersectionObserver' in window) {
    var observer = new IntersectionObserver(function (entries) {
      entries.forEach(function (entry) {
        if (!entry.isIntersecting) return;
        links.forEach(function (a) { a.classList.toggle('active', a.getAttribute('href') === '#' + entry.target.id); });
      });
    }, { rootMargin: '-20% 0px -70% 0px' });
    document.querySelectorAll('main section[id]').forEach(function (s) { observer.observe(s); });
  }
  document.querySelectorAll('main pre').forEach(function (pre) {
    if (pre.dataset.nocopy !== undefined) return;
    var b = document.createElement('button');
    b.type = 'button'; b.className = 'copy'; b.textContent = tr('Copy');
    b.addEventListener('click', function () {
      var code = pre.querySelector('code') || pre;
      var done = function () { b.textContent = tr('Copied'); setTimeout(function () { b.textContent = tr('Copy'); }, 1400); };
      if (navigator.clipboard) navigator.clipboard.writeText(code.innerText).then(done, function () { b.textContent = tr('Select & copy'); });
      else b.textContent = tr('Select & copy');
    });
    pre.appendChild(b);
  });
  document.querySelectorAll('main h2[id], main h3[id], main section[id] > h2').forEach(function (h) {
    var id = h.id || h.parentElement.id;
    if (!id || h.querySelector('.anchor')) return;
    var a = document.createElement('a');
    a.className = 'anchor'; a.href = '#' + id; a.textContent = '#';
    a.setAttribute('aria-label', tr('Link to this section'));
    h.appendChild(a);
  });
  document.querySelectorAll('.ctabs').forEach(function (bar) {
    var group = bar.dataset.tabs;
    var buttons = bar.querySelectorAll('button');
    var panels = document.querySelectorAll('.ctab-panel[data-tabs="' + group + '"]');
    function show(name) {
      buttons.forEach(function (b) { b.classList.toggle('active', b.dataset.tab === name); });
      panels.forEach(function (p) { p.classList.toggle('active', p.dataset.tab === name); });
    }
    buttons.forEach(function (b) { b.addEventListener('click', function () { show(b.dataset.tab); }); });
    if (buttons.length) show(buttons[0].dataset.tab);
  });
  if (window.docI18n) window.docI18n.onChange(function () {
    document.querySelectorAll('main pre .copy').forEach(function (b) { b.textContent = tr('Copy'); });
    document.querySelectorAll('.anchor').forEach(function (a) { a.setAttribute('aria-label', tr('Link to this section')); });
  });
})();

/* better-html-style - search (the site index, a server endpoint, or the sections of the page itself), its keys, the phone layout
   of the search box, embed mode (?embed=1), ?theme= / ?lang= / ?q= in the address */
(function () {
  'use strict';
  var Q = new URLSearchParams(location.search);
  var EMBED = Q.get('embed') === '1';
  var root = document.documentElement;
  if (window.DOC_FR) { // the words of the site in the French table of base.js (the same object), applied again
    var words = { 'Search': 'Rechercher', 'Home': 'Accueil', 'Edit this page': 'Modifier cette page', 'Updated': 'Mis à jour', 'pages': 'pages',
      'Changelog': 'Historique', 'Full changelog': 'Historique complet', 'Sections': 'Sections', 'On this page': 'Sur cette page',
      'Shortcut / for the search': 'Raccourci / pour la recherche' };
    for (var w in words) if (!Object.prototype.hasOwnProperty.call(window.DOC_FR, w)) window.DOC_FR[w] = words[w];
    if (window.docI18n && root.lang === 'fr') window.docI18n.apply();
  }
  if (EMBED) {
    root.classList.add('embed');
    var levels = document.querySelector('.topbar .level-switch'), pageHead = document.querySelector('main > header');
    if (levels && pageHead) pageHead.appendChild(levels); // no top bar: the level switch of an explanation page stays reachable, in the header of the page
    document.querySelectorAll('a[href]').forEach(function (a) {
      var h = a.getAttribute('href');
      if (h && !/^(#|https?:|mailto:|[\w.-]+\.(pdf|rpm|deb|apk|zip|gz|sh|json)(\?|#|$))/.test(h) && h.indexOf('embed=') < 0) a.setAttribute('href', h + (h.indexOf('?') < 0 ? '?' : '&') + 'embed=1');
    });
  }
  if (Q.get('theme') === 'dark' || Q.get('theme') === 'light') root.setAttribute('data-theme', Q.get('theme'));
  if (Q.get('lang') === 'fr' || Q.get('lang') === 'en') { try { window.docI18n.set(Q.get('lang')); } catch (e) { root.lang = Q.get('lang'); } }
  var meta = document.querySelector('meta[name="docs-search"]');
  var ENDPOINT = meta ? meta.content : '';
  function lang() { return root.lang === 'fr' ? 'fr' : 'en'; }

  // the index: the site's (search-index.js, deferred: read when searching), else the sections of this page, by language
  var own = {};
  function pageIndex() {
    var l = lang();
    if (own[l]) return own[l];
    var out = [], title = document.querySelector('main h1');
    function text(el) {
      var c = el.cloneNode(true);
      c.querySelectorAll('[data-lang]:not([data-lang="' + l + '"]), script, style, svg, .copy, .anchor').forEach(function (x) { x.remove(); });
      c.querySelectorAll('th, td, li, p, div, pre, dt, dd, summary, caption, figcaption, h3, h4, h5, h6').forEach(function (x) { x.append(' '); }); // two cells or blocks never glued in a result
      return c.textContent.replace(/\s+/g, ' ').trim();
    }
    document.querySelectorAll('main section[id]').forEach(function (s) {
      var h = s.querySelector('h2'), body = text(s), head = h ? text(h) : s.id;
      out.push({ k: 'page', t: (title ? text(title) + ' › ' : '') + head, d: body.slice(head.length, head.length + 110), u: '#' + s.id, x: body.slice(0, 600) });
      s.querySelectorAll('h3[id]').forEach(function (h3) { out.push({ k: 'page', t: head + ' › ' + text(h3), d: '', u: '#' + h3.id, x: '' }); });
    });
    own[l] = out;
    return out;
  }
  function index() { return (window.DOCS_INDEX && window.DOCS_INDEX.entries) || pageIndex(); }

  // a query is words: each one must be found (prefix 3000 > start of a word 2600 > inside 2000 > a typo 900 / 800 > letters in order);
  // the title counts fully, the description a little less, the body text least
  function dist(a, b, max) {
    if (Math.abs(a.length - b.length) > max) return max + 1;
    var prev2 = null, prev = [], cur, i, j;
    for (j = 0; j <= b.length; j++) prev[j] = j;
    for (i = 1; i <= a.length; i++) {
      cur = [i];
      for (j = 1; j <= b.length; j++) {
        cur[j] = Math.min(prev[j] + 1, cur[j - 1] + 1, prev[j - 1] + (a[i - 1] === b[j - 1] ? 0 : 1));
        if (prev2 && i > 1 && j > 1 && a[i - 1] === b[j - 2] && a[i - 2] === b[j - 1]) cur[j] = Math.min(cur[j], prev2[j - 2] + 1);
      }
      prev2 = prev; prev = cur;
    }
    return prev[b.length];
  }
  // typo tolerance: a missing, doubled, wrong or swapped letter still finds it (a swap counts as one)
  function typo(q, s) {
    if (q.length < 4) return -1;
    var max = q.length >= 8 ? 2 : 1, words = s.split(/[^a-z0-9à-ÿ]+/), best = max + 1;
    for (var k = 0; k < words.length; k++) {
      var w = words[k];
      if (w.length < 3) continue;
      var d = Math.min(dist(q, w, max), dist(q, w.slice(0, q.length + 1), max) + 1);
      if (d < best) best = d;
    }
    return best <= max ? 900 - best * 100 : -1;
  }
  function wscore(w, s) {
    s = (s || '').toLowerCase();
    var i = s.indexOf(w);
    if (i === 0) return 3000 - s.length / 10;
    if (i > 0) return (/[^a-z0-9à-ÿ]/.test(s[i - 1]) ? 2600 : 2000) - i / 10;
    var t = typo(w, s);
    if (t >= 0) return t;
    if (w.length >= 4) { // letters in order, tight (sesend -> session_send)
      var p = 0, first = -1, last = 0;
      for (var j = 0; j < s.length && p < w.length; j++) if (s[j] === w[p]) { if (first < 0) first = j; last = j; p++; }
      if (p === w.length && last - first <= w.length * 2) return 600 - (last - first);
    }
    return -1;
  }
  function entryScore(words, e) {
    var total = 0;
    if ((e.t || '').toLowerCase().replace(/\s+/g, ' ').trim() === words.join(' ')) return 1e6; // its whole title: that page first, not a section of it
    for (var n = 0; n < words.length; n++) {
      var w = words[n], best = Math.max(wscore(w, e.t), wscore(w, e.d) - 400, wscore(w, e.x) - 800);
      if (best < 0) return -1;
      total += best;
    }
    return total / words.length;
  }
  function queryWords(q) { return q.toLowerCase().split(/\s+/).filter(Boolean); }
  function esc(x) { return String(x).replace(/[&<>"]/g, function (c) { return { '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;' }[c]; }); }
  function mark(q, s) { // one pass on the raw text: a word of the query never matches inside a <mark> or an &amp; written before it
    var words = queryWords(q).map(function (w) { return w.replace(/[.*+?^${}()|[\]\\]/g, '\\$&'); }).sort(function (a, b) { return b.length - a.length; });
    if (!words.length) return esc(s);
    return String(s).split(new RegExp('(' + words.join('|') + ')', 'ig')).map(function (part, n) { return n % 2 ? '<mark>' + esc(part) + '</mark>' : esc(part); }).join('');
  }
  function localSearch(q) {
    var l = lang(), words = queryWords(q);
    return index().filter(function (e) { return !e.l || e.l === l; }).map(function (e) {
      return { e: e, s: entryScore(words, e) };
    }).filter(function (x) { return x.s >= 0; }).sort(function (a, b) { return b.s - a.s; }).slice(0, 8).map(function (x) { return x.e; });
  }
  function remoteSearch(q, done) {
    fetch(ENDPOINT + '?q=' + encodeURIComponent(q) + '&lang=' + lang()).then(function (r) { return r.json(); })
      .then(function (j) { done(j.results || []); }, function () { done(localSearch(q)); });
  }
  window.docsSearch = function (q, done) { if (ENDPOINT) remoteSearch(q, done); else done(localSearch(q)); };

  function bind(input) {
    var box = input.parentNode, ul = box.querySelector('.results'), sel = 0, cur = [], seq = 0;
    function show(q, list) {
      cur = list; sel = 0;
      ul.innerHTML = cur.length ? cur.map(function (e, n) {
        var u = e.u;
        if (EMBED && u.charAt(0) !== '#') u = u.indexOf('#') >= 0 ? u.replace('#', (u.indexOf('?') < 0 ? '?' : '&') + 'embed=1#') : u + (u.indexOf('?') < 0 ? '?' : '&') + 'embed=1';
        return '<li class="' + (n ? '' : 'sel') + '"><a href="' + esc(u) + '"><span class="t">' + mark(q, e.t) + '<span class="d">' + esc(e.d || '') + '</span></span>' + (e.k ? '<span class="kind ' + esc(e.k) + '">' + esc(e.k) + '</span>' : '') + '</a></li>';
      }).join('') : '<li class="none">' + (lang() === 'fr' ? 'Aucun résultat' : 'No result') + '</li>';
      ul.hidden = false;
    }
    function render() {
      var q = input.value.trim(), n = ++seq;
      if (!q) { ul.hidden = true; return; }
      window.docsSearch(q, function (list) { if (n === seq) show(q, list); });
    }
    function move(d) {
      var li = ul.querySelectorAll('li'); if (!cur.length) return;
      li[sel].classList.remove('sel'); sel = (sel + d + cur.length) % cur.length; li[sel].classList.add('sel');
      li[sel].scrollIntoView({ block: 'nearest' });
    }
    input.addEventListener('input', render);
    input.addEventListener('focus', render);
    input.addEventListener('keydown', function (ev) {
      if (ev.key === 'ArrowDown') { ev.preventDefault(); move(1); }
      else if (ev.key === 'ArrowUp') { ev.preventDefault(); move(-1); }
      else if (ev.key === 'Enter' && cur[sel]) { ev.preventDefault(); ul.querySelectorAll('a')[sel].click(); ul.hidden = true; }
      else if (ev.key === 'Escape') { ul.hidden = true; input.blur(); }
    });
    ul.addEventListener('click', function (ev) { if (ev.target.closest('a')) ul.hidden = true; });
    document.addEventListener('click', function (ev) { if (!box.contains(ev.target)) ul.hidden = true; });
  }
  document.querySelectorAll('.sbox input').forEach(bind);
  if (Q.get('q')) window.addEventListener('load', function () { var qi = document.querySelector('.sbox input'); if (qi) { qi.value = Q.get('q'); qi.dispatchEvent(new Event('input')); } }); // ?q=text opens the results

  var menuBtn = document.querySelector('#menuBtn');
  if (menuBtn && !document.querySelector('#sidebar')) menuBtn.style.display = 'none'; // a page without sidebar: nothing for the menu to open
  (function () { // phone: the small search moves under the top bar (sticky), else it lives in the top bar
    var sb = document.querySelector('.topbar .sbox.small'), bar = document.querySelector('.topbar'); if (!sb || !bar) return;
    // a bar that also holds a level switch (explanation pages) has no room for it sooner
    var home = sb.parentNode, mq = window.matchMedia(bar.querySelector('.level-switch') ? '(max-width: 640px)' : '(max-width: 480px)');
    function place() {
      if (mq.matches) { sb.classList.add('mob'); bar.after(sb); }
      else { sb.classList.remove('mob'); home.insertBefore(sb, home.firstChild); }
    }
    place(); mq.addEventListener('change', place);
  })();
  // RGAA 12.10: "/" is a one-character shortcut, so the reader can turn it off (the choice is kept); Ctrl+K is a combination and stays
  var skey = (root.getAttribute('data-storage-key') || 'page') + '-shortcut';
  function slash() { try { return localStorage.getItem(skey) !== 'off'; } catch (e) { return true; } }
  (function () {
    var foot = document.querySelector('footer.site'); if (!foot || !document.querySelector('.sbox input') || EMBED) return;
    var lab = document.createElement('label'), box = document.createElement('input'), txt = document.createElement('span');
    lab.className = 'keys-toggle'; box.type = 'checkbox'; box.checked = slash(); txt.setAttribute('data-i18n', ''); txt.textContent = 'Shortcut / for the search';
    lab.appendChild(box); lab.appendChild(txt); foot.appendChild(lab);
    function hints() { document.querySelectorAll('.sbox kbd.hint').forEach(function (k) { k.hidden = !box.checked; }); }
    box.addEventListener('change', function () { try { localStorage.setItem(skey, box.checked ? 'on' : 'off'); } catch (e) { /* storage unavailable */ } hints(); });
    hints();
    if (window.docI18n && root.lang === 'fr') window.docI18n.apply();
  })();
  document.addEventListener('keydown', function (ev) { // / (unless turned off) or Ctrl+K: the search
    if (((ev.key === '/' && slash()) || ((ev.ctrlKey || ev.metaKey) && ev.key.toLowerCase() === 'k')) && !/INPUT|TEXTAREA|SELECT/.test(document.activeElement.tagName) && !document.activeElement.isContentEditable) {
      var big = document.querySelector('.hero .sbox input'), i = big && big.offsetParent ? big : document.querySelector('.sbox.small input') || big; // the big one when it is shown
      if (i) { ev.preventDefault(); i.focus(); i.select(); }
    }
  });
  function ph() { var fr = lang() === 'fr'; document.querySelectorAll('input[data-ph-fr]').forEach(function (i) { if (!i.dataset.ph) i.dataset.ph = i.placeholder; i.placeholder = fr ? i.dataset.phFr : i.dataset.ph; }); own = {}; }
  ph(); new MutationObserver(ph).observe(root, { attributes: true, attributeFilter: ['lang'] });
  // a host page (an app panel showing the docs in a frame) can switch the language / theme of an embedded copy
  window.addEventListener('message', function (ev) {
    var d = ev.data || {};
    if (d.docsLang && window.docI18n) window.docI18n.set(d.docsLang);
    if (d.docsTheme) root.setAttribute('data-theme', d.docsTheme);
  });
})();