# PLAN

Open work in priority order; completed items retire to `docs/PLAN_LOG.md`.
This repository is a fork of upstream `es`; `master` is the working branch.

## Open

- [x] Commit `share/capture.es` and `test/tests/capture.es` after a clean out-of-tree build and full suite pass (54/54) (done 2026-10-14 16:50 EDT).
- [ ] Confirm the Mechatron Prime CI result for the CI migration commit and report coverage gaps to mechatron-prime; retired CI configs stay in place, disabled (context: docs/CI.md).
- [ ] Add top-level `./build` and `./test` wrappers that run through `nix develop -c` and build out of tree (the shared checkout currently mixes macOS arm64 and Linux x86-64 objects, which breaks `make`).
- [ ] Investigate and harden against host `AWK` overrides (e.g., `frawk`) in non-Nix local workflows.
- [ ] Make `share/capture.es` run its helper with `bash -c` instead of `bash -lc`, so the login profile cannot leak stderr noise or state into captures (prove with a failing test using a noisy temporary HOME).

## Recently completed

- [x] Preserve historical fork state on `legacy-attempt-to-add-readline-and-jc` and re-point `yolo` to upstream `master` (completed 2026-02-18 09:57 EST).
- [x] Add `flake.nix` + `flake.lock` for reproducible local development (completed 2026-02-18 09:57 EST).
- [x] Expose a real Nix package/check output for `es` so CI and Garnix can build it (completed 2026-02-18 09:57 EST).
- [x] Add GitHub Actions workflow with downloadable build artifacts across Linux/macOS architectures (completed 2026-02-18 09:57 EST).
- [x] Add `garnix.yaml` and README badges for GitHub Actions + Garnix status (completed 2026-02-18 09:57 EST).
- [x] Add GitHub Actions test matrix (Linux + macOS, x86_64 + aarch64) with artifact gating (completed 2026-02-18 10:22 EST).
- [x] Harden `test/tests/wait.es` for Darwin process visibility/`ps` behavior so macOS can run the full unfiltered suite (completed 2026-02-18 10:38 EST).
- [x] Add seeded deterministic benchmark workloads (`es` vs `bash`, with optional `awk` baseline) via `./bm` + `bench/dprng.awk` (completed 2026-02-22 00:40 EST).
- [x] Add `WHY_ES.md` onboarding/sales guide with examples validated by `test/tests/why_es_doc.es` (completed 2026-02-22 00:29 EST).
