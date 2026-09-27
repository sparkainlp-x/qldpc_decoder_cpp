# Changelog

All notable changes to this project are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and the project uses
[Semantic Versioning](https://semver.org/).

## [Unreleased]

### Fixed
- `hls/run_hls.tcl` was stored as a single line with literal `\n` sequences, so Tcl read the whole file as one comment and the script did nothing. It is now a normal multi-line script (synthesis is still UNRUN in CI).

### Changed
- Documentation is English throughout: the README's French section, `docs/deployment-zcu111.md`, `vivado/README.md`, Makefile messages, and code comments and console strings. Hardware documents carry explicit UNRUN / TARGET status notes.
- CI actions bumped to `actions/checkout@v7` and `actions/upload-artifact@v7` (Node 24).

## [0.1.1] - 2026-09-26

### Changed
- `CITATION.cff` version bump for the Zenodo archival release (DOI 10.5281/zenodo.22985528; concept DOI 10.5281/zenodo.22985527).

## [0.1.0] - 2026-09-26

### Added
- English-first README, evidence tags (the ~70 ns GF(2) micro-benchmark is REPORTED; host unspecified), MIT license, `CITATION.cff`.
- CMake + Catch2 software CI with an opt-in self-hosted hardware job.

### Removed
- Promotional drafts; `verify_correction()` limitation documented (FER is 0 by construction until replaced).
