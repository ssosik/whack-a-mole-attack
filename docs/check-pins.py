#!/usr/bin/env python3
"""Verify docs/wiring.html's pin table still matches Config.h.

The document states that its pin assignments are mirrored from Config.h. Across
three pin remaps they drifted every time, each leaving a stale number in a table
that reads as authoritative -- which is worse than no table. This fails the
build instead of waiting for a reader to notice.

Checks only the facts that exist in both places. Run via `make check-pins`.
"""
import re
import sys
from pathlib import Path

CFG = Path("firmware/WhackAMole/Config.h")
DOC = Path("docs/wiring.html")


def config_pins(src):
    def arr(name):
        m = re.search(name + r"\[\w+\]\s*=\s*\{([^}]*)\}", src)
        if not m:
            sys.exit(f"could not find {name} in {CFG}")
        return [v.strip() for v in m.group(1).split(",")]

    m = re.search(r"kPixelDataPin\s*=\s*(\d+)", src)
    if not m:
        sys.exit(f"could not find kPixelDataPin in {CFG}")
    return arr("kP1ButtonPins"), arr("kP2ButtonPins"), m.group(1)


def doc_cell(doc, label):
    m = re.search(r"<tr><td>" + re.escape(label) + r"</td><td class=\"m\">([^<]*)</td>", doc)
    if not m:
        sys.exit(f"could not find the {label!r} row in {DOC}")
    return m.group(1).strip()


def main():
    p1, p2, pixel = config_pins(CFG.read_text())
    doc = DOC.read_text()

    # Config stores six pins per player: five caps then start.
    expect = {
        "Pixel data": pixel,
        "P1 buttons \u2014 B R W G Y": ", ".join(p1[:5]),
        "P1 start": p1[5],
        "P2 buttons \u2014 B R W G Y": ", ".join(p2[:5]),
        "P2 start": p2[5],
    }

    bad = []
    for label, want in expect.items():
        got = doc_cell(doc, label)
        if got != want:
            bad.append(f"  {label}: doc says {got!r}, Config.h says {want!r}")

    # The level shifter table names the pixel pin a second time.
    m = re.search(r"<td>Teensy pin <b>(\d+)</b>", doc)
    if not m:
        bad.append("  level shifter table: could not find the 'Teensy pin <n>' cell")
    elif m.group(1) != pixel:
        bad.append(
            f"  level shifter 1A: doc says pin {m.group(1)}, kPixelDataPin is {pixel}"
        )

    if bad:
        print("docs/wiring.html has drifted from Config.h:\n" + "\n".join(bad))
        return 1
    print(f"pins OK — P1 {p1}  P2 {p2}  pixel {pixel}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
