# Changelog

All notable changes to CyberDash-ESP32. Format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/);
versioning follows [SemVer](https://semver.org/).

## [Unreleased]

### Added
- GitHub Actions: `build.yml` (push/PR validation), `release.yml` (tag → GitHub Release with `firmware.bin` / `bootloader.bin` / `partitions.bin` / `manifest.json`), `pages.yml` (Pages deploy).
- `scripts/package_release.py` — PlatformIO post-build packager. Copies binaries into `dist/` and writes `manifest.json` (sha256, sizes, flash offsets).
- `version.h` with CI-injected `CYBERDASH_BUILD_TAG` / `CYBERDASH_BUILD_SHA`.
- `secrets.example.h` template and `.gitignore`d `secrets.h` — credentials no longer baked into release binaries.
- `flasher.html` — Web Serial firmware installer (GitHub Releases fetch + local `.bin` upload).
- `index.html` — Pages landing site.

### Changed
- `config.h` now sources Wi-Fi / OTA credentials from `secrets.h` (or `secrets.example.h` as fallback with a build warning).

### Security
- Removed hardcoded Wi-Fi credentials and OTA password from `config.h` in the working tree. **Earlier commits still contain them in git history — rotate those credentials and consider a history rewrite if the repo is public.**

## [1.0.0] — TBD

Initial release. See README for features.
