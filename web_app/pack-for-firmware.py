#!/usr/bin/env python3
"""Pack the web_app/ static files into a PROGMEM asset header for the firmware.

Scans web_app/ (the directory this script lives in) for index.html, app.css,
js/**, and assets/**. Writes each file as a PROGMEM byte array plus a routing
table into ../syslog_mon_v2e/syslog_mon_v2e/embedded_assets.h.

Run this before every firmware build that should reflect UI changes:

    cd web_app
    python pack-for-firmware.py

Then compile + upload firmware as usual.

Size ceiling: currently well under 80 KB packed. Fits in the 1.87 MB app slot
provided by PartitionScheme=min_spiffs with plenty of room.
"""

import os
import sys
from pathlib import Path

WEB_APP_DIR = Path(__file__).parent
OUTPUT = WEB_APP_DIR.parent / "syslog_mon_v2e" / "syslog_mon_v2e" / "embedded_assets.h"

# Files to include and their URL mappings.
# Each entry: (repo-relative path, URL path, MIME type)
INCLUDES = []

MIME_BY_EXT = {
    ".html": "text/html; charset=utf-8",
    ".css":  "text/css; charset=utf-8",
    ".js":   "application/javascript; charset=utf-8",
    ".json": "application/json",
    ".png":  "image/png",
    ".jpg":  "image/jpeg",
    ".jpeg": "image/jpeg",
    ".svg":  "image/svg+xml",
    ".ico":  "image/x-icon",
    ".map":  "application/json",
}


def c_symbol(rel_path: str) -> str:
    """Make a valid C identifier from a path like 'js/ui/tab-connection.js'."""
    s = rel_path.replace("/", "_").replace("\\", "_").replace(".", "_").replace("-", "_")
    return "asset_" + s


def read_bytes(p: Path) -> bytes:
    return p.read_bytes()


def format_bytes_as_c(data: bytes) -> str:
    # 16 bytes per line. Cheap and readable.
    out = []
    for i in range(0, len(data), 16):
        chunk = data[i:i + 16]
        hex_values = ", ".join(f"0x{b:02x}" for b in chunk)
        out.append("    " + hex_values + ",")
    return "\n".join(out)


def collect_files():
    """Walk web_app/ and collect serveable files."""
    files = []

    # Root-level index.html
    idx = WEB_APP_DIR / "index.html"
    if idx.exists():
        files.append((idx.relative_to(WEB_APP_DIR), "/", "text/html; charset=utf-8"))
        files.append((idx.relative_to(WEB_APP_DIR), "/index.html", "text/html; charset=utf-8"))

    # app.css
    css = WEB_APP_DIR / "app.css"
    if css.exists():
        files.append((css.relative_to(WEB_APP_DIR), "/app.css", MIME_BY_EXT[".css"]))

    # js/** and assets/**
    for sub in ("js", "assets"):
        root = WEB_APP_DIR / sub
        if not root.exists():
            continue
        for f in sorted(root.rglob("*")):
            if f.is_dir():
                continue
            ext = f.suffix.lower()
            mime = MIME_BY_EXT.get(ext)
            if mime is None:
                # Skip unknown types (e.g. .py, .md)
                continue
            rel = f.relative_to(WEB_APP_DIR)
            url = "/" + str(rel).replace("\\", "/")
            files.append((rel, url, mime))

    return files


def main():
    files = collect_files()
    if not files:
        print("No files found to pack", file=sys.stderr)
        sys.exit(1)

    # Deduplicate content (index.html is listed twice for / and /index.html).
    # Build symbol table keyed by the file path.
    by_path = {}
    for rel, url, mime in files:
        key = str(rel).replace("\\", "/")
        if key not in by_path:
            data = read_bytes(WEB_APP_DIR / rel)
            by_path[key] = {
                "rel":    key,
                "sym":    c_symbol(key),
                "data":   data,
                "size":   len(data),
                "routes": [],
            }
        by_path[key]["routes"].append((url, mime))

    total = sum(entry["size"] for entry in by_path.values())

    # Render the header.
    with open(OUTPUT, "w", encoding="utf-8", newline="\n") as fp:
        fp.write("// GENERATED FILE — do not edit by hand.\n")
        fp.write("// Run `python web_app/pack-for-firmware.py` to regenerate.\n")
        fp.write(f"// Total packed size: {total} bytes across {len(by_path)} files.\n\n")
        fp.write("#ifndef IDS_EMBEDDED_ASSETS_H\n")
        fp.write("#define IDS_EMBEDDED_ASSETS_H\n\n")
        fp.write("#include <pgmspace.h>\n")
        fp.write("#include <stddef.h>\n")
        fp.write("#include <stdint.h>\n\n")

        # Emit PROGMEM arrays.
        for entry in by_path.values():
            fp.write(f"// {entry['rel']} ({entry['size']} bytes)\n")
            fp.write(f"const uint8_t {entry['sym']}[] PROGMEM = {{\n")
            fp.write(format_bytes_as_c(entry["data"]))
            fp.write("\n};\n\n")

        # Emit the route table.
        fp.write("struct EmbeddedAsset {\n")
        fp.write("    const char*    path;\n")
        fp.write("    const char*    mime;\n")
        fp.write("    const uint8_t* data;\n")
        fp.write("    size_t         size;\n")
        fp.write("};\n\n")

        fp.write("const EmbeddedAsset EMBEDDED_ASSETS[] PROGMEM = {\n")
        for entry in by_path.values():
            for url, mime in entry["routes"]:
                fp.write(f'    {{ "{url}", "{mime}", {entry["sym"]}, sizeof({entry["sym"]}) }},\n')
        fp.write("};\n\n")
        fp.write("const size_t EMBEDDED_ASSETS_COUNT =\n")
        fp.write("    sizeof(EMBEDDED_ASSETS) / sizeof(EMBEDDED_ASSETS[0]);\n\n")

        fp.write("#endif\n")

    route_count = sum(len(e["routes"]) for e in by_path.values())
    print(f"Wrote {OUTPUT.relative_to(WEB_APP_DIR.parent)}")
    print(f"  {len(by_path)} files, {route_count} routes, {total:,} bytes total")
    for entry in by_path.values():
        paths = ", ".join(r[0] for r in entry["routes"])
        print(f"  {entry['size']:>7,} B  {paths}")


if __name__ == "__main__":
    main()
