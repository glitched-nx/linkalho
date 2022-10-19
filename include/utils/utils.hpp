#pragma once

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
bool isErista();
HardwareType getHardwareType();
const std::string getLocale();
const std::string getTimezone();
Result getBaasAccountAdministrator(const AccountUid user_id, Service *out_admin_srv);
Result baasAdministrator_isLinkedWithNintendoAccount(Service *admin_srv, bool *out_linked);
