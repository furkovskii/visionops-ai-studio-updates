"""Compatibility regression for the v0.14.0 JuniqOps brand migration."""
import ast
import json
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "brand-source"
DST = ROOT / "juniqops-build"
PREFIX = "VisionOps_AI_Studio/"
NAMES = ("VisionOps_AI_Studio_update.zip", "VisionOps_AI_Studio_Setup.zip")
IMMUTABLE = (
    "security.py", "secure_store.py", "update_manager.py", "requirements.txt",
)
REQUIRED = ("app.py", "version.json", "templates/index.html", "security.py", "update_manager.py")

def check(name):
    with zipfile.ZipFile(SRC / name) as old, zipfile.ZipFile(DST / name) as new:
        assert old.testzip() is None and new.testzip() is None
        assert old.namelist() == new.namelist(), "Package names/structure must remain stable"
        assert json.loads(new.read(PREFIX + "version.json"))["version"] == "0.14.0"
        for rel in REQUIRED:
            assert PREFIX + rel in new.namelist()
        for rel in IMMUTABLE:
            if PREFIX + rel in old.namelist():
                assert old.read(PREFIX + rel) == new.read(PREFIX + rel), f"Critical runtime modified: {rel}"
        for path in new.namelist():
            if path.endswith((".py", ".pyw")):
                ast.parse(new.read(path).decode("utf-8-sig"), filename=path)
        assert old.read(PREFIX + "VisionOps.ico") != new.read(PREFIX + "VisionOps.ico")
        assert len(new.read(PREFIX + "VisionOps.ico")) > 100
        return new

if __name__ == "__main__":
    for archive in NAMES:
        check(archive)
    with zipfile.ZipFile(DST / NAMES[0]) as update, zipfile.ZipFile(DST / NAMES[1]) as setup:
        for name in update.namelist():
            assert update.read(name) == setup.read(name), f"Installer/update drift: {name}"
    print("JuniqOps v0.14.0: package compatibility, security modules, version and icon OK")
