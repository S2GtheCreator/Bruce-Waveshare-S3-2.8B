"""Validate the locally built Waveshare release without connecting to hardware."""
from pathlib import Path
import hashlib
import json
import re

root = Path(__file__).resolve().parents[1]
build = root / '.pio/build/waveshare_28b'
merged = (root / 'Bruce-waveshare_28b.bin').read_bytes()
app = (build / 'firmware.bin').read_bytes()
for name, offset in [('bootloader.bin', 0), ('partitions.bin', 0x8000), ('firmware.bin', 0x10000)]:
    data = (build / name).read_bytes()
    assert merged[offset:offset + len(data)] == data, name
assert len(app) < 0x470000
assert len(merged) == len(app) + 0x10000
assert merged[0x9000:0xf000] == b'\xff' * 0x6000
mapping = (build / 'firmware.map').read_text(encoding='utf-8', errors='replace')
for name in ['libRF24.a', 'libST25R3916-fork.a', 'libNFC-RFAL-fork.a', 'keeloq_embedded_plaintext']:
    assert name not in mapping, name
assert not re.search(r'(?<![A-Za-z0-9_])RF24::', mapping)
for name in ['src/modules/rf/protocols/rf_keeloq_mfcodes_data.h', 'src/modules/rfid/ST25R3916.cpp', 'tools/gen_mfcodes.py']:
    assert not (root / name).exists(), name
print(json.dumps({'merged_bytes': len(merged), 'merged_sha256': hashlib.sha256(merged).hexdigest(),
                  'app_bytes': len(app), 'app_sha256': hashlib.sha256(app).hexdigest(),
                  'offsets_and_restricted_backend_checks': 'PASS', 'hardware_tested': False}, indent=2))
