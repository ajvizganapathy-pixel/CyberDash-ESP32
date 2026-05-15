"""
CyberDash · PlatformIO post-build packager.

After a successful build, copies the firmware/bootloader/partitions binaries
into `dist/` and writes a `manifest.json` describing the artifact set. CI
uploads `dist/` to a GitHub Release; the web flasher reads `manifest.json`
to discover binaries and target offsets.

Wired in via platformio.ini:
    extra_scripts = post:scripts/package_release.py
"""

from __future__ import annotations

import hashlib
import json
import os
import shutil
import time
from pathlib import Path

Import("env")  # noqa: F821 — provided by PlatformIO


def _sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def _copy(src: Path, dst_dir: Path) -> dict | None:
    if not src.exists():
        return None
    dst_dir.mkdir(parents=True, exist_ok=True)
    dst = dst_dir / src.name
    shutil.copy2(src, dst)
    return {
        "file": src.name,
        "size": dst.stat().st_size,
        "sha256": _sha256(dst),
    }


def _package(source, target, env):  # noqa: ARG001
    project_dir = Path(env["PROJECT_DIR"])
    build_dir = Path(env.subst("$BUILD_DIR"))
    dist_dir = project_dir / "dist"
    dist_dir.mkdir(exist_ok=True)

    # Standard PlatformIO ESP32 outputs
    candidates = {
        "firmware":   build_dir / "firmware.bin",
        "bootloader": build_dir / "bootloader.bin",
        "partitions": build_dir / "partitions.bin",
    }

    # Standard ESP32 flash offsets (matches default Arduino partitions)
    offsets = {
        "bootloader": "0x1000",
        "partitions": "0x8000",
        "firmware":   "0x10000",
    }

    artifacts = {}
    for key, src in candidates.items():
        info = _copy(src, dist_dir)
        if info:
            info["offset"] = offsets.get(key)
            artifacts[key] = info

    if not artifacts:
        print("[package_release] No build artifacts found — skipping manifest.")
        return

    manifest = {
        "name":       "CyberDash-ESP32",
        "version":    os.environ.get("CYBERDASH_VERSION", "dev"),
        "tag":        os.environ.get("GITHUB_REF_NAME", "dev"),
        "commit":     os.environ.get("GITHUB_SHA", "local")[:7],
        "built_at":   time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "board":      env.get("BOARD"),
        "platform":   env.get("PIOPLATFORM"),
        "framework":  ", ".join(env.get("PIOFRAMEWORK") or []),
        "chip":       "esp32",
        "artifacts":  artifacts,
        "flash": [
            {"offset": offsets["bootloader"], "file": "bootloader.bin"},
            {"offset": offsets["partitions"], "file": "partitions.bin"},
            {"offset": offsets["firmware"],   "file": "firmware.bin"},
        ],
    }

    manifest_path = dist_dir / "manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2))

    print(f"[package_release] Wrote {len(artifacts)} artifact(s) -> {dist_dir}")
    for key, info in artifacts.items():
        print(f"  · {key:10s} {info['file']:18s} {info['size']:>8d} bytes  sha256={info['sha256'][:12]}…")
    print(f"[package_release] Manifest: {manifest_path}")


env.AddPostAction("buildprog", _package)  # noqa: F821
