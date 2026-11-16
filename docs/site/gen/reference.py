"""Reference pages of libutils: one page per section of the library = the text written by hand (src/reference/<slug>.<lang>.md)
followed by what is read from the headers: its C++ API (public classes, functions, enums of include/utils/<dir>/*.hpp), its C API
(include/utils/<dir>/CApi.h, by group) and, for some sections, a table read from the sources (exception codes, attribute macros).
search_entries() adds every function of the C API and every C++ type to the search."""

import json
import re
import subprocess
from pathlib import Path

import _cppapi
import mdlib

# slug, include dirs of the C++ API (relative to include/utils), C headers of the C API (relative to include/utils), extra generated part
DOMAINS = [
    ("exception", ["exception"], ["exception/CApi.h"], "codes"),
    ("verbose", ["verbose"], ["verbose/CApi.h"], ""),
    ("pool", ["pool"], ["pool/CApi.h"], ""),
    ("cli", ["cli"], ["cli/CApi.h"], ""),
    ("arguments", ["arguments"], ["arguments/CApi.h"], ""),
    ("network", ["network"], ["network/CApi.h"], ""),
    ("math", ["math"], ["math/CApi.h"], ""),
    ("concepts", ["concepts"], [], ""),
    ("encapsulation", ["encapsulation"], ["encapsulation/CApi.h"], ""),
    ("system", ["system"], ["system/CApi.h"], ""),
    ("type", ["type"], ["type/CApiTypes.h", "type/CApi.h"], ""),
    ("iomanip", ["manip/iomanip"], ["manip/iomanip/CApi.h"], ""),
    ("smanip", ["manip/smanip"], ["manip/smanip/CApi.h"], ""),
    ("algorithms", ["algorithms"], ["algorithms/CApi.h"], ""),
    ("security", ["security"], ["security/CApi.h"], ""),
    ("platform", ["platform"], ["platform/CApi.h", "platform/Types.h"], ""),
    ("attribute", [], [], "attributes"),
]
SKIPPED = ("CApi.hpp", "CApiTypes.hpp")  # the C++ side of the C API: implementation helpers, not an API of the section
WORDS = {
    "cpp": {"en": "C++ API", "fr": "API C++"},
    "c": {"en": "C API", "fr": "API C"},
    "cpp_lead": {"en": "Read from the headers of `include/utils/%s/` (public part only, the trailing comments of the code kept). Include `<utils/utils.hpp>` with the define of the section, never these headers one by one.",
                 "fr": "Lu dans les en-têtes de `include/utils/%s/` (partie publique seulement, avec les commentaires du code). Inclure `<utils/utils.hpp>` avec le define de la section, jamais ces en-têtes un par un."},
    "c_lead": {"en": "Read from %s, included by `<utils.h>` with the define of the section. Every function that can fail returns `UTILS_OK` / `UTILS_KO` (or `NULL` / `0`) and fills `utils_exception` (see [the C API guide](page:c-api#errors)).",
               "fr": "Lu dans %s, inclus par `<utils.h>` avec le define de la section. Chaque fonction qui peut échouer renvoie `UTILS_OK` / `UTILS_KO` (ou `NULL` / `0`) et remplit `utils_exception` (voir [le guide de l'API C](page:c-api#errors))."},
    "no_c": {"en": "This section has no C API: its content only exists at compile time in C++.",
             "fr": "Cette section n'a pas d'API C : son contenu n'existe qu'à la compilation, en C++."},
    "namespace": {"en": "Namespace", "fr": "Namespace"},
    "codes": {"en": "Codes", "fr": "Codes"},
    "codes_lead": {"en": "`utils::exception::InternalCode::<Code>` (the same value in `utils_exception.code` of the C API), generated at the build from `cmake/config/exceptions/**/*.json`. **Restrictions**: the types a code accepts (`-` = every type).",
                   "fr": "`utils::exception::InternalCode::<Code>` (la même valeur dans `utils_exception.code` de l'API C), générés au build depuis `cmake/config/exceptions/**/*.json`. **Restrictions** : les types qu'un code accepte (`-` = tous)."},
    "head_codes": {"en": "| Code | Message | Restrictions |", "fr": "| Code | Message | Restrictions |"},
    "macros": {"en": "Macros", "fr": "Macros"},
    "macros_lead": {"en": "Read from `include/utils/attribute/`: the definition of each macro in C++20 (`c++20.hpp` + `common.hpp`) and in C23 (`c23.h` + `common.h`); an empty cell is an empty macro (attribute not available there). The other versions define the same names, empty when the attribute does not exist in that version.",
                    "fr": "Lu dans `include/utils/attribute/` : la définition de chaque macro en C++20 (`c++20.hpp` + `common.hpp`) et en C23 (`c23.h` + `common.h`) ; une case vide est une macro vide (attribut indisponible). Les autres versions définissent les mêmes noms, vides quand l'attribut n'existe pas dans cette version."},
    "head_macros": {"en": "| Macro | C++20 | C23 | Role |", "fr": "| Macro | C++20 | C23 | Rôle |"},
    "kinds": {"MACRO": ("Macros", "Macros"), "DEFINE": ("Defines", "Defines"), "TYPEDEF": ("Types", "Types"), "ENUM": ("Enums", "Énumérations"),
              "STRUCT": ("Structures", "Structures"), "GLOBAL": ("Globals", "Globales"), "PROTOTYPE": ("Functions", "Fonctions")},
}

