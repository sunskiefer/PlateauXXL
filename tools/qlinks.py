#!/usr/bin/env python3
"""Every page's Q-Links, from where its controls sit on the screen.

    python3 tools/qlinks.py --write    rewrite every `qlinks "PAGE" = ...` line (and qlinks_track) in layout.conf
    python3 tools/qlinks.py --check    fail if a line differs from what --write would make (build.sh, run_tests.sh)

Rules (asked for after the first Force test, 2026-10-08):
  - every control on a page (knobs, switches, buttons, option lists, popups) is on a Q-Link;
  - Q-Links run in reading order: the top row left to right, then the next row, and so on (Q-Link 1, 2, 3, ...);
    a row is the controls whose centre is within ROW_PX of the row's first (highest) one;
  - a page has at most 16 Q-Links (MPC: two knob banks of 8). A screen with more controls is shown on several
    Q-Link sub-pages, each given its rows by a `#@qrows "PAGE" = 1-2` line just before its qlinks line;
  - qlinks_track (the Q-Links outside page-follow mode) = the first page's.
A sub-page's controls are its tab's lines with no banks=, or with that page in banks=.
"""
import os
import re
import shlex
import sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
LAYOUT = os.path.join(ROOT, "layout.conf")
CONTROLS = ("knob", "toggle", "button", "enum_h", "enum_v", "popup", "slider_v", "slider_h", "stepper", "menu")
ROW_PX = 50
MAX = 16


def attrs(text):
    out = {}
    for tok in shlex.split(text):
        if "=" in tok:
            k, v = tok.split("=", 1)
            out[k] = v
    return out


def plan(lines):
    """-> {line index: new text}, and errors"""
    tabs, cur = [], None
    for i, raw in enumerate(lines):
        line = raw.strip()
        m = re.match(r"\[tab (.+)\]$", line)
        if m:
            cur = {"name": m.group(1), "controls": [], "pages": [], "rows": {}}
            tabs.append(cur)
            continue
        if cur is None:
            continue
        m = re.match(r'#@qrows "(.+)" = (\d+)(?:-(\d+))?$', line)
        if m:
            cur["rows"][m.group(1)] = (int(m.group(2)), int(m.group(3) or m.group(2)))
            continue
        m = re.match(r'qlinks "(.+)" = ', line)
        if m:
            cur["pages"].append((i, m.group(1)))
            continue
        kind, _, rest = line.partition(" ")
        if kind in CONTROLS:
            a = attrs(rest)
            b = set(a["banks"].split("|")) if a.get("banks") else None
            cur["controls"].append((a["key"], int(a["cx"]), int(a["cy"]), b))
    out, errors, first = {}, [], None
    for t in tabs:
        covered = set()
        for i, page in t["pages"]:
            ctl = [c for c in t["controls"] if c[3] is None or page in c[3]]
            ctl.sort(key=lambda c: (c[2], c[1]))
            rows = []
            for c in ctl:
                if rows and c[2] - rows[-1][0][2] <= ROW_PX:
                    rows[-1].append(c)
                else:
                    rows.append([c])
            lo, hi = t["rows"].get(page, (1, len(rows)))
            keys = [c[0] for row in rows[lo - 1:hi] for c in sorted(row, key=lambda c: c[1])]
            if len(keys) > MAX:
                errors.append("%s / %s: %d controls, more than %d Q-Links: split it with #@qrows" % (t["name"], page, len(keys), MAX))
            covered.update(keys)
            out[i] = 'qlinks "%s" = %s' % (page, ",".join(keys))
            if first is None:
                first = ",".join(keys)
        missing = [c[0] for c in t["controls"] if c[0] not in covered]
        if missing:
            errors.append("%s: not on any Q-Link: %s" % (t["name"], ", ".join(missing)))
    for i, raw in enumerate(lines):
        if raw.startswith("qlinks_track"):
            out[i] = "qlinks_track = " + (first or "")
    return out, errors


def main():
    mode = sys.argv[1] if len(sys.argv) > 1 else "--check"
    lines = open(LAYOUT).read().split("\n")
    new, errors = plan(lines)
    for e in errors:
        print("qlinks: " + e, file=sys.stderr)
    if errors:
        sys.exit(1)
    changed = [i for i, text in new.items() if lines[i] != text]
    if mode == "--write":
        for i, text in new.items():
            lines[i] = text
        open(LAYOUT, "w").write("\n".join(lines))
        print("qlinks: %d lines written (%d changed)" % (len(new), len(changed)))
    else:
        if changed:
            for i in changed:
                print("qlinks: layout.conf:%d is not in reading order; run tools/qlinks.py --write\n  have: %s\n  want: %s"
                      % (i + 1, lines[i], new[i]), file=sys.stderr)
            sys.exit(1)
        print("qlinks: every control on a Q-Link, in reading order (%d pages)" % (len(new) - 1))


if __name__ == "__main__":
    main()
