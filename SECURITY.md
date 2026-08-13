# Security Policy

## Supported Versions

| Version | Supported          |
| ------- | ------------------ |
| 1.0.x   | :white_check_mark: |

## Reporting a Vulnerability

This is a small, single-binary Flipper Zero application with no network
access and no external input beyond the device's own buttons and hardware
RNG — the practical attack surface is limited. If you still find a
memory-safety issue (e.g. a stroke-buffer or recursion-depth bound being
insufficient) or another vulnerability, please report it privately rather
than opening a public issue.

- **Preferred**: use [GitHub's private vulnerability reporting](https://github.com/TFD-42/flip-bonsai/security/advisories/new) for this repository.
- **Alternative**: email amdiver42@gmail.com.

Please include:

- A description of the vulnerability and its potential impact
- Steps to reproduce, or a proof of concept if available
- Any known mitigations

We aim to acknowledge reports within 7 days and will keep you updated as the
issue is triaged and resolved. Please give us reasonable time to address the
issue before any public disclosure.

Note: this project also carries ported logic from
[cbonsai](https://gitlab.com/jallbrit/cbonsai). If you find an issue in the
shared algorithm itself (not specific to this port), consider reporting it
upstream too.
