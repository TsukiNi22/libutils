"""The changelog page, read from CHANGELOG.md (one section per version, the sub-sections Changed / Fixed / Added / Removed under it).
The entries are written in English: the French page says so and keeps them as they are."""

import re

import mdlib

LEAD = {"en": "Every version of libutils, newest first, read from `CHANGELOG.md` ([Keep a Changelog](https://keepachangelog.com/en/1.0.0/), [semantic versioning](https://semver.org/spec/v2.0.0.html)). An `(unofficial)` version has no tag: a package built at that version is not guaranteed.",
        "fr": "Chaque version de libutils, la plus récente d'abord, lue dans `CHANGELOG.md` ([Keep a Changelog](https://keepachangelog.com/en/1.0.0/), [versionnage sémantique](https://semver.org/spec/v2.0.0.html)). Une version `(unofficial)` n'a pas de tag : un paquet construit à cette version n'est pas garanti.\n\n> [!NOTE]\n> Les entrées de l'historique sont écrites en anglais, comme dans le dépôt."}

def pages(ctx: object) -> list[dict]:
    text = (ctx.site / ".." / ".." / "CHANGELOG.md").resolve().read_text(encoding="utf-8")
    body = text[text.index("\n## ") + 1:] if "\n## " in text else text
    out, version = [], ""
    for line in body.split("\n"):
        m = re.match(r"^##\s+(.*)$", line)
        if m:
            title = m.group(1).strip()
            version = mdlib.slugify(re.sub(r"\s+-\s+.*$", "", title).strip("[]")) or "version"
            title = title.replace("<none>", "unreleased")
            out.append("## %s {#%s}" % (title.replace("[", "").replace("]", ""), version))
            continue
        m = re.match(r"^(###+)\s+(.*)$", line)
        if m:
            out.append("%s %s {#%s-%s}" % (m.group(1), m.group(2).strip(), version, mdlib.slugify(m.group(2))))
            continue
        if re.match(r"^\[[^\]]+\]:\s*\S+", line) or line.strip() == "---":  # compare links and rules: not shown
            continue
        out.append(line)
    md = "\n".join(out)
    page = {"slug": "changelog", "section": "resources", "order": 10, "kind": "", "source": (ctx.site / ".." / ".." / "CHANGELOG.md").resolve()}
    page["en"] = {"title": "Changelog", "summary": "What changed in each version, newest first.", "keywords": "changelog history versions release notes added fixed changed removed", "md": LEAD["en"] + "\n\n" + md}
    page["fr"] = {"title": "Historique", "summary": "Ce qui a changé à chaque version, la plus récente d'abord.", "keywords": "changelog historique versions notes de version ajouté corrigé modifié supprimé", "md": LEAD["fr"] + "\n\n" + md}
    return [page]
