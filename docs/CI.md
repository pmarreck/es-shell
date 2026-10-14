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

## Coverage gaps

Mechatron Prime CI tests x86_64 Linux only. Until other runners exist,
ARM64 Linux and macOS are neither built nor tested by CI, and no build
artifacts are published. The flake still declares outputs for
`aarch64-linux`, `aarch64-darwin` and `x86_64-darwin`.
