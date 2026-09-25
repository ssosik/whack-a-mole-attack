#!/usr/bin/env python3
"""Wrap docs/wiring.html (an Artifact fragment) into a standalone print document.

The artifact platform supplies the <!doctype>/<head>/<body> skeleton at publish
time, so the source file is a fragment. Printing it needs a real document, a
forced light palette (a PDF has no viewer theme), and page-break rules.
"""
import pathlib, sys

src = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else "docs/wiring.html")
dst = pathlib.Path(sys.argv[2] if len(sys.argv) > 2 else "build/wiring-print.html")

frag = src.read_text()
i = frag.index("</style>") + len("</style>")
head, body = frag[:i], frag[i:]

PRINT_CSS = """
<style>
:root, :root[data-theme="dark"] {
  --bg:#FFFFFF; --surface:#FFFFFF; --surface-2:#F4F6F8;
  --ink:#14191E; --ink-2:#414C56; --ink-3:#6B7681; --rule:#C9D1D8;
  --v5:#B8342B; --gnd:#232930; --data:#0B7454; --sw:#23598F;
  --warn:#8A5000; --warn-bg:#FBF2E0; --warn-rule:#D9B06A;
}
@page { size: A4 portrait; margin: 15mm 14mm; }
html, body { background:#FFFFFF; }
.wrap { max-width:none; padding:0; }
.scroll { overflow:visible; }
svg { min-width:0; width:100%; }
.tw { overflow:visible; }
figure, table, .rule-box, .warn { break-inside:avoid; }
tr, ol.steps li { break-inside:avoid; }
.sechead, h1, h2, h3 { break-after:avoid; }
thead { display:table-header-group; }
h1 { font-size:40px; }
body { font-size:11pt; line-height:1.5; }
section { margin-top:26px; }
figcaption, p, ol.steps li { max-width:none; }
.legend { font-size:10px; }
</style>
"""

FOOT = """
<p style="margin-top:36px;padding-top:10px;border-top:1px solid var(--rule);
   font-family:var(--mono);font-size:9.5px;color:var(--ink-3)">
  Whack-A-Mole Attack &middot; harness reference &middot; values mirrored from firmware/WhackAMole/Config.h
</p>
"""

body = body.rstrip()
assert body.endswith("</div>"), "fragment should end with the wrapper div"
doc = ('<!doctype html>\n<html lang="en">\n<head>\n<meta charset="utf-8">\n'
       '<meta name="viewport" content="width=device-width, initial-scale=1">\n'
       + head + PRINT_CSS + '\n</head>\n<body>\n'
       + body[:-len("</div>")] + FOOT + "</div>\n</body>\n</html>\n")

dst.parent.mkdir(parents=True, exist_ok=True)
dst.write_text(doc)
print(f"{dst} ({len(doc)} bytes)")
