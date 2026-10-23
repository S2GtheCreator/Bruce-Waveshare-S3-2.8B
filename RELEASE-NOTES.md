# v1.16.1-ws28b-pro — experimental Waveshare release

Independent Bruce derivative for **Waveshare ESP32-S3-Touch-LCD-2.8B N16R8**. Not an official Bruce/Waveshare release.

**Pro has not been tested on the physical board.** Actual PlatformIO build, 8 host test groups, esptool validation, binary range checks and SHA-256 checks passed; these do not certify runtime/hardware behavior.

Features: Turkish Q HID/touch input, Touch UI improvements, Battery Manager, passive 2.4 GHz Wi-Fi Analyzer, and BadUSB Script Manager/preview/device confirmation/cancel.

## License-driven exclusions

- Embedded KeeLoq manufacturer-key data, key/IV/ciphertext and automatic fallback removed because redistribution permission was not established.
- ST25R3916/RFAL dependencies and ST25R3916 SPI/I2C support removed because compatible permission for the AGPL combination was not established under the installed SLA0052 terms.
- RF24 1.4.11 GPL-2.0-only backend removed: NRF24 spectrum/MouseJack/jammer, related BLE jammer backend and NRF24 JavaScript radio operations unavailable. Internal ESP32 Wi-Fi Analyzer remains.
- Original binaries/history/development bundles/stage/recovery archives are not redistributed. This release contains newly built sanitized firmware and fresh source history.

## Downloads and flashing

- Merged image: **3,826,064 bytes**, write at **0x0**. **Can reset NVS/settings.**
- Application-only image: **3,760,528 bytes**, write at **0x10000** only with a matching existing partition layout.
- Project source ZIP plus exact patched dependency-source ZIP and original third-party notices.
- FLASH-INSTRUCTIONS.md, BUILD-REPORT.md, LICENSE-AUDIT.md and SHA256SUMS.txt.

Verify the checksums before flashing and back up your device. No filesystem image is included. See the included instructions.

Merged SHA-256: `aae350f4ccaa51ca04d956bb60a4039e4ac09b3ec5416d6d564932de8d2cb364`

## Credits and license

Based on [BruceDevices/firmware](https://github.com/BruceDevices/firmware), formerly pr3y/Bruce, with credits to all original authors. Bruce/derivative changes: AGPL-3.0-or-later. Individual libraries retain their licenses; full notices, dependency sources and the review are provided. No endorsement is implied. Use only on systems you own or are authorized to test.
