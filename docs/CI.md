# Continuous integration

Mechatron Prime CI, the project's self-hosted Nix continuous-integration
service, builds every pushed commit on `master`. It builds the flake
attributes listed in `.mechatron-prime/targets`:

- `packages.x86_64-linux.default`: the `es` binary.
- `checks.x86_64-linux.test`: builds `es`, then runs `make testrun` and the
  full `test/tests` corpus inside the Nix sandbox.

## Retired configuration kept in place

Two earlier CI configurations remain in the tree, unchanged, for reference.
Neither one runs.

- `.github/workflows/ci.yml` (GitHub Actions) is disabled in the repository's
  Actions settings. It tested on x86_64 and ARM64 Linux and on macOS and
  uploaded per-platform build artifacts.
- `garnix.yaml` configured Garnix CI, a hosted service that shut down on
  2026-07-15. With no service to read it, the file has no effect.

## Supported platforms

This fork supports Linux only for now; macOS is out of scope, a decision the
BDFN (Benevolent Dictator For Now) made on 2026-10-14. Mechatron Prime CI
builds and tests x86_64 Linux. ARM64 Linux is not yet tested, and CI does not
publish build artifacts. The flake still declares macOS outputs, but nothing
builds or tests them, so they may stop working. Re-enabling the GitHub Actions
workflow would restore macOS and ARM64 runs if that decision changes.
