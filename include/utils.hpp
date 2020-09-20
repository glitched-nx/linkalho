#ifndef __LINKALHO_UTILS_HPP__
#define __LINKALHO_UTILS_HPP__

#include <switch.h>

void attemptReboot();
HidsysNotificationLedPattern getBreathePattern();
HidsysNotificationLedPattern getConfirmPattern();
HidsysNotificationLedPattern getClearPattern();
void sendLedPattern(HidsysNotificationLedPattern pattern);

#endif // __LINKALHO_UTILS_HPP__