"""Rebrand verified v0.13.9 Windows packages to JuniqOps v0.14.0.

Technical IDs, update ZIP names, module paths, Supabase identifiers, and
previous updater trust chain remain unchanged for backwards compatibility.
Only customer-visible product copy, artwork and version metadata are changed.
"""
import ast
import json
import math
import re
import struct
import zlib
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "brand-source"
OUTPUT = ROOT / "juniqops-build"
OUTPUT.mkdir(parents=True, exist_ok=True)
OLD_VERSION, VERSION = "0.13.9", "0.14.0"
PREFIX = "VisionOps_AI_Studio/"
ARCHIVES = ("VisionOps_AI_Studio_update.zip", "VisionOps_AI_Studio_Setup.zip")
TEXT_SUFFIXES = (".py", ".pyw", ".html", ".css", ".js", ".json", ".txt", ".bat", ".ps1", ".md")


def rename_visible(text: str) -> str:
    # Retain VisionOps_AI_Studio/, VisionOps.ico and any API/bridge symbol.
    text = text.replace("VisionOps AI Studio", "JuniqOps")
    text = text.replace("VisionOps AI", "JuniqOps")
    text = re.sub(r"\bVisionOps\b(?!\.ico)", "JuniqOps", text)
    text = re.sub(r"\bVISIONOPS\b", "JUNIQOPS", text)
    return text


def dist_segment(px, py, ax, ay, bx, by):
    dx, dy = bx - ax, by - ay
    t = max(0.0, min(1.0, ((px - ax) * dx + (py - ay) * dy) / max(1.0, dx * dx + dy * dy)))
    return math.hypot(px - ax - t * dx, py - ay - t * dy)


def icon_ico():
    """Make a self-contained J/Q icon with no external build dependency."""
    n = 256
    raw = bytearray()
    for y in range(n):
        raw.append(0)
        for x in range(n):
            q = math.hypot((x-156) / 50.0, (y-116) / 57.0)
            qring = abs(q - 1.0) * 51.0 < 10.0
            j = (dist_segment(x, y, 74, 62, 74, 167) < 11
                 or dist_segment(x, y, 74, 166, 59, 189) < 11
                 or dist_segment(x, y, 59, 189, 37, 190) < 11)
            tail = dist_segment(x, y, 184, 155, 220, 191) < 11
            corners = ((56 < x < 70 or 99 < x < 113) and (90 < y < 101 or 130 < y < 140))
            if j:
                rgba = (239, 247, 250, 255)
            elif qring or tail or corners:
                rgba = (36, 220, 213, 255)
            else:
                rgba = (7, 24, 39, 255)
            raw.extend(rgba)
    def chunk(tag, data):
        return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data) & 0xffffffff)
    png = (b"\x89PNG\r\n\x1a\n" +
           chunk(b"IHDR", struct.pack(">2I5B", n, n, 8, 6, 0, 0, 0)) +
           chunk(b"IDAT", zlib.compress(bytes(raw), 9)) + chunk(b"IEND", b""))
    header = struct.pack("<HHH", 0, 1, 1)
    entry = struct.pack("<BBBBHHII", 0, 0, 0, 0, 1, 32, len(png), 22)
    return header + entry + png


def repackage(name: str):
    source = SOURCE / name
    if not source.is_file():
        raise RuntimeError(f"Missing approved source archive: {source}")
    out = OUTPUT / name
    with zipfile.ZipFile(source) as zi, zipfile.ZipFile(out, "w", compression=zipfile.ZIP_DEFLATED) as zo:
        if zi.testzip():
            raise RuntimeError("Invalid base ZIP")
        names = zi.namelist()
        if not names or any(not x.startswith(PREFIX) or ".." in Path(x).parts for x in names):
            raise RuntimeError("Unsafe base ZIP entry")
        if len(set(names)) != len(names):
            raise RuntimeError("Duplicate base ZIP entry")
        if PREFIX + "version.json" not in names:
            raise RuntimeError("Missing version metadata")
        if json.loads(zi.read(PREFIX + "version.json"))["version"] != OLD_VERSION:
            raise RuntimeError("Unexpected source version")
        for item in zi.infolist():
            value = zi.read(item.filename)
            if item.filename == PREFIX + "VisionOps.ico":
                value = icon_ico()
            elif item.filename == PREFIX + "version.json":
                config = json.loads(value)
                config["version"] = VERSION
                for key in ("name", "product", "product_name", "app_name", "title"):
                    if isinstance(config.get(key), str):
                        config[key] = rename_visible(config[key])
                value = (json.dumps(config, ensure_ascii=False, indent=2) + "\n").encode("utf-8")
            elif item.filename.endswith(TEXT_SUFFIXES) and not item.is_dir():
                try:
                    decoded = value.decode("utf-8-sig")
                except UnicodeDecodeError as ex:
                    raise RuntimeError(f"Unexpected binary text file: {item.filename}") from ex
                changed = rename_visible(decoded)
                if item.filename.endswith((".py", ".pyw")):
                    ast.parse(changed, filename=item.filename)
                value = changed.encode("utf-8")
            zo.writestr(item, value)
    with zipfile.ZipFile(out) as z:
        assert z.testzip() is None
        assert json.loads(z.read(PREFIX + "version.json"))["version"] == VERSION
    print(f"Prepared {out} ({out.stat().st_size} bytes)")


if __name__ == "__main__":
    for archive in ARCHIVES:
        repackage(archive)
