#pragma once
#include <stdint.h>
// Explicit estimate for a generic single-cell LiPo, NOT a measured cell SOC curve.
inline int batteryEstimatePercent(uint32_t mv) {
    const uint16_t volts[] = {3300, 3500, 3600, 3700, 3800, 3900, 4100, 4200};
    const uint8_t percent[] = {0, 5, 15, 35, 60, 75, 95, 100};
    if (mv <= volts[0]) return 0;
    for (unsigned i = 1; i < sizeof(volts)/sizeof(volts[0]); ++i) {
        if (mv <= volts[i]) return percent[i-1] + (mv-volts[i-1])*(percent[i]-percent[i-1])/(volts[i]-volts[i-1]);
    }
    return 100;
}
struct BatteryLowDebounce {
    unsigned count=0; bool low=false;
    bool update(uint32_t mv, bool valid) {
        if (!valid) { count=0; return low; }
        if (mv <= 3450) { if (count < 3) ++count; if (count >= 3) low=true; }
        else { count=0; if (mv >= 3600) low=false; }
        return low;
    }
};
