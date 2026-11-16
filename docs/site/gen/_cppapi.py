"""C++ header parser of the libutils docs (taken from scripts/gen_api.py of the libutils skill): public classes, functions, enums
and defines of a header with their trailing comments. Not a generator (leading _): imported by reference.py."""

import re

ATTRIBUTE_MACROS = re.compile(r"\b_(hot|cold|nodiscard|noinline|unused|hidden|ctor|dtor|fallthrough|likely|unlikely|noaddress|packed|legacy)\b\s*")
MIGRATION = re.compile(r"_migration\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)\s*")

# ---------------------------------------------------------------- #
# Source cleaning

def split_comment(line: str) -> tuple[str, str]:
    """Return (code without comments, trailing // comment) while respecting strings."""
    code, i, quote = [], 0, None
    while i < len(line):
        c = line[i]
        if quote:
            code.append(c)
            if c == "\\" and i + 1 < len(line):
                code.append(line[i + 1]); i += 2; continue
            if c == quote: quote = None
        elif line.startswith('R"(', i):
            end = line.find(')"', i + 3)
            end = len(line) if end == -1 else end + 2
            code.append(line[i:end]); i = end; continue
        elif c in "\"'":
            quote = c; code.append(c)
        elif line.startswith("//", i):
            return "".join(code), line[i + 2:].strip()
        else:
            code.append(c)
        i += 1
    return "".join(code), ""

def clean_lines(text: str) -> list[tuple[str, str]]:
    """Remove the block comments and the preprocessor, return [(code, comment)]."""
    out, in_block = [], False
    for raw in text.split("\n"):
        line = raw
        if in_block:
            end = line.find("*/")
            if end == -1: continue
            line, in_block = line[end + 2:], False
        while True: # block comments on the same line
            code, _ = split_comment(line)
            start = code.find("/*")
            if start == -1: break
            end = line.find("*/", start + 2)
            if end == -1:
                line, in_block = line[:start], True
                break
            line = line[:start] + " " + line[end + 2:]
        code, comment = split_comment(line)
        if code.strip().startswith("#"): # preprocessor (continuations included)
            out.append(("", ""))
            continue
        out.append((code.rstrip(), comment))
    return out

def defines(text: str) -> list[tuple[str, str, str]]:
    """#define NAME value // comment (without the include guards)."""
    res = []
    for raw in text.split("\n"):
        m = re.match(r"^\s*#\s*define\s+(\w+)(\([^)]*\))?\s*(.*)$", raw)
        if not m or re.match(r"^\w+_H$", m.group(1)): continue
        value, comment = split_comment(m.group(3))
        res.append((m.group(1) + (m.group(2) or ""), value.strip().rstrip("\\").strip(), comment))
    return res

def description(text: str) -> str:
    m = re.search(r"File Description:\s*\n((?:##.*\n)+)", text)
    if not m: return ""
    desc = " ".join(l.lstrip("#").strip() for l in m.group(1).strip().split("\n"))
    return "" if desc.startswith("You know, I don t think") else desc

# ---------------------------------------------------------------- #
# Parsing

def simplify(sig: str) -> str:
    sig = re.sub(r"\s+", " ", sig).strip()
    sig = ATTRIBUTE_MACROS.sub("", sig)
    sig = MIGRATION.sub(r"[deprecated ~v\1.\2.\3] ", sig)
    sig = re.sub(r"\b(inline|override|final)\b\s*", "", sig)
    sig = re.sub(r"\s+([;,)])", r"\1", sig).replace("( ", "(")
    # Constructor init list written with parenthesis (members always start with '_')
    sig = re.sub(r"(\)(?:\s*noexcept)?(?:\s*requires\s+[^:]*?)?)\s*:\s*_\w+\s*[({].*$", r"\1", sig)
    return sig.strip().rstrip(";").strip()

