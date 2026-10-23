# License and distribution review

Review date: 2026-10-23. Scope: this sanitized Waveshare distribution, its source snapshot, and the final release binary. This is a technical source/license review, not a legal opinion or a guarantee about every jurisdiction.

## Outcome

Bruce's AGPL-3.0-or-later license and original notices are retained. Derivative changes use the same license. Third-party licenses are preserved separately, rather than represented as having been relicensed wholesale under AGPL. Corresponding project source, build scripts, exact dependency source snapshot, version/commit inventory, and notices accompany the release.

Clear conflicts or data-rights uncertainties were resolved by removal:

| Excluded content | Evidence and reason | Effect |
| --- | --- | --- |
| Embedded KeeLoq manufacturer-key database, AES key/IV/ciphertext, generator and automatic decryption fallback | Upstream THIRD_PARTY.md expressly distinguishes proprietary manufacturer data from code licensing. Independent redistribution permission was not established. | No built-in manufacturer-key fallback. Generic cipher/parser and independently supplied lawful external keystore support remain. |
| ST25R3916-fork and NFC-RFAL-fork, plus ST25R3916 driver/selection | Installed SLA0052 Rev3, clauses 4 and 5 impose hardware-use and open-source licensing restrictions; no compatible additional permission was established for this AGPL combination. | ST25R3916 SPI/I2C NFC support excluded. PN532/RC522 code remains. |
| RF24 1.4.11 backend and dependent NRF24 implementations | Actual package metadata says GPL-2.0-only; RF24.cpp/RF24.h explicitly specify GPL version 2, without an or-later grant. The historical upstream inventory's GPL-2.0/MIT shorthand is not used as a relicensing permission. | NRF24 spectrum, MouseJack, NRF24 jammer, RF24-based BLE jammer and NRF24 JS radio operations unavailable. Menu/API report exclusion. ESP32 Wi-Fi Analyzer retained. |
| Original history, original firmware, development bundle and recovery/stage archives | They predate these removals and retain excluded material. | Not published. Fresh sanitized history and newly built firmware are used. |

## Retained dependency notices

The table identifies the top-level grants observed in actual local sources. Full original texts, source headers and nested notices are in `third_party/licenses/`; nested components retain their own terms. Exact versions and Git commits are recorded in `DEPENDENCIES.json`. Common GPLv3/LGPL license texts are also supplied for packages whose grant is in a source header.

