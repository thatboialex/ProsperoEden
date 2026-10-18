#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
import zipfile
spec = importlib.util.spec_from_file_location('collector', Path(__file__).with_name('collect-smash-diagnostics.py'))
module = importlib.util.module_from_spec(spec); spec.loader.exec_module(module)
class CollectionTests(unittest.TestCase):
    def test_logs_only_and_existing_output_untouched(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary) / 'data'; root.mkdir()
            for name in ('logs/mod-diagnostics.log', 'logs/crash-test.txt', 'logs/crash-test-mod-diagnostics.log.prev', 'user/log/eden_log.txt', 'user/log/svc_break/trace.json', 'user/sdmc/FsAccessLog.txt',
                         'user/sdmc/ultimate/arcropolis/logs/arc.log', 'keys/prod.keys', 'user/save/private.txt'):
                file = root / name; file.parent.mkdir(parents=True, exist_ok=True); file.write_text(name)
            output = Path(temporary) / 'logs.zip'
            self.assertEqual(module.collect(root, output), 7)
            with zipfile.ZipFile(output) as archive:
                self.assertIsNone(archive.testzip())
                self.assertFalse(any('keys/' in name or 'save/' in name for name in archive.namelist()))
                self.assertEqual(len(json.loads(archive.read('diagnostics-manifest.json'))), 7)
            old = output.read_bytes()
            with self.assertRaises(ValueError): module.collect(root, output)
            self.assertEqual(output.read_bytes(), old)
    def test_external_directory_symlink_rejected(self):
        with tempfile.TemporaryDirectory() as temporary:
            base = Path(temporary); root = base / 'data'; root.mkdir()
            outside = base / 'outside'; outside.mkdir(); (outside / 'secret.log').write_text('private')
            (root / 'logs').symlink_to(outside, target_is_directory=True)
            with self.assertRaises(ValueError): module.collect(root, base / 'logs.zip')
if __name__ == '__main__': unittest.main()
