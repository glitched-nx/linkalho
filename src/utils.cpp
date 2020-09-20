#include <iostream>
#include <cstring>
#include "constants.hpp"
#include "utils.hpp"

void attemptReboot()
{
#ifndef DEBUG
    Result rc = bpcInitialize();
    if (R_FAILED(rc))
        printf("bpcInit: %08X\n", rc);
    else
    {
        bpcRebootSystem();
        bpcExit();
    }
#else
    std::cout << "Reboot would happen here" << std::endl;
#endif
}

HidsysNotificationLedPattern getBreathePattern()
{
    HidsysNotificationLedPattern pattern;
    memset(&pattern, 0, sizeof(pattern));

    pattern.baseMiniCycleDuration = 0x8;    // 100ms.
    pattern.totalMiniCycles = 0x2;          // 2 mini cycles. Last one 12.5ms.
    pattern.totalFullCycles = 0x0;          // Repeat forever.
    pattern.startIntensity = 0x0;           // 0%

    pattern.miniCycles[0].ledIntensity = 0xF;        // 100%.
    pattern.miniCycles[0].transitionSteps = 0xF;     // 15 steps. Transition time 1.5s.
    pattern.miniCycles[0].finalStepDuration = 0x0;   // Forced 12.5ms.
    pattern.miniCycles[1].ledIntensity = 0x0;        // 0%.
    pattern.miniCycles[1].transitionSteps = 0xF;     // 15 steps. Transition time 1.5s.
    pattern.miniCycles[1].finalStepDuration = 0x0;   // Forced 12.5ms.

    return pattern;
}

HidsysNotificationLedPattern getConfirmPattern()
{
    HidsysNotificationLedPattern pattern;
    memset(&pattern, 0, sizeof(pattern));

    pattern.baseMiniCycleDuration = 0x1;             // 12.5ms.
    pattern.totalMiniCycles = 0x2;                   // 3 mini cycles. Last one 12.5ms.
    pattern.totalFullCycles = 0x3;                   // Repeat 3 times.
    pattern.startIntensity = 0x0;                    // 0%.

    pattern.miniCycles[0].ledIntensity = 0xF;        // 100%.
    pattern.miniCycles[0].transitionSteps = 0xF;     // 15 steps. Transition time 187.5ms.
    pattern.miniCycles[0].finalStepDuration = 0x0;   // Forced 12.5ms.
    pattern.miniCycles[1].ledIntensity = 0x0;        // 0%.
    pattern.miniCycles[1].transitionSteps = 0x2;     // 2 steps. Transition time 25ms.
    pattern.miniCycles[1].finalStepDuration = 0x0;   // Forced 12.5ms.

    return pattern;
}

HidsysNotificationLedPattern getClearPattern()
{
    HidsysNotificationLedPattern pattern;
    memset(&pattern, 0, sizeof(pattern));
    return pattern;
}

void sendLedPattern(HidsysNotificationLedPattern pattern)
{
    s32 total_entries;
    u64 uniquePadIds[2];
    memset(uniquePadIds, 0, sizeof(uniquePadIds));
    Result res = hidsysGetUniquePadsFromNpad(hidGetHandheldMode() ? CONTROLLER_HANDHELD : CONTROLLER_PLAYER_1, uniquePadIds, 2, &total_entries);
    if (R_SUCCEEDED(res)) {
        for (auto i = 0; i < total_entries; ++i) {
            hidsysSetNotificationLedPattern(&pattern, uniquePadIds[i]);
        }
    }
}