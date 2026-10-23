# Bruce Waveshare S3 2.8B

Independent **Bruce 1.16.1-ws28b-pro** derivative for the Waveshare ESP32-S3-Touch-LCD-2.8B.

**Experimental pre-release. The owner reports successful operation on their Waveshare setup as of 2026-10-23.** See the owner device report below for its scope. A successful build, host tests and a general operational report do not establish calibrated battery accuracy, electrical safety, or exhaustive per-feature validation.

## Project Overview

This project adapts [Bruce firmware](https://github.com/BruceDevices/firmware) to the Waveshare 2.8B board and adds Turkish touch input, battery diagnostics, a passive Wi-Fi analyzer, and a BadUSB script manager. It is an independent community derivative, not an official Bruce or Waveshare release. Original Bruce authors retain their copyrights and credit.

The reported upstream base is `ba519c936c87b89c9c667c15b932d7faca5360d5`; the original port snapshot is `b0dc10fd3eaf527a5ab59828c2b1125c02093c8b`. Distribution changes are recorded in [CHANGELOG.md](CHANGELOG.md) and [PUBLICATION-CHANGES.json](PUBLICATION-CHANGES.json).

## Supported Hardware

- **Waveshare ESP32-S3-Touch-LCD-2.8B**, N16R8 configuration: 16 MB flash, 8 MB OPI PSRAM.
- ST7701 RGB display and GT911 touch controller, using the existing board initialization.
- PlatformIO environment: `waveshare_28b`.

Other upstream board configurations are retained as source references; this release does not validate them. RF, NFC/RFID, IR, and other expansion functions require their respective external hardware and correct wiring. They are not built into the Waveshare display board merely because menus exist.

## Features

- Turkish Q keyboard support and Turkish touch text input.
- Touch UI improvements, bounded text widgets, and shell history.
- Battery Manager with explicit unknown/uncertain states; percentage is an estimate.
- Passive 2.4 GHz Wi-Fi Analyzer with channel/RSSI views and masked CSV/JSON export.
- BadUSB Script Manager, editor, validation, preview, device confirmation, and cancellation.
- Existing Bruce modules and original driver initialization, subject to hardware availability.

**Distribution change:** no embedded KeeLoq manufacturer-key database, AES key/IV/ciphertext, or automatic decryption fallback is included. Automatic manufacturer-key lookup without an external keystore is unavailable. The generic cipher and parser remain; users are responsible for lawful provenance of any separately supplied data.

## Installation Guide

1. Open [Releases](https://github.com/S2GtheCreator/Bruce-Waveshare-S3-2.8B/releases/tag/v1.16.1-ws28b-pro).
2. Download the merged firmware, `SHA256SUMS.txt`, and `FLASH-INSTRUCTIONS.md`.
3. Verify the SHA-256 checksum before flashing.
4. Back up the current flash and configuration, disconnect external loads, and use a suitable USB data cable.
5. Follow the instructions below. Retain a known working recovery image locally.

## Flashing Instructions

Use esptool with the correct serial port. The following commands use esptool 5 syntax; replace `PORT` with your actual port.

```sh
python -m esptool --chip esp32s3 --port PORT read-flash 0x0 0x1000000 backup.bin
python -m esptool --chip esp32s3 --port PORT write-flash 0x0 Bruce-1.16.1-ws28b-pro-merged.bin
```

**Writing the merged image at 0x0 can reset NVS settings.** It includes the bootloader at 0x0, partition table at 0x8000, and application at 0x10000. It does not include a filesystem image.

For an existing compatible partition layout, the application-only image may be written at **0x10000** to avoid writing the NVS region:

```sh
python -m esptool --chip esp32s3 --port PORT write-flash 0x10000 Bruce-1.16.1-ws28b-pro-app-0x10000.bin
```

Do not assume app-only flashing is suitable for an unknown factory or third-party partition layout. See [FLASH-INSTRUCTIONS.md](FLASH-INSTRUCTIONS.md).

## Screenshots

No verified screenshots of this Pro build running on the Waveshare board are available. No simulated device screenshots are presented as evidence. Existing artwork and upstream documentation are retained in the source tree with their original context.

## Known Issues

- A general successful-operation report is available from the owner. Separate results for display flicker, touch behavior, USB HID, Wi-Fi/BLE and long-running stability have not been documented.
- Battery voltage/percentage require calibration and physical confirmation. Automatic USB/charging-source discrimination and software power cutoff are not supported.
- No full Unicode display font, new kinetic/swipe driver, or desktop-class editor is claimed.
- Application-only updates require a compatible partition table. Merged flashing can overwrite settings.
- Embedded KeeLoq manufacturer-key fallback has been removed from this distribution.
- Inherited dependency warnings and hardware-specific license conditions remain documented in [THIRD_PARTY.md](THIRD_PARTY.md) and [LICENSE-AUDIT.md](LICENSE-AUDIT.md).

## Owner device report — 2026-10-23

The owner reports that everything developed so far is working without issues on their Waveshare setup. This is a qualitative owner-reported operational result, not an independently witnessed or instrumented test. A flashed-binary SHA-256 and separate feature-by-feature results were not recorded, so the report does not certify every feature or identify the exact tested binary. Display/touch, Windows HID, battery/ADC/power and wireless measurements remain to be documented individually.

## Tested / Untested Features

| Item | Evidence / status |
| --- | --- |
| Sanitized release build | See [BUILD-REPORT.md](BUILD-REPORT.md) for actual build results and binary validation. |
| Turkish keyboard reports, preview, storage, bounded reads, text handling, battery model | Host tests; see build report. They do not send real HID input or test electronics. |
| Owner device report, 2026-10-23 | Everything developed so far reported as working without issues; exact flashed binary and individual test cases not recorded. |
| Previous `ws28b-trq` firmware | Reported by the user as working; it is a different build. |
| Pro LCD/touch stability, Windows HID, ADC/battery/power path, Wi-Fi/BLE, RF/NFC/IR hardware | General owner report above; **separate per-feature physical results and measurements not documented.** |

Historical reports under `docs/historical/` describe the earlier build and are explicitly not certificates for this sanitized release.

## Building from Source

Install Python and PlatformIO Core. The verified toolchain versions are recorded in [BUILD-REPORT.md](BUILD-REPORT.md).

```sh
git clone https://github.com/S2GtheCreator/Bruce-Waveshare-S3-2.8B.git
cd Bruce-Waveshare-S3-2.8B
git checkout v1.16.1-ws28b-pro
pio run -e waveshare_28b
```

The build produces `.pio/build/waveshare_28b/firmware.bin` and the merged `Bruce-waveshare_28b.bin`. Internet access is normally required for dependencies. An exact dependency source snapshot and its inventory are provided with the release to preserve the sources used for this build. Extract its `waveshare_28b/` folder into `.pio/libdeps/` if rebuilding from that snapshot. Platform/framework packages are identified in the build report and inventory. This is not a promise of bit-for-bit reproducibility across machines.

## Credits

- [BruceDevices/firmware](https://github.com/BruceDevices/firmware), formerly `pr3y/Bruce`, and all Bruce contributors.
- Waveshare for the ESP32-S3-Touch-LCD-2.8B hardware and documentation.
- S2GtheCreator for this independent Waveshare distribution.
- Flipper Zero, Momentum, Arduino, Espressif, Bodmer, M5Stack, and all library authors identified in [THIRD_PARTY.md](THIRD_PARTY.md). Original source headers and copyright notices are preserved.

A separate repository with fresh sanitized history is used because a fork would keep the removed manufacturer-key material accessible through ancestor commits. The original local development history remains private and unmodified.

## License

Bruce and the derivative changes are distributed under **GNU AGPL-3.0-or-later**; see [LICENSE](LICENSE). Third-party components retain their respective licenses and conditions; see [THIRD_PARTY.md](THIRD_PARTY.md), [LICENSE-AUDIT.md](LICENSE-AUDIT.md), and `third_party/licenses/`. The AGPL label does not relicense those components or proprietary manufacturer data.

When redistributing binaries, provide their corresponding source, modifications, and build scripts and preserve the required notices. Network deployments of modified AGPL-covered software must also meet the applicable source-offer requirements.

## Disclaimer

Provided without warranty, subject to the licenses. Use only on systems you own or are explicitly authorized to test, and follow applicable law. Firmware flashing and unverified electrical configurations can damage hardware or erase data. This project is not endorsed by Bruce, Waveshare, Flipper Devices, or other named upstream authors.

## Restricted NFC components excluded

ST25R3916-fork and NFC-RFAL-fork use ST SLA0052 Rev3 terms, including clauses 4 and 5. Compatible redistribution permission for this AGPL combined work was not established, so both dependencies and the ST25R3916 driver/selection were removed. ST25R3916 SPI/I2C NFC functionality is unavailable in this release; PN532/RC522 code remains subject to its own hardware and license requirements.

## RF24 compatibility exclusion

The installed RF24 1.4.11 backend expressly uses GPL-2.0-only. No compatible permission for combining it with this AGPLv3 distribution was established, so the RF24 dependency and NRF24 implementation are excluded. NRF24 spectrum, MouseJack, NRF24 jammer, related BLE jammer backend and NRF24 JavaScript radio operations are unavailable. The menu reports exclusion; JavaScript functions throw an explicit error. The ESP32 passive Wi-Fi Analyzer does not use RF24 and is retained.