| Component | Build version | Observed license / grant | Evidence |
| --- | --- | --- | --- |
| Adafruit BusIO | 1.17.2-bruce.1 | MIT | `third_party/licenses/Adafruit BusIO/LICENSE` |
| Adafruit PN532 | 1.3.3-bruce.1 | BSD notice | `third_party/licenses/Adafruit PN532/license.txt` |
| Adafruit Si4713 Library | 1.2.3 | BSD notice | `third_party/licenses/Adafruit Si4713 Library/license.txt` |
| AnimatedGIF | 2.2.3 | Apache-2.0 | `third_party/licenses/AnimatedGIF/LICENSE` |
| ArduinoJson | 7.4.3 | MIT | `third_party/licenses/ArduinoJson/LICENSE.txt` |
| AsyncTCP | 3.5.0 | LGPL-3.0 | `third_party/licenses/AsyncTCP/LICENSE` |
| BER-TLV | 0.0.0+20261008190710 | MIT | `third_party/licenses/BER-TLV/LICENSE` |
| ESP Amiibolink | 0.0.2 | AGPL-3.0 | `third_party/licenses/ESP Amiibolink/LICENSE` |
| ESP Chameleon Ultra | 0.0.2 | AGPL-3.0 | `third_party/licenses/ESP Chameleon Ultra/LICENSE` |
| ESP PN32BLE | 0.0.2 | AGPL-3.0 | `third_party/licenses/ESP PN32BLE/LICENSE` |
| ESP PN532 UART | 0.0.2 | AGPL-3.0 | `third_party/licenses/ESP PN532 UART/LICENSE` |
| ESP PN532Killer | 0.0.7 | AGPL-3.0 | `third_party/licenses/ESP PN532Killer/LICENSE` |
| ESP32-PSRamFS | 1.0.6 | MIT | `third_party/licenses/ESP32-PSRamFS/LICENSE` |
| ESP32Time | 2.0.6 | MIT | `third_party/licenses/ESP32Time/LICENSE` |
| ESP8266Audio | 2.4.1 | GPL-3.0-or-later wrappers; mixed nested licenses | `third_party/licenses/ESP8266Audio/LICENSE` |
| ESP8266SAM | 1.1.0 | GPL-3.0-or-later source header | `third_party/licenses/ESP8266SAM/SOURCE-HEADERS.txt` |
| ESPAsyncWebServer | 3.12.1 | LGPL-3.0 | `third_party/licenses/ESPAsyncWebServer/LICENSE` |
| FastLED | 3.10.6 | MIT and bundled notices | `third_party/licenses/FastLED/LICENSE` |
| FFT | 0.0.1 | MIT-style grant | `third_party/licenses/FFT/LICENSE` |
| IRremoteESP8266 | 2.8.6 | LGPL-2.1 | `third_party/licenses/IRremoteESP8266/LICENSE.txt` |
| JPEGDecoder | 2.0.0 | picojpeg permissive notice; see original text | `third_party/licenses/JPEGDecoder/license.txt` |
| LibSSH-ESP32 | 5.10.0 | LGPL-2.1 and embedded component notices | `third_party/licenses/LibSSH-ESP32/COPYING` |
| mquickjs | 0.0.6 | MIT | `third_party/licenses/mquickjs/LICENSE` |
| NimBLE-Arduino | 2.5.1 | Apache-2.0 and bundled notices | `third_party/licenses/NimBLE-Arduino/LICENSE` |
| NTPClient | 3.2.1 | MIT in NTPClient.cpp header | `third_party/licenses/NTPClient/SOURCE-HEADERS.txt` |
| OneWire | 2.3.8 | permissive source-header grant | `third_party/licenses/OneWire/SOURCE-HEADERS.txt` |
| PNGdec | 1.1.6 | Apache-2.0 | `third_party/licenses/PNGdec/LICENSE` |
| RadioLib | 7.8.1 | MIT | `third_party/licenses/RadioLib/license.txt` |
| RFID_MFRC522v2 | 2.0.6 | LGPL-2.1 (actual fork license) | `third_party/licenses/RFID_MFRC522v2/LICENSE` |
| SimpleCLI | 1.1.4 | MIT | `third_party/licenses/SimpleCLI/LICENSE` |
| SmartRC-CC1101-Driver-Lib | 2.5.7 | original custom attribution/permission notice; retained verbatim | `third_party/licenses/SmartRC-CC1101-Driver-Lib/LICENSE.txt` |
| Time | 1.6.1 | LGPL-2.1-or-later source header | `third_party/licenses/Time/SOURCE-HEADERS.txt` |
| TinyGPSPlus | 1.1.0 | LGPL-2.1-or-later source header | `third_party/licenses/TinyGPSPlus/SOURCE-HEADERS.txt` |
| WireGuard-ESP32 | 0.1.5 | BSD-3-Clause and embedded component notices | `third_party/licenses/WireGuard-ESP32/LICENSE` |

ESP8266Audio's bundled libmad headers explicitly allow GPL version 2 **or later**; GPLv3 is selected where needed for this combination, unlike the excluded RF24-only grant. Apache-2.0, permissive notices and LGPL conditions remain applicable. The exact dependency snapshot includes source/build metadata and notices needed to modify and rebuild the libraries; it excludes Git histories, examples, tests, downloaded caches and compiled output.

## Vendored and platform code

Original Bruce vendored headers and license files remain intact, including TFT_eSPI/font notices, Bad_Usb_Lib Arduino LGPL/Espressif Apache notices, HAL, RTC, QR code, utility and PN532_SRIX attribution. See THIRD_PARTY.md for origins. PlatformIO's Arduino ESP32/ESP-IDF packages are identified by exact installed versions in BUILD-REPORT.md; their original upstream source/license terms remain applicable. No manufacturer-key data, ST/RFAL libraries or RF24 library is included in the public dependency snapshot.

## Redistribution obligations

Keep LICENSE, third-party notices and applicable copyright/source headers. Make the corresponding source and build scripts available alongside redistributed firmware. Preserve LGPL modification/relinking rights; do not add restrictions on reverse engineering needed to debug such modifications. Modified AGPL network deployments must comply with the applicable source-offer clause. Do not use upstream author/trademark names to imply endorsement.

Historical upstream THIRD_PARTY.md is retained for attribution; this review and DEPENDENCIES.json describe the actual current distribution and correct its ambiguous historical inventory entries. No claim is made that a compilation test establishes legal rights or physical device operation.
