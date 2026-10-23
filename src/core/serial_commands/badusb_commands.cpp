#ifdef WAVESHARE_28B
#include <pro_modules.h>
#endif
#include "badusb_commands.h"
#include "core/sd_functions.h"
#include "helpers.h"
#include "modules/badusb_ble/ducky_typer.h"

uint32_t badusbFileCallback(cmd *c) {
#if WS28B_BADUSB_PRO
    if(waveshareProRunning()){serialDevice->println("BadUSB Pro owns HID; stop it on the device first.");return false;}
#endif

#ifndef LITE_VERSION
    // badusb run_from_file HelloWorld.txt

    Command cmd(c);

    Argument arg = cmd.getArgument("filepath");
    String filepath = arg.getValue();
    filepath.trim();

    if (filepath.indexOf(".txt") == -1) {
        serialDevice->println("Invalid filename");
        return false;
    }
    if (!filepath.startsWith("/")) filepath = "/" + filepath;

    FS *fs;
    if (!getFsStorage(fs)) return false;

    if (!(*fs).exists(filepath)) {
        serialDevice->println("File does not exist");
        return false;
    }

#ifdef USB_as_HID
    ducky_startKb(hid_usb, false);
    if (!hid_usb) return false;
    key_input(*fs, filepath, hid_usb);
    bool textOk = hid_usb->textError().failure == HidTextFailure::None;
    ducky_endUsb(hid_usb);

    // TODO: need to reinit serial when finished
    // Kb.end();
    // USB.~ESPUSB(); // Explicit call to destructor
    // serialDevice->begin(115200);

    return textOk;
#else
    return false;
#endif
#else
    return false;
#endif
}

uint32_t badusbBufferCallback(cmd *c) {
#if WS28B_BADUSB_PRO
    if(waveshareProRunning()){serialDevice->println("BadUSB Pro owns HID; stop it on the device first.");return false;}
#endif

#ifndef LITE_VERSION
    if (!(_setupPsramFs())) return false;

    char *txt = _readFileFromSerial();
    String tmpfilepath = "/tmpramfile"; // TODO: Change to use char *txt directly
    File f = PSRamFS.open(tmpfilepath, FILE_WRITE);
    if (!f) return false;

    f.write((const uint8_t *)txt, strlen(txt));
    f.close();
    free(txt);

#ifdef USB_as_HID
    ducky_startKb(hid_usb, false);
    if (!hid_usb) { PSRamFS.remove(tmpfilepath); return false; }
    key_input(PSRamFS, tmpfilepath, hid_usb);
    bool textOk = hid_usb->textError().failure == HidTextFailure::None;
    ducky_endUsb(hid_usb);

    PSRamFS.remove(tmpfilepath);
    return textOk;
#else
    PSRamFS.remove(tmpfilepath);
    return false;
#endif
#else
    return false;
#endif
}

void createBadUsbCommands(SimpleCLI *cli) {
#ifndef LITE_VERSION
    Command badusbCmd = cli->addCompositeCmd("bu,badusb");

    Command fileCmd = badusbCmd.addCommand("run_from_file", badusbFileCallback);
    fileCmd.addPosArg("filepath");

    Command bufferCmd = badusbCmd.addCommand("run_from_buffer", badusbBufferCallback);
#endif
}
