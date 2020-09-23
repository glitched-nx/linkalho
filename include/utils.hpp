#ifndef __LINKALHO_UTILS_HPP__
#define __LINKALHO_UTILS_HPP__

#include <switch.h>

void attemptForceReboot();
HidsysNotificationLedPattern getBreathePattern();
HidsysNotificationLedPattern getConfirmPattern();
HidsysNotificationLedPattern getClearPattern();
void sendLedPattern(HidsysNotificationLedPattern pattern);
const std::string getPayload();
const std::string getRunningOS();

#endif // __LINKALHO_UTILS_HPP__