# Flip Bonsai — Flipper Zero

[![CI](https://github.com/TFD-42/flip-bonsai/actions/workflows/ci.yml/badge.svg)](https://github.com/TFD-42/flip-bonsai/actions/workflows/ci.yml)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)

A port of [cbonsai](https://gitlab.com/jallbrit/cbonsai) to Flipper Zero.

<img width="3752" height="1844" alt="1000730019" src="https://github.com/user-attachments/assets/01eb7aea-36cb-4f11-bb54-76c9a664ea15" />

Same recursive growth algorithm as the original (trunk / left-shoot /
right-shoot / dying / dead branch types, driven by `life` and `multiplier`),
ported line-for-line from `setDeltas()` and `branch()`. The only real
adaptation is the renderer: cbonsai draws ASCII glyphs on a terminal
character grid, but the Flipper's 128x64 screen is a monochrome pixel
canvas, so wood steps are drawn as short connected line segments and
leaves as single dots/discs instead of characters.

## Attribution & license

This is a derivative work of [cbonsai](https://gitlab.com/jallbrit/cbonsai)
by jallbrit and contributors, which is licensed GPL-3.0-or-later. Because
the core growth algorithm is ported directly, this project is licensed
**GPL-3.0-or-later** too — see [LICENSE](LICENSE) — not a permissive
license, to stay compliant with the upstream terms.

## Installation

Copy `dist/cbonsai.fap` to `SD Card/apps/Misc/` (via qFlipper), or with the
Flipper connected over USB:

```bash
ufbt launch
```

## Controls

| Screen | Action |
|---|---|
| Menu | Up/Down to select, OK to confirm |
| Tree | OK: grow a new random tree, Back: return to menu |
| Anywhere | Long Back: quit |

## Build

```bash
ufbt          # compile → dist/cbonsai.fap
```

Requires [ufbt](https://github.com/flipperdevices/flipperzero-ufbt)
(`brew install ufbt`).

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md).

## Project status

See [STATUS.md](STATUS.md).

## Security

See [SECURITY.md](SECURITY.md) to report a vulnerability.