def root(ctx: object) -> Path:
    return (ctx.site / ".." / "..").resolve()

def w(key: str, lang: str) -> str:
    return WORDS[key][lang]

# ----- C++ API -----
def tracked(ctx: object) -> set[str]:
    """The headers of the repository (a header generated at the build and ignored by git is not documented)."""
    run = subprocess.run(["git", "-C", str(root(ctx)), "ls-files", "include"], capture_output=True, text=True)
    return set(run.stdout.split()) if run.returncode == 0 else set()

def cpp_headers(ctx: object, dirs: list[str]) -> list[Path]:
    base = root(ctx) / "include" / "utils"
    files = tracked(ctx)
    out = []
    for d in dirs:
        out += [p for p in sorted((base / d).rglob("*.hpp"))
                if p.name not in SKIPPED and (not files or p.relative_to(root(ctx)).as_posix() in files)]
    return out

def cpp_part(ctx: object, slug: str, dirs: list[str], lang: str, entries: list[dict] | None) -> str:
    base = root(ctx) / "include"
    out = ["## %s {#cpp-api}" % w("cpp", lang), "", w("cpp_lead", lang) % "`, `include/utils/".join(dirs), ""]
    for path in cpp_headers(ctx, dirs):
        text = path.read_text(encoding="utf-8")
        rel = path.relative_to(base).as_posix()
        try:
            items = _cppapi.Parser(text).parse()
        except Exception as e:  # never block the build on one header
            items = [{"kind": "using", "name": "", "ns": "", "sig": "/* not parsed: %s */" % e, "comment": "", "members": []}]
        body = []
        defs = [d for d in _cppapi.defines(text) if not d[0].startswith("_") or "(" in d[0]]
        if defs:
            body += [("#define %s %s" % (n, v if len(v) <= 80 else "...")).rstrip() + (" // %s" % c if c else "") for n, v, c in defs] + [""]
        current = None
        for item in items:
            if item["ns"] != current:
                current = item["ns"]
                body.append("// namespace %s" % current if current else "// global")
            body += _cppapi.render_item(item)
            if entries is not None and item["kind"] in ("class", "struct", "union", "enum") and item["name"]:
                entries.append({"name": "%s::%s" % (item["ns"], item["name"]) if item["ns"] else item["name"], "id": anchor(rel),
                                "desc": item["comment"] or rel})
        if not any(line.strip() for line in body):
            continue
        out += ["### `%s` {#%s}" % (rel, anchor(rel)), ""]
        desc = _cppapi.description(text)
        if desc:
            out += [desc, ""]
        out += ["```cpp", *body, "```", ""]
    return "\n".join(out)

def anchor(rel: str) -> str:
    return "h-" + mdlib.slugify(re.sub(r"^utils/", "", rel))

