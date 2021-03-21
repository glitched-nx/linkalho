#ifndef __LINKALHO_UTILS_HPP__
#define __LINKALHO_UTILS_HPP__

#include <switch.h>

enum HardwareType {
    Erista,
    Mariko,
    Aula,
    UnknownHardware
};

void attemptForceReboot();
HidsysNotificationLedPattern getBreathePattern();
HidsysNotificationLedPattern getConfirmPattern();
HidsysNotificationLedPattern getClearPattern();
void sendLedPattern(HidsysNotificationLedPattern pattern);
const std::string getPayload();
const std::string getRunningOS();
bool isErista();
HardwareType getHardwareType();
const std::string getHardwareName(HardwareType hwType);

#endif // __LINKALHO_UTILS_HPP__