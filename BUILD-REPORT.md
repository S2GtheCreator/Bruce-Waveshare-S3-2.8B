# Sanitized release build report

Date: 2026-10-23. Target: waveshare_28b. Result: **SUCCESS**. The final PlatformIO run completed in 183.39 seconds; cached compilation units were reused where unchanged and the firmware was linked again. This is an actual build, not a syntax-only check. The earlier pre-publication binaries are not redistributed.

PlatformIO Core 6.2.0; pioarduino platform 55.3.39; Arduino ESP32 3.3.9; ESP-IDF libraries 5.5.4; Xtensa GCC 14.2.0+20260121. Windows toolchain alias fallback uses the same installed unified compiler and ESP32-S3 dynconfig, as documented in boards/waveshare_28b/native_toolchain.py. Exact library versions/commits: DEPENDENCIES.json and the dependency-source release snapshot.

| Output | Bytes | SHA-256 |
| --- | ---: | --- |
| Application at 0x10000 | 3760528 | `5e36976e2e09697c8b3afe5fe38a97bcd9485df698bef3af2e10e1c75fdab76e` |
| Merged image at 0x0 | 3826064 | `aae350f4ccaa51ca04d956bb60a4039e4ac09b3ec5416d6d564932de8d2cb364` |

The application fits the 0x470000-byte partition. Bootloader 0x0, partition table 0x8000 and application 0x10000 ranges match their separately generated files byte for byte. The merged image's 0x9000–0xEFFF region is erased padding, so writing the full image can reset NVS. No filesystem image is included. esptool 5.3.0 image-info validated bootloader and application checksum/hash. Embedded Arduino/IDF app-descriptor dates are upstream framework metadata and are not used as the date of this build.

The removed KeeLoq AES key, IV and a ciphertext prefix were searched in the actual application/merged bytes and were absent. The link map contains no excluded RF24, ST25R3916-fork or NFC-RFAL-fork archives/classes or KeeLoq decryption fallback.

All **8 host test groups passed**: UI/UTF-8 bounds and original hardware/HID source preservation, 8 safe-save fault cases, 22 preview dry-run cases, battery model, 11 network-text cases, 20 script-limit cases, 11 bounded-read cases, and 447 Turkish/US keyboard report cases. The preservation test explicitly excludes THIRD_PARTY.md because its publication notices were intentionally edited; preserved driver/keyboard source hashes were not refreshed to hide code changes.

The build retains inherited warnings, including macro redefinitions and toolchain LTO notes. Successful compilation does not prove runtime correctness. **No physical Pro flashing, boot, LCD/touch, Windows HID, battery/ADC/power, Wi-Fi/BLE or expansion hardware test was performed.**

License-driven exclusions and feature effects: LICENSE-AUDIT.md. Machine-readable checks: RELEASE-VALIDATION.json. Historical reports describe different earlier binaries.
