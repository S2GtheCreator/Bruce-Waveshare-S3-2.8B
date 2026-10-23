# Installation, flashing and recovery

Target: **Waveshare ESP32-S3-Touch-LCD-2.8B N16R8 only**. Pro hardware validation is pending.

1. Download the release assets and check the SHA-256 values against `SHA256SUMS.txt`.
2. Use a USB data cable and identify the correct serial port. Close programs using it.
3. Back up flash before writing; protect the backup because it may contain private configuration. Do not upload it to a public issue.
4. Enter the board's documented download mode if automatic bootloader entry fails.

Commands below require Python and esptool 5. Replace `PORT` (for example `COM5`) with the actual port.

```sh
python -m esptool --chip esp32s3 --port PORT read-flash 0x0 0x1000000 backup.bin
```

## Full image at 0x0

```sh
python -m esptool --chip esp32s3 --port PORT write-flash 0x0 Bruce-1.16.1-ws28b-pro-merged.bin
```

**This can reset NVS/settings.** The merged image contains bootloader 0x0, partition table 0x8000 and application 0x10000. It contains no LittleFS image and is not a complete 16 MB device backup. Do not use `erase-flash` unless deliberately accepting loss of all stored data.

## Application-only update at 0x10000

Only when the installed partition table matches this project's `custom_16Mb.csv`:

```sh
python -m esptool --chip esp32s3 --port PORT write-flash 0x10000 Bruce-1.16.1-ws28b-pro-app-0x10000.bin
```

This avoids writing the NVS address range. It does not guarantee compatibility with an unknown firmware or preserve settings across incompatible application formats.

## Verify the checksum (PowerShell)

```powershell
Get-FileHash -Algorithm SHA256 -LiteralPath .\Bruce-1.16.1-ws28b-pro-merged.bin
```

Compare the complete value with `SHA256SUMS.txt`. Linux/macOS users may use `sha256sum -c SHA256SUMS.txt` after downloading all listed assets.

## Recovery

Return to download mode and restore your own known-good full backup at 0x0, or use a matching trusted recovery image. Original private backup/stage binaries are deliberately not redistributed because they predate the manufacturer-key removal. Do not mix bootloaders, partition tables or application offsets from unrelated devices.

No physical flashing, boot, display, HID or power-path test was performed as part of this release preparation.