class Parser:
    def __init__(self, text: str):
        self.lines = clean_lines(text)
        self.items: list[dict] = [] # {kind, name, ns, sig, comment, members}

    def parse(self) -> list[dict]:
        self.block(0, len(self.lines), ns="", scope=None)
        return self.items

    def block(self, start: int, end: int, ns: str, scope: dict | None, access: str = "public") -> None:
        """Parse statements between lines [start, end). scope = the class being parsed (None at namespace level)."""
        i, buf, comment, template = start, "", "", ""
        while i < end:
            code, com = self.lines[i]
            i += 1
            s = code.strip()
            if not s: continue
            if scope is not None:
                m = re.match(r"^(public|private|protected)\s*:\s*(.*)$", s)
                if m and not buf:
                    access, s = m.group(1), m.group(2)
                    if not s: continue
            if not buf: comment = com
            buf = (buf + " " + s).strip()
            if re.fullmatch(r"template\s*<.*>", buf) and buf.count("<") == buf.count(">"):
                template, buf = buf, ""
                continue
            # Statement opening a body (first '{' outside of any parenthesis)
            col = self.body_brace(code, buf[:len(buf) - len(s)])
            if col is not None:
                head = (buf[:len(buf) - len(s)] + " " + code.strip()[:col - (len(code) - len(code.lstrip()))]).strip()
                close, after = self.match_brace(i - 1, col)
                # Constructor init list 'X(...): Base(), _a{a}, _b(b) {...}': the member init braces are glued
                # to an identifier, the body brace isn't -> skip until the real body
                m = re.match(r"^(.*?\)(?:\s*noexcept)?(?:\s*requires\s+.*?)?)\s*(?<!:):(?!:)\s*[\w:<>]+\s*[({].*$", head + "{")
                glued = col > 0 and bool(re.match(r"[\w>]", code[col - 1]))
                if m and self.kind_of(head) == "function":
                    head = m.group(1)
                if m and glued and self.kind_of(head) == "function":
                    line_idx = close
                    pos = len(self.lines[close][0]) - len(after)
                    while line_idx < len(self.lines):
                        found = self.next_body_brace(line_idx, pos)
                        if found is None: break
                        line_idx, col2, glued = found
                        close, after = self.match_brace(line_idx, col2)
                        if not glued: break
                        line_idx, pos = close, len(self.lines[close][0]) - len(after)
                kind = self.kind_of(head)
                if kind == "init": # brace initializer: '= {..};' or 'x{..};' -> plain statement
                    if scope is None or access == "public":
                        rest = self.text(i - 1, close)
                        stmt = (buf[:len(buf) - len(s)] + " " + rest[:rest.rfind("}") + 1]).strip()
                        self.statement(template + " " + stmt + ";", comment, ns, scope)
                    i, buf, template = close + 1, "", ""
                    continue
                if kind == "namespace":
                    name = head.split()[1] if len(head.split()) > 1 else ""
                    self.block(i, close, ns=name or ns, scope=None) if close > i - 1 else None
                    if close == i - 1: # one line namespace (migration) -> parse the content of the line
                        pass
                    i, buf, template = close + 1, "", ""
                    continue
                if kind in ("class", "struct", "union") and (scope is None or access == "public"):
                    item = {"kind": kind, "name": self.type_name(head), "ns": ns, "sig": simplify(template + " " + head),
                            "comment": comment, "members": []}
                    self.add(item, scope)
                    self.block(i, close, ns=ns, scope=item, access="public" if kind != "class" else "private")
                elif kind == "enum" and (scope is None or access == "public"):
                    body = self.text(i - 1, close)
                    values = body[body.index("{") + 1: body.rindex("}")] if "}" in body else ""
                    values = ", ".join(v.strip().split("=")[0].strip() for v in values.split(",") if v.strip())
                    self.add({"kind": "enum", "name": self.type_name(head), "ns": ns, "sig": simplify(head) + " {" + values + "}",
                              "comment": comment, "members": []}, scope)
                elif kind == "function" and (scope is None or access == "public"):
                    self.add({"kind": "function", "name": "", "ns": ns, "sig": simplify(template + " " + head),
                              "comment": comment, "members": []}, scope)
                i, buf, template = close + 1, "", ""
                if after.strip().startswith(";"): pass
                continue
            if buf.endswith(";") and buf.count("(") == buf.count(")"):
                if scope is None or access == "public":
                    self.statement(template + " " + buf, comment, ns, scope)
                buf, template = "", ""
        return

    def statement(self, stmt: str, comment: str, ns: str, scope: dict | None) -> None:
        s = simplify(stmt)
        if not s or s.startswith(("static_assert", "friend")): return
        if re.search(r"=\s*delete$", s): return
        if re.match(r"^(class|struct)\s+\w+$", s): return # forward declaration
        if s.startswith("using namespace"): return
        if scope is None and not ("(" in s or s.startswith(("using", "extern", "constexpr", "inline", "typedef"))):
            return # namespace level variable without interest
        kind = "using" if s.startswith(("using", "typedef")) else ("function" if "(" in s else "member")
        if scope is not None and kind == "member" and re.match(r"^(mutable\s+)?[\w:<>, *&]+\s+_\w+", s):
            return # private like member in a public section (shouldn't happen)
        self.add({"kind": kind, "name": "", "ns": ns, "sig": s, "comment": comment, "members": []}, scope)

    def add(self, item: dict, scope: dict | None) -> None:
        (scope["members"] if scope is not None else self.items).append(item)

    def next_body_brace(self, line_idx: int, pos: int) -> tuple[int, int, bool] | None:
        """Next '{' outside of parenthesis from (line, pos): (line, col, glued to an identifier)."""
        depth = 0
        while line_idx < len(self.lines):
            code = self.lines[line_idx][0]
            for j in range(pos, len(code)):
                ch = code[j]
                if ch == "(": depth += 1
                elif ch == ")": depth -= 1
                elif ch == "{" and depth == 0:
                    return line_idx, j, j > 0 and bool(re.match(r"[\w>]", code[j - 1]))
            line_idx, pos = line_idx + 1, 0
        return None

    @staticmethod
    def body_brace(code: str, before: str) -> int | None:
        """Column of the first '{' of `code` outside of parenthesis (`before` = previous lines of the statement)."""
        depth = before.count("(") - before.count(")")
        quote = None
        for j, ch in enumerate(code):
            if quote:
                if ch == quote: quote = None
            elif ch in "\"'": quote = ch
            elif ch == "(": depth += 1
            elif ch == ")": depth -= 1
            elif ch == "{" and depth == 0: return j
        return None

    @staticmethod
    def kind_of(head: str) -> str:
        h = re.sub(r"^template\s*<.*?>\s*", "", head)
        if re.match(r"^namespace\b", h): return "namespace"
        if re.match(r"^enum\b", h): return "enum"
        if re.match(r"^(class|struct|union)\b", h) and "(" not in h.split(":")[0]:
            return h.split()[0]
        if "(" in h: return "function"
        if re.search(r"=\s*$|\w+\s*$", h): return "init" # variable with a brace initializer
        return "other"

    @staticmethod
    def type_name(head: str) -> str:
        h = re.sub(r"^template\s*<.*?>\s*", "", head)
        m = re.match(r"^(?:enum\s+class|enum\s+struct|enum|class|struct|union)\s+(?:\[\[.*?\]\]\s*)?(?:_\w+\s+)?(\w+)", h)
        return m.group(1) if m else h

    def match_brace(self, line_idx: int, col: int) -> tuple[int, str]:
        """Return (line index of the matching '}', rest of that line)."""
        depth, i, c = 0, line_idx, col
        while i < len(self.lines):
            code = self.lines[i][0]
            j = c if i == line_idx else 0
            quote = None
            while j < len(code):
                ch = code[j]
                if quote:
                    if ch == "\\": j += 1
                    elif ch == quote: quote = None
                elif ch in "\"'": quote = ch
                elif ch == "{": depth += 1
                elif ch == "}":
                    depth -= 1
                    if depth == 0: return i, code[j + 1:]
                j += 1
            i += 1
        return len(self.lines) - 1, ""

    def text(self, a: int, b: int) -> str:
        return " ".join(self.lines[k][0] for k in range(a, b + 1))

