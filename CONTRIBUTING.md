# Contributing to flip-bonsai

Thanks for considering a contribution. This document covers how to get set
up locally and how changes get reviewed and merged.

## Getting started

You'll need [ufbt](https://github.com/flipperdevices/flipperzero-ufbt)
(`brew install ufbt` on macOS) and a Flipper Zero connected over USB if you
want to test on hardware.

```bash
git clone https://github.com/TFD-42/flip-bonsai.git
cd flip-bonsai
ufbt
```

## Running / testing

There's no automated test suite — this is a small, self-contained C app for
Flipper's embedded firmware. Verification is: it builds cleanly, and it
behaves correctly on-device.

```bash
ufbt              # build, fails the way any C compile error would
ufbt launch       # build, install, and launch on a connected Flipper
```

## Linting / formatting

No linter is configured. Match the existing style in `cbonsai.c`: 4-space
indentation, braces on the same line, and comments only where the *why*
isn't obvious from the code.

CI runs `ufbt` on every pull request — please make sure it builds locally
before opening one.

## A note on the ported algorithm

`set_deltas()` and `grow_branch()` are a direct port of upstream
[cbonsai](https://gitlab.com/jallbrit/cbonsai)'s `setDeltas()`/`branch()`.
If you're changing the growth logic itself (not just the Flipper-specific
rendering/bounds), consider whether the change belongs upstream instead.

## Commit conventions

No strict convention enforced — write clear, imperative commit messages.

## Pull request process

1. Fork the repository and create a branch from `main`.
2. Make your change.
3. Ensure `ufbt` builds successfully locally.
4. Open a pull request against `main` using the PR template. Describe what
   changed and why.
5. Address review feedback before merge.

## Reporting bugs / requesting features

Use the [issue templates](.github/ISSUE_TEMPLATE/) — they collect the
information needed to triage quickly.

## Code of conduct

This project follows the [Code of Conduct](CODE_OF_CONDUCT.md). By
participating, you agree to uphold it.