# ----- C API -----
def c_groups(text: str) -> list[dict]:
    """The groups of a C header of the C API: [{kind, label, lines}] (sections /* KIND */ after a //---- banner, groups /* label */)."""
    groups: list[dict] = []
    kind, banner, cur, started, depth_cpp = None, False, None, False, 0
    for line in text.split("\n"):
        s = line.strip()
        if not started:
            started = s.startswith("\\****")
            continue
        if s.startswith("//---"):
            banner = True
            continue
        m = re.fullmatch(r"/\*\s*([A-Z][A-Z _]+?)\s*\*/", s)
        if banner and m:
            kind, banner, cur = m.group(1), False, None
            continue
        banner = False
        if s.startswith("// xstyle:") or re.match(r"#\s*(ifndef|define)\s+\w+_H\b", s) or re.match(r"#\s*endif\s*/\*\s*\w+_H\s*\*/", s):
            continue
        if s == "#ifdef __cplusplus":
            depth_cpp = 1
            continue
        if depth_cpp:
            if s.startswith("#endif"):
                depth_cpp = 0
            continue
        if kind in (None, "INCLUDE"):
            continue
        m = re.fullmatch(r"/\*\s*(.+?)\s*\*/", s)
        if m and not line.rstrip().endswith("\\"):
            cur = {"kind": kind, "label": m.group(1), "lines": []}
            groups.append(cur)
            continue
        if cur is None:
            if not s:
                continue
            cur = {"kind": kind, "label": "", "lines": []}
            groups.append(cur)
        cur["lines"].append(line.rstrip())
    for g in groups:
        while g["lines"] and not g["lines"][-1].strip():
            g["lines"].pop()
        while g["lines"] and not g["lines"][0].strip():
            g["lines"].pop(0)
        indent = min((len(x) - len(x.lstrip()) for x in g["lines"] if x.strip()), default=0)
        g["lines"] = [x[indent:] for x in g["lines"]]
    return [g for g in groups if g["lines"]]

def short_label(label: str) -> str:
    """The heading of a group: its label up to the first ' (' or ':' (the rest is said under it)."""
    head = re.split(r"\s\(|:\s|,\s", label, maxsplit=1)[0].strip()
    return (head[:1].upper() + head[1:]) if head else ""

def c_names(lines: list[str]) -> list[str]:
    names = []
    for line in lines:
        code = re.sub(r"//.*$|/\*.*?\*/", "", line)
        for m in re.finditer(r"\b(utils_[\w#]+?)\s*\(", code):
            name = m.group(1)
            if line.lstrip().startswith("#define") and code.lstrip().startswith("#define %s(" % name):
                pass
            name = re.sub(r"##(\w+)##", r"<\1>", name).replace("##", "")
            names.append(name)
    return list(dict.fromkeys(names))

def c_part(ctx: object, slug: str, headers: list[str], lang: str, entries: list[dict] | None) -> str:
    out = ["## %s {#c-api}" % w("c", lang), ""]
    if not headers:
        return "\n".join(out + [w("no_c", lang), ""])
    base = root(ctx) / "include" / "utils"
    out += [w("c_lead", lang) % ", ".join("`include/utils/%s`" % h for h in headers), ""]
    n = 0
    for h in headers:
        text = (base / h).read_text(encoding="utf-8")
        if len(headers) > 1:
            out += ["### `utils/%s` {#c-%s}" % (h, mdlib.slugify(h)), ""]
            desc = _cppapi.description(text)
            if desc:
                out += [desc, ""]
        level = "####" if len(headers) > 1 else "###"
        for g in c_groups(text):
            n += 1
            gid = "c-%d" % n
            kind_word = WORDS["kinds"].get(g["kind"], (g["kind"].title(), g["kind"].title()))[0 if lang == "en" else 1]
            title = kind_word + (" · " + short_label(g["label"]) if short_label(g["label"]) else "")
            out += ["%s %s {#%s}" % (level, title, gid), ""]
            if g["label"] and g["label"] != short_label(g["label"]) and len(g["label"]) > len(short_label(g["label"])) + 2:
                out += ["*%s*" % g["label"].replace("*", "\\*"), ""]
            out += ["```c", *g["lines"], "```", ""]
            if entries is not None:
                for name in c_names(g["lines"]):
                    entries.append({"name": name, "id": gid, "desc": g["label"] or h})
    return "\n".join(out)