# ---------------------------------------------------------------- #
# Rendering

SKIP_NAMES = {"requires", "static_cast", "alignas", "decltype", "noexcept", "sizeof", "deprecated", "if", "template"}

def func_name(sig: str) -> str:
    """Name of the declared function / concept (attributes and requires clauses ignored)."""
    m = re.search(r"\bconcept\s+(\w+)", sig)
    if m: return m.group(1)
    if sig.startswith(("using", "typedef")): return ""
    sig = re.sub(r"\[\[.*?\]\]|\[deprecated[^\]]*\]", "", sig)
    sig = re.sub(r"^template\s*<.*?>\s*", "", sig)
    for m in re.finditer(r"(operator\s*[^\s(]+|[A-Za-z_]\w*)\s*\(", sig):
        name = m.group(1)
        if name in SKIP_NAMES or name.startswith("_"): continue
        return name
    return ""

def render_item(item: dict, indent: str = "") -> list[str]:
    com = f" // {item['comment']}" if item["comment"] else ""
    if item["kind"] in ("class", "struct", "union"):
        out = [f"{indent}{item['sig']} {{{com}"]
        for m in item["members"]:
            out += render_item(m, indent + "    ")
        out.append(f"{indent}}};")
        return out
    end = "" if item["kind"] == "enum" else ";"
    return [f"{indent}{item['sig']}{end}{com}"]
