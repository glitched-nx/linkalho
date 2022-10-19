#include <switch.h>
#include <borealis.hpp>
#include "styles/visual_overrides.hpp"
#include "views/country_select_view.hpp"
#include "views/restore_backup_view.hpp"
#include "views/create_backup_view.hpp"
#include "views/link_accounts_view.hpp"
#include "views/unlink_accounts_view.hpp"
#include "views/account_select_view.hpp"



#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include "views/confirm_view.hpp"
#include "views/worker_view.hpp"
#include "core/file_operations.hpp"
#include "constants.hpp"
#include "utils/progress_event.hpp"
#include "utils/utils.hpp"
#include <iostream>
// #include "country_list.hpp"
#include <bit>

#include <ranges>
#include <vector>

using namespace std;
using namespace brls::i18n::literals;

std::string FormatHex128(AccountUid Number){
	auto ptr = reinterpret_cast<u8*>(Number.uid);
	std::stringstream strm;
	strm << std::hex << std::uppercase;
	for(u32 i = 0; i < 16; i++) strm << (u32)ptr[i];
	return strm.str();
}

struct LinkedAccountInfo {
    u64 account_id;
    u64 nintendo_account_id;
};

Result GetBaasAccountAdministrator(const AccountUid user_id, Service *out_admin_srv) {
    return serviceDispatchIn(accountGetServiceSession(), 250, user_id,
        .out_num_objects = 1,
        .out_objects = out_admin_srv,
    );
}

Result BaasAdministrator_DeleteRegistrationInfoLocally(Service *admin_srv) {
    return serviceDispatch(admin_srv, 203);
}

Result BaasAdministrator_IsLinkedWithNintendoAccount(Service *admin_srv, bool *out_linked) {
    return serviceDispatchOut(admin_srv, 250, *out_linked);
}

Result BaasAdministrator_GetNintendoAccountId(Service *admin_srv, u64 *out_id) {
    return serviceDispatchOut(admin_srv, 120, *out_id);
}

Result BaasAdministrator_GetAccountId(Service *admin_srv, u64 *out_id) {
    return serviceDispatchOut(admin_srv, 1, *out_id);
}

bool IsLinked(AccountUid g_SelectedUser) {
        bool linked = false;
        Service baas;
        const auto rc = GetBaasAccountAdministrator(g_SelectedUser, &baas);
        if(R_SUCCEEDED(rc)) {
            // cout << "GetBaasAccountAdministrator success" << endl;
            BaasAdministrator_IsLinkedWithNintendoAccount(&baas, &linked);
            serviceClose(&baas);
        } else cout << "GetBaasAccountAdministrator fail" << endl;
        return linked;
    }

// LinkedAccountInfo GetUserLinkedInfo(AccountUid g_SelectedUser) {
//     LinkedAccountInfo info = {};
//     Service baas;
//     const auto rc = GetBaasAccountAdministrator(g_SelectedUser, &baas);
//     if(R_SUCCEEDED(rc)) {
//         BaasAdministrator_GetAccountId(&baas, &info.account_id);
//         BaasAdministrator_GetNintendoAccountId(&baas, &info.nintendo_account_id);
//         serviceClose(&baas);
//     }
//     return info;
// }

Result UnlinkLocally(AccountUid g_SelectedUser) {
    Service baas;
    auto rc = GetBaasAccountAdministrator(g_SelectedUser, &baas);
    if(R_SUCCEEDED(rc)) {
        bool linked = false;
        rc = BaasAdministrator_IsLinkedWithNintendoAccount(&baas, &linked);
        std::cout << "is linked:" << linked << std::endl;
        if(R_SUCCEEDED(rc) && linked) {
            rc = BaasAdministrator_DeleteRegistrationInfoLocally(&baas);
        } else { std::cout << "bork unlink2" << std::endl; }
        serviceClose(&baas);
    } else {std::cout << "bork unlink1" << std::endl;}
    return rc;
}

