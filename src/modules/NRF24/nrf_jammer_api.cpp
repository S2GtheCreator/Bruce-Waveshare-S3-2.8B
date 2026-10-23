// SPDX-License-Identifier: AGPL-3.0-or-later
// Distribution compatibility stubs: no RF24 driver code is linked.
#include "nrf_jammer_api.h"
#if !defined(LITE_VERSION)
bool isNRF24Available() { return false; }
bool startBLEJammer(BLEJamMode, int) { return false; }
void updateBLEJammer() {}
void stopBLEJammer() {}
bool isBLEJammingActive() { return false; }
int getCurrentBLEChannel() { return -1; }
void setBLEJammingPower(int) {}
bool jamBLEChannel(int) { return false; }
bool jamBLEAdvertisingChannels() { return false; }
bool jamBLEConnectionChannel(NimBLEAddress) { return false; }
bool jamDuringConnect(NimBLEAddress) { return false; }
#endif
