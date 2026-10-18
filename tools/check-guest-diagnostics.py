#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Deliberately fault a legal guest and inspect persistent guest diagnostics."""
from pathlib import Path
import subprocess
import tempfile
root = Path(__file__).resolve().parents[1]
cache = Path((root / '.local/headless-cache').read_text().strip())
work = Path(tempfile.mkdtemp(prefix='guest-diagnostics-', dir=root / 'results'))
(work / 'user').mkdir()
print('Guest diagnostics evidence:', work, flush=True)
r = subprocess.run([str(cache / 'build/bin/eden-headless'),
                    str(root / 'build/fixture/core-diagnostic-fault.nro')],
                   cwd=work, capture_output=True, timeout=20)
(work / 'stdout.txt').write_bytes(r.stdout)
(work / 'stderr.txt').write_bytes(r.stderr)
journal = (work / 'user/log/mod-diagnostics.log').read_text(errors='replace')
assert r.returncode == 1, 'Planned guest fault must fail without a native signal crash'
assert b'EDEN_GUEST_MEMORY given back' in r.stderr, 'Guest fault must release guest memory'
for marker in ('guest-debug', 'EDEN_DIAGNOSTIC_FIXTURE', 'guest-fault', 'prefetch-abort',
               'fault_pc=0000000000000000', 'stopped_pc=', 'x1=0000000000001234', 'module base=', 'frame pc='):
    assert marker in journal, marker
print('Guest diagnostics PASS: actual guest debug output, PC/registers, module map and backtrace survive a guest fault')