# ----- tables read from the sources -----
def codes_part(ctx: object, lang: str) -> str:
    folder = root(ctx) / "cmake" / "config" / "exceptions"
    out = ["## %s {#codes}" % w("codes", lang), "", w("codes_lead", lang), ""]
    for path in sorted(folder.rglob("*.json")):
        errors = json.loads(path.read_text(encoding="utf-8")).get("errors", [])
        if not errors:
            continue
        rel = path.relative_to(folder).as_posix()
        out += ["### `%s` {#codes-%s}" % (rel, mdlib.slugify(rel[:-5])), "", w("head_codes", lang), "|---|---|---|"]
        for e in errors:
            out.append("| `%s` | %s | %s |" % (e.get("code", "?"), e.get("message", "").replace("|", "\\|"), ", ".join(e.get("restrictions", [])) or "-"))
        out.append("")
    return "\n".join(out)

def macro_defs(paths: list[Path]) -> dict[str, dict]:
    """Every definition of each macro (a macro can depend on the platform or the compiler): {name, values, comment}."""
    out: dict[str, dict] = {}
    for path in paths:
        for name, value, comment in _cppapi.defines(path.read_text(encoding="utf-8")):
            key = name.split("(")[0]
            if not key.startswith("_") or key.startswith("__"):
                continue
            d = out.setdefault(key, {"name": name, "values": [], "comment": ""})
            if value not in d["values"]:
                d["values"].append(value)
            if comment and not d["comment"] and value:
                d["comment"] = comment
    return out

def attributes_part(ctx: object, lang: str) -> str:
    base = root(ctx) / "include" / "utils" / "attribute"
    cpp = macro_defs([base / "c++20.hpp", base / "common.hpp"])
    c = macro_defs([base / "c23.h", base / "common.h"])
    empty = "*(empty)*" if lang == "en" else "*(vide)*"
    out = ["## %s {#macros}" % w("macros", lang), "", w("macros_lead", lang), "", w("head_macros", lang), "|---|---|---|---|"]
    def cell(values: list[str]) -> str:
        return " / ".join(("`%s`" % v.replace("|", "\\|")) if v else empty for v in values)
    for key in list(cpp) + [k for k in c if k not in cpp]:
        a, b = cpp.get(key), c.get(key)
        name = (a or b)["name"]
        comment = (a and a["comment"]) or (b and b["comment"]) or ""
        out.append("| `%s` | %s | %s | %s |" % (name, cell(a["values"]) if a else "-", cell(b["values"]) if b else "-", comment.replace("|", "\\|")))
    return "\n".join(out + [""])

# ----- pages -----
def pages(ctx: object) -> list[dict]:
    out = []
    folder = ctx.site / "src" / "reference"
    for order, (slug, dirs, headers, extra) in enumerate(DOMAINS, 1):
        page = {"slug": slug, "section": "reference", "order": order * 10, "kind": "", "source": folder / ("%s.%s.md" % (slug, ctx.langs[0]))}
        for lang in ctx.langs:
            path = folder / ("%s.%s.md" % (slug, lang))
            if not path.exists():
                raise SystemExit("reference: %s is missing" % path)
            front, body = mdlib.split_front(path.read_text(encoding="utf-8"))
            parts = [body.rstrip(), ""]
            if extra == "codes":
                parts.append(codes_part(ctx, lang))
            if extra == "attributes":
                parts.append(attributes_part(ctx, lang))
            if dirs:
                parts.append(cpp_part(ctx, slug, dirs, lang, None))
            if dirs or headers:
                parts.append(c_part(ctx, slug, headers, lang, None))
            page[lang] = {"title": front["title"], "summary": front.get("summary", ""), "keywords": front.get("keywords", ""), "md": "\n".join(parts)}
        out.append(page)
    return out

def search_entries(ctx: object) -> list[dict]:
    out, seen = [], set()
    for slug, dirs, headers, _ in DOMAINS:
        found: list[tuple[str, dict]] = []
        cpp: list[dict] = []
        c: list[dict] = []
        if dirs:
            cpp_part(ctx, slug, dirs, "en", cpp)
        if headers:
            c_part(ctx, slug, headers, "en", c)
        found = [("api", e) for e in cpp + c]
        for lang in ctx.langs:
            for kind, e in found:
                key = (lang, e["name"])
                if key in seen:
                    continue
                seen.add(key)
                out.append({"k": kind, "l": lang, "t": e["name"], "d": e["desc"][:110], "u": "%s.html#%s" % (slug, e["id"]), "x": ""})
    return out
