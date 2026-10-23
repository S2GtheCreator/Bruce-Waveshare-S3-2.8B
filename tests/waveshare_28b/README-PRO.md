# Pro host checks

Run Python scripts in this directory with Python 3 and a C++11 GCC host compiler.
Set HOST_CXX to the compiler path when g++ is not on PATH (Windows fallback: TDM-GCC-32).

* pro_ui_contract.py: actual scrollable text widget, UTF-8 metrics and baseline file digests.
* pro_storage_contract.py: actual safeSave implementation with in-memory filesystem fault injection.
* pro_preview_contract.py: actual preview loop and shared HID codec; never sends keyboard input.
* pro_battery_contract.py: actual battery estimate/debounce model, not an ADC hardware test.
* turkish_q_contract.py: actual USB keyboard source with mocked transport, 447 report cases.

No host test flashes a device, runs a payload, types keys, or proves LCD/Wi-Fi/ADC operation.
Legacy recovery/displayfix test scripts are retained as historical checks for their original revisions.

* pro_network_text_contract.py: actual SSID/CSV escaping, 11 cases.
* pro_script_limits_contract.py: actual pre-split memory limits + approved-line integrity, 20 cases.
* pro_read_contract.py: actual bounded read/error/cancel/timeout path, 11 cases.

Feature-off configurations are checked with the actual SDK/compiler in syntax-only mode;
only the default full Pro target has a complete PlatformIO build and merged image.