LinkedAccountInfo GetUserLinkedInfo(AccountUid g_SelectedUser) {
    LinkedAccountInfo info = {};
    Service baas;
    const auto rc = GetBaasAccountAdministrator(g_SelectedUser, &baas);
    if(R_SUCCEEDED(rc)) {
        BaasAdministrator_GetAccountId(&baas, &info.account_id);
        BaasAdministrator_GetNintendoAccountId(&baas, &info.nintendo_account_id);
        serviceClose(&baas);
    }
    return info;
}


int main(int argc, char* argv[])
{
    pmshellInitialize();
    bool canUseLed = false;
    if (R_SUCCEEDED(hidsysInitialize())) {
        canUseLed = true;
    }
    initDirs();

    brls::i18n::loadTranslations();

    // std::vector<std::string> keys;
    // keys.reserve(COUNTRIES.size());
    // for(const auto& c : COUNTRIES){
    //     keys.emplace_back(c.first);
    // }

    // for(const auto& k : keys){
    //     std::cout << k << std::endl;
    // }
    // std::cout << "Portugal: " << getOrDefault(COUNTRIES, "Portugal", "SHOULD_HAVE_BEEN_PT") << std::endl;
    // std::cout << "NotPortugal: " << getOrDefault(COUNTRIES, "NotPortugal", "DEFAUT_PT") << std::endl;
    std::cout << "locale: " << getLanguage() << std::endl;
    std::cout << "timezone: " << getTimezone() << std::endl;

    if (R_SUCCEEDED(accountInitialize(AccountServiceType_Administrator))) {
        AccountUid* uids = new AccountUid[ACC_USER_LIST_SIZE];
        s32 userCount = 0;

        if (R_SUCCEEDED(accountListAllUsers(uids, ACC_USER_LIST_SIZE, &userCount))) {
            for (int i = 0; i < userCount; i++) {
                // Icon data
                u8* iconBuffer;
                u32 imageSize, realSize;
                AccountUid uid = uids[i];
                // Lookup and cache the users details
                AccountProfile profile;
                AccountProfileBase profileBase = {};
                if (R_SUCCEEDED(accountGetProfile(&profile, uid)) && R_SUCCEEDED(accountProfileGet(&profile, nullptr, &profileBase)) /*&& R_SUCCEEDED(accountProfileGetImageSize(&profile, &imageSize)) && (iconBuffer = (u8*)malloc(imageSize)) != NULL && R_SUCCEEDED(accountProfileLoadImage(&profile, iconBuffer, imageSize, &realSize))*/) {
                // if (R_SUCCEEDED(accountGetProfile(&profile, uid)) && R_SUCCEEDED(accountProfileGet(&profile, nullptr, &profileBase)) /*&& R_SUCCEEDED(accountProfileGetImageSize(&profile, &imageSize)) && (iconBuffer = (u8*)malloc(imageSize)) != NULL && R_SUCCEEDED(accountProfileLoadImage(&profile, iconBuffer, imageSize, &realSize))*/) {
                    auto username = std::string(profileBase.nickname, 0x20);
                    std::cout << std::endl;
                    std::cout << std::hex << (uid.uid[0] & 0xffffffff) << "-";
                    std::cout << ((uid.uid[0] >> 32) & 0xffff) << "-" << ((uid.uid[0] >> 48) & 0xffff) << "-";
                    std::cout << (uid.uid[1] & 0xff) << ((uid.uid[1] >> 8) & 0xff) << "-";
                    std::cout << std::hex << ((uid.uid[1] >> 32) & 0xffffffff) << ((uid.uid[1] >> 16) & 0xffff) << std::endl;
                    std::cout << "profileBase.nickname: " << username << std::endl;
                    // std::cout << "pixa: " << FormatHex128(uid) << endl;
                    accountProfileClose(&profile);
                }
                const auto linked_info = GetUserLinkedInfo(uid);
                std::cout << "account_id: " << linked_info.account_id << endl;
                std::cout << "nintendo_account_id: " << linked_info.nintendo_account_id << endl;
                std::cout << "profileBase.islinked: " << (IsLinked(uid) ? "linked" : "not linked") << std::endl;
                // if (linked_info.nintendo_account_id == 0x7141f86639a6b8a0) {
                //     cout <<  "apagar" << hex << linked_info.nintendo_account_id << endl;
                //     UnlinkLocally(uid);
                // }
            }
        }

        delete[] uids;
        accountExit();
    } else {
        std::cout << "bork1" << std::endl;
    }

    auto linkalhoStyle = VisualOverrides::LinkalhoStyle();
    auto linkalhoTheme = VisualOverrides::LinkalhoTheme();

    if (!brls::Application::init(APP_TITLE_LOWER, linkalhoStyle, linkalhoTheme))
    {
        brls::Logger::error(string("Unable to init ") + APP_TITLE);
        return EXIT_FAILURE;
    }

    brls::AppletFrame* rootFrame = new brls::AppletFrame(false, false);
    rootFrame->setTitle(string(APP_TITLE) + " v" + string(APP_VERSION));
    if (R_SUCCEEDED(accountInitialize(AccountServiceType_System))) {
        s32 acc_count = 0;
        accountGetUserCount(&acc_count);
        accountExit();
    }

    auto operationList = new brls::List();

    auto accountSelectView = new AccountSelectView();
    operationList->addView(accountSelectView);

    auto linkAccountsView = new LinkAccountsView(canUseLed);
    operationList->addView(linkAccountsView);

    auto unlinkAccountsView = new UnlinkAccountsView(canUseLed);
    operationList->addView(unlinkAccountsView);

    auto restoreBackupView = new RestoreBackupView(canUseLed);
    operationList->addView(restoreBackupView);

    auto createBackupView = new CreateBackupView(canUseLed);
    operationList->addView(createBackupView);

    auto countrySelectView = new CountrySelectView();
    countrySelectView->getValueSelectedEvent()->subscribe([](int result) {
        cout << "selected2 " << result << endl;
    });
    operationList->addView(countrySelectView);

    // auto accountSelectView = new AccountSelectView();
    // operationList->addView(accountSelectView);

    // auto userSelectItem = new brls::SelectListItem("translations/account_select_view/title"_i18n, {"a", "b"});
    // userSelectItem->getValueSelectedEvent()->subscribe([](int result) {
    //     cout << "selected3 " << result << endl;
    // });
    // stringstream info_ss;
    // if (R_SUCCEEDED(accountInitialize(AccountServiceType_System))) {
    //     s32 acc_count = 0;
    //     accountGetUserCount(&acc_count);
    //     accountExit();
    //     if (acc_count > 0) {
    //         info_ss << "Found " << acc_count << " account" << (acc_count == 1 ? "" : "s") << endl;
    //     }
    // }

    // HardwareType hwType = getHardwareType();
    // info_ss << "Reboot to payload ";
    // if (hwType != Erista) {
    //     info_ss << " disabled" << endl;
    // } else {
    //     if (getPayload().empty()) {
    //         info_ss << "inactive (" << CUSTOM_PAYLOAD_FILE << " not found)";
    //     } else {
    //         info_ss << "active";
    //     }
    // }
    // userSelectItem->setReduceDescriptionSpacing(true);
    // userSelectItem->setDescription(info_ss.str());
    // operationList->addView(userSelectItem);

    rootFrame->setContentView(operationList);
    brls::Application::pushView(rootFrame);
    rootFrame->registerAction("", brls::Key::MINUS, []{return true;}, true);
    rootFrame->updateActionHint(brls::Key::MINUS, ""); // make the change visible

    // Run the app
    while (brls::Application::mainLoop())
        ;

    pmshellExit();
    if (canUseLed)
        hidsysExit();

    // Exit
    return EXIT_SUCCESS;
}
