#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Bundle copied ProsperoEden diagnostic logs; game files, keys and saves are excluded."""
import argparse
import hashlib
import json
from pathlib import Path
import zipfile

def collect(root, output):
    root = root.resolve(strict=True)
    if output.exists():
        raise ValueError('Output already exists')
    files = set()
    for pattern in ('logs/**/*', 'user/log/**/*', 'user/sdmc/FsAccessLog.txt',
                    'user/sdmc/ultimate/arcropolis/logs/*.log'):
        for file in root.glob(pattern):
            if file.is_symlink() or not file.is_file():
                continue
            if not file.resolve().is_relative_to(root):
                raise ValueError('Log directory escapes input root')
            if not any(suffix in ('.log', '.txt', '.json') for suffix in file.suffixes):
                continue
            files.add(file)
    if not files:
        raise ValueError('No diagnostic logs found')
    manifest = []
    output.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(output, 'x', zipfile.ZIP_DEFLATED) as archive:
        for file in sorted(files):
            name = file.relative_to(root).as_posix()
            with file.open('rb') as source:
                digest = hashlib.file_digest(source, 'sha256').hexdigest()
            archive.write(file, name)
            manifest.append({'path': name, 'bytes': file.stat().st_size, 'sha256': digest})
        archive.writestr('diagnostics-manifest.json', json.dumps(manifest, indent=2) + '\n')
    return len(files)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('root', type=Path, help='copied /data/prosperoeden directory')
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    try:
        count = collect(args.root, args.output)
    except (OSError, ValueError) as error:
        parser.exit(1, str(error) + '\n')
    print(f'Collected {count} diagnostic files in {args.output}')
