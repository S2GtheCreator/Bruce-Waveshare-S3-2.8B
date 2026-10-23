from pathlib import Path
import json,hashlib,sys
root=Path(__file__).resolve().parents[2]
baseline=Path(sys.argv[1]) if len(sys.argv)>1 else root.parent/'bruce-1.16.1-waveshare-native-hid'
allowed={
 'lib/Bad_Usb_Lib/KeyboardLayout_tr_TR.cpp','lib/Bad_Usb_Lib/Bad_Usb_Lib.h',
 'lib/Bad_Usb_Lib/USBHIDKeyboard.cpp','lib/Bad_Usb_Lib/USBHIDKeyboard.h',
 'lib/Bad_Usb_Lib/BleKeyboard.cpp','lib/Bad_Usb_Lib/BleKeyboard.h',
 'src/core/config.cpp','src/core/config.h','src/core/settings.cpp',
 'src/core/menu_items/OthersMenu.cpp','src/modules/badusb_ble/ducky_typer.cpp',
 'src/core/serial_commands/badusb_commands.cpp','src/modules/bjs_interpreter/badusb_js.cpp',
 'boards/waveshare_28b/waveshare_28b.ini',
}
if baseline.is_dir():
 manifest={}
 for folder in ['src','include','boards','lib','examples']:
  for p in (baseline/folder).rglob('*'):
   if not p.is_file() or p.suffix not in {'.c','.cpp','.h','.hpp','.inc','.ini','.csv','.txt'}:continue
   if p.suffix=='.txt' and folder!='examples':continue
   manifest[p.relative_to(baseline).as_posix()]=hashlib.sha256(p.read_bytes()).hexdigest()
 manifest['boards/_boards_json/waveshare_28b.json']=hashlib.sha256((baseline/'boards/_boards_json/waveshare_28b.json').read_bytes()).hexdigest()
 manifest['custom_16Mb.csv']=hashlib.sha256((baseline/'custom_16Mb.csv').read_bytes()).hexdigest()
 (root/'boards/waveshare_28b/turkish-q-baseline-sha256.json').write_text(json.dumps(manifest,indent=2))
else:
 manifest=json.loads((root/'boards/waveshare_28b/turkish-q-baseline-sha256.json').read_text())
for name,digest in manifest.items():
 if name not in allowed:assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
print('PASS: original source outside declared text/layout changes, display/GT911/battery/WiFi/CDC/partition and existing example scripts preserved')
c=(root/'src/core/config.cpp').read_text();h=(root/'src/core/config.h').read_text()
assert 'int badUSBBLEKeyboardLayout = 13;' in h
assert 'setting["badUSBLayoutRevision"] = 1;' in c
assert 'setting["badUSBLayoutRevision"].isNull() || setting["badUSBLayoutRevision"].as<int>() < 1' in c
assert 'if (count > 0) saveFile();' in c
assert 'badUSBBLEKeyboardLayout = setting["badUSBBLEKeyboardLayout"].as<int>();' in c
settings=(root/'src/core/settings.cpp').read_text()
assert '"English US"' in settings and '"Turkish Q"' in settings
assert 'bruceConfig.setBadUSBBLEKeyboardLayout(opt)' in settings
assert 'saveFile();' in c[c.index('void BruceConfig::setBadUSBBLEKeyboardLayout'):c.index('void BruceConfig::validateBadUSBBLEKeyboardLayout')]
print('PASS: default/layout migration/persistence and Turkish Q + English US menu wiring static checks; real reboot pending')
parser=(root/'src/modules/badusb_ble/ducky_typer.cpp').read_text()
assert 'Text error - script stopped' in parser and 'if (returnToMenu) goto EXIT;' in parser
assert 'strcmp(PriCmd->command, "STRINGLN") == 0' in parser
if baseline.is_dir():
 old=(baseline/'src/modules/badusb_ble/ducky_typer.cpp').read_text()
 start='} else if (PriCmd->type == DuckyCommandType_WaitForButtonPress)'
 end='            if (PriCmd == nullptr)'
 assert old[old.index(start):old.index(end,old.index(start))]==parser[parser.index(start):parser.index(end,parser.index(start))]
print('PASS: STRING/STRINGLN errors stop execution; non-text command parser preserved')
