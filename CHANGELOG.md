# Changelog

## Documentation follow-up — 2026-10-23

- Recorded the owner's report that everything developed so far is working without issues on the Waveshare setup.
- Kept the distinction between a general owner report and individually documented physical tests/calibrated measurements; the exact tested binary was not recorded.
- No firmware, release tag or checksum changes.

## v1.16.1-ws28b-pro — 2026-10-23

Experimental community distribution for Waveshare ESP32-S3-Touch-LCD-2.8B.

### Waveshare Pro changes

- Turkish Q HID support and Turkish touch input.
- Touch text widgets, shell history and bounded rendering.
- Battery Manager with unknown/uncertain state and estimated percentage.
- Passive 2.4 GHz Wi-Fi Analyzer with masked exports.
- BadUSB Script Manager, validation, preview, device confirmation and cancellation.

### Publication changes

- Removed embedded KeeLoq manufacturer database, AES key/IV/ciphertext, generator and automatic decryption fallback because redistribution rights could not be established from available evidence.
- Kept generic cipher/parser code and all applicable upstream attribution. Automatic manufacturer lookup now requires a separately supplied lawful external keystore.
- Created fresh sanitized history instead of retaining removed data in fork ancestors.
- Removed upstream automatic release/stale-issue workflows and local editor files from this distribution.
- Added English project documentation, third-party notices, dependency-source inventory, build evidence, release checksums and clear test limitations.

### Limitations

Pro firmware remains untested on a physical board. Host tests and compilation do not certify display stability, battery accuracy, power behavior, real HID delivery or wireless/radio performance. Automatic power-source discrimination and software power cutoff are not supported.

Original pre-publication reports are retained in `docs/historical/` with explicit scope labels.

## Restricted NFC components excluded

ST25R3916-fork and NFC-RFAL-fork use ST SLA0052 Rev3 terms, including clauses 4 and 5. Compatible redistribution permission for this AGPL combined work was not established, so both dependencies and the ST25R3916 driver/selection were removed. ST25R3916 SPI/I2C NFC functionality is unavailable in this release; PN532/RC522 code remains subject to its own hardware and license requirements.

## RF24 compatibility exclusion

The installed RF24 1.4.11 backend expressly uses GPL-2.0-only. No compatible permission for combining it with this AGPLv3 distribution was established, so the RF24 dependency and NRF24 implementation are excluded. NRF24 spectrum, MouseJack, NRF24 jammer, related BLE jammer backend and NRF24 JavaScript radio operations are unavailable. The menu reports exclusion; JavaScript functions throw an explicit error. The ESP32 passive Wi-Fi Analyzer does not use RF24 and is retained.
