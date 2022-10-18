#include <switch.h>
#include <stdio.h>
#include <stdlib.h>
#include <borealis.hpp>
#include <string>
#include <sys/stat.h>
#include "confirm_page.hpp"
#include "worker_page.hpp"
#include "file_operations.hpp"
#include "constants.hpp"
#include "visual_overrides.hpp"
#include "progress_event.hpp"
#include "utils.hpp"
#include <iostream>
#include "country_list.hpp"
#include <bit>

#include <ranges>
#include <vector>

using namespace std;

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

    std::vector<std::string> keys;
    keys.reserve(COUNTRIES.size());
    for(const auto& c : COUNTRIES){
        keys.emplace_back(c.first);
    }

    // for(const auto& k : keys){
    //     std::cout << k << std::endl;
    // }
    std::cout << "Portugal: " << getOrDefault(COUNTRIES, "Portugal", "SHOULD_HAVE_BEEN_PT") << std::endl;
    std::cout << "NotPortugal: " << getOrDefault(COUNTRIES, "NotPortugal", "DEFAUT_PT") << std::endl;
    std::cout << "locale: " << getLanguage() << std::endl;
    std::cout << "timezone: " << getTimezone() << std::endl;

    // bool success_bcat = R_SUCCEEDED(pmshellTerminateProgram(0x010000000000000C));  // BCAT
    // bool success_account = R_SUCCEEDED(pmshellTerminateProgram(0x010000000000001E));  // ACCOUNT
    // // if (success_account && success_bcat) {
    // if (success_account) {
    //     cout << "Success!" << endl;
    // } else {
    //     // cout << "BCAT: " << (success_bcat ? "success!" : "Failed!") << endl;
    //     cout << "ACCOUNT: " << (success_account ? "success!" : "Failed!") << endl;
    // }


    // uint64_t pid = 0;
    // NcmProgramLocation location = {0x010000000000001E, NcmStorageId_BuiltInSystem};
    // NcmProgramLocation loc = {
    //         .program_id = 0x010000000000001E,
    //         .storageID = NcmStorageId_BuiltInSystem,
    //         .pad = {}
    //     };
    // Result res = pmshellLaunchProgram(0, &location, &pid);
    // if (R_FAILED(res)) {
    //     cout << "pmshellLaunchProgram fail" << endl;
    // } else {
    //     cout << "pmshellLaunchProgram success" << endl;
    // }

    // if (R_SUCCEEDED(setInitialize()))
    // if (R_SUCCEEDED(setsysInitialize())) {
    //     s32 max_entries=512;
    //     SetSysAccountOnlineStorageSettings* ss = (SetSysAccountOnlineStorageSettings*)malloc(max_entries*sizeof(SetSysAccountOnlineStorageSettings));
    //     s32 total_out = 1;
    //     if (R_SUCCEEDED(setsysGetAccountOnlineStorageSettings(&total_out, ss, max_entries))) {
    //         std::cout << "total: " << total_out << std::endl;
    //         for (s32 i = 0; i < total_out; ++i) {
    //             SetSysAccountOnlineStorageSettings set = ss[i];
    //             auto uid = set.uid;
    //             std::cout << "extracted: ";
    //             std::cout << std::hex << (uid.uid[0] & 0xffffffff) << "-";
    //             std::cout << ((uid.uid[0] >> 32) & 0xffff) << "-" << ((uid.uid[0] >> 48) & 0xffff) << "-";
    //             std::cout << (uid.uid[1] & 0xff) << ((uid.uid[1] >> 8) & 0xff) << "-";
    //             std::cout << std::hex << ((uid.uid[1] >> 32) & 0xffffffff) << ((uid.uid[1] >> 16) & 0xffff) << std::endl;
    //         }
    //     } else {
    //         std::cout << "nope" << std::endl;
    //     }
    // } else
    //     std::cout << "nope2" << std::endl;
    if (R_SUCCEEDED(accountInitialize(AccountServiceType_Administrator))) {
        AccountUid* uids = new AccountUid[ACC_USER_LIST_SIZE];
        s32 userCount = 0;

        if (R_SUCCEEDED(accountListAllUsers(uids, ACC_USER_LIST_SIZE, &userCount))) {
            for (int i = 0; i < userCount; i++) {
                // Icon data
                u8* iconBuffer;
                u32 imageSize, realSize;
                AccountUid uid = uids[i];



                // FsFileSystem acc;
                // if(R_SUCCEEDED(fsOpen_SystemSaveData(&acc, FsSaveDataSpaceId_User, 0x8000000000000010, uid))) {
                //     cout << "Succeess fsOpen_SystemSaveData!" << endl;
                //     if(R_SUCCEEDED(fsdevMountDevice("account", acc))) {
                //         cout << "Succeess fsdevMountDevice!" << endl;
                //     } else {
                //         cout << "Failed fsdevMountDevice!" << endl;
                //     }
                //     if (R_SUCCEEDED(fsdevUnmountDevice("account"))) {
                //         cout << "fsdevUnmountDevice successful!" << endl;
                //     } else {
                //         cout << "fsdevUnmountDevice failed!" << endl;
                //     }
                //     fsFsClose(&acc);
                // } else {
                //     cout << "Failed fsOpen_SystemSaveData!" << endl;
                // }


                // Lookup and cache the users details
                AccountProfile profile;
                AccountProfileBase profileBase = {};
                // AccountUserData userdata = {};
                if (R_SUCCEEDED(accountGetProfile(&profile, uid)) && R_SUCCEEDED(accountProfileGet(&profile, nullptr, &profileBase)) /*&& R_SUCCEEDED(accountProfileGetImageSize(&profile, &imageSize)) && (iconBuffer = (u8*)malloc(imageSize)) != NULL && R_SUCCEEDED(accountProfileLoadImage(&profile, iconBuffer, imageSize, &realSize))*/) {
                // if (R_SUCCEEDED(accountGetProfile(&profile, uid)) && R_SUCCEEDED(accountProfileGet(&profile, nullptr, &profileBase)) /*&& R_SUCCEEDED(accountProfileGetImageSize(&profile, &imageSize)) && (iconBuffer = (u8*)malloc(imageSize)) != NULL && R_SUCCEEDED(accountProfileLoadImage(&profile, iconBuffer, imageSize, &realSize))*/) {
                    auto username = std::string(profileBase.nickname, 0x20);
                    std::cout << std::endl;
                    // std::cout << std::endl;
                    // std::cout << std::endl;
                    // std::cout << std::endl;
                    // std::cout << std::endl;
                    // std::cout << "found username: " << username << std::endl;
                    // std::cout << "working..........................." << std::endl;
                    // std::cout << "retrying........" << std::endl;
                    // std::cout << "expected: " << "ce3f7d3d-3217-1000-ad1b-fecd2b82c289" << std::endl;
                    std::cout << std::hex << (uid.uid[0] & 0xffffffff) << "-";
                    std::cout << ((uid.uid[0] >> 32) & 0xffff) << "-" << ((uid.uid[0] >> 48) & 0xffff) << "-";
                    std::cout << (uid.uid[1] & 0xff) << ((uid.uid[1] >> 8) & 0xff) << "-";
                    std::cout << std::hex << ((uid.uid[1] >> 32) & 0xffffffff) << ((uid.uid[1] >> 16) & 0xffff) << std::endl;
                    std::cout << "profileBase.nickname: " << username << std::endl;
                    // std::cout << "pixa: " << FormatHex128(uid) << endl;
                    // std::cout << "entrypoint achieved! pwned!" << std::endl;

                    // std::cout << (R_SUCCEEDED(accountTrySelectUserWithoutInteraction(&uid, true)) ? "linked" : "not linked") << std::endl;
                    // std::cout << (R_SUCCEEDED(accountTrySelectUserWithoutInteraction(&uid, false)) ? "linked2" : "not linked2") << std::endl;
                    // std::cout << "profileBase.lastEditTimestamp: " << profileBase.lastEditTimestamp << std::endl;
                    // std::cout << "userdata.unk_x0: " << std::setfill('0') << std::setw(0x32) << std::hex << userdata.unk_x0 << std::endl;
                    // std::cout << "userdata.unk_x9: " << std::setfill('0') << std::setw(0x7) << std::hex << userdata.unk_x9 << std::endl;
                    // std::cout << "userdata.unk_x20: " << std::setfill('0') << std::setw(0x60) << std::hex << userdata.unk_x20 << std::endl;
                    // std::cout << "userdata.miiID : " << std::setfill('~') << std::setw(64) << std::hex << userdata.miiID << std::endl;
                    // std::cout << "userdata.unk_x20: " << std::setfill('~') << std::setw(64) << std::hex << userdata.unk_x20 << std::endl;
                    accountProfileClose(&profile);
                }
                const auto linked_info = GetUserLinkedInfo(uid);
                std::cout << "account_id: " << hex << linked_info.account_id << endl;
                std::cout << "nintendo_account_id: " << hex << linked_info.nintendo_account_id << endl;
                std::cout << "profileBase.islinked: " << (IsLinked(uid) ? "linked" : "not linked") << std::endl;
                if (linked_info.nintendo_account_id == 0x7141f86639a6b8a0) {
                    cout <<  "apagar" << hex << linked_info.nintendo_account_id << endl;
                    UnlinkLocally(uid);
                }
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

    brls::List* operationList = new brls::List();

    brls::ListItem* linkItem = new brls::ListItem("Link all accounts");
    linkItem->getClickEvent()->subscribe([canUseLed](brls::View* view) {
        brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
        stagedFrame->setTitle("Link all accounts");

        stagedFrame->addStage(
            new ConfirmPage(stagedFrame, "Linking all accounts will overwrite all previous links.\n(Your saves will be preserved)\n\nIf you had any previously linked NNID account, it will be overwritten!", false, canUseLed)
        );
        stagedFrame->addStage(
            new WorkerPage(stagedFrame, "Linking...", [](){
                linkAccount();
            })
        );
        stagedFrame->addStage(
            new ConfirmPage(stagedFrame, "Accounts linked!", true, canUseLed)
        );

        brls::Application::pushView(stagedFrame);
        stagedFrame->registerAction("", brls::Key::PLUS, []{return true;}, true);
        stagedFrame->updateActionHint(brls::Key::PLUS, ""); // make the change visible
        stagedFrame->registerAction("", brls::Key::B, []{return true;}, true);
        stagedFrame->updateActionHint(brls::Key::B, ""); // make the change visible
    });
    operationList->addView(linkItem);

    brls::ListItem* unlinkItem = new brls::ListItem("Unlink all accounts");
    unlinkItem->getClickEvent()->subscribe([canUseLed](brls::View* view) {
        brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
        stagedFrame->setTitle("Unlink all accounts");

        stagedFrame->addStage(
            new ConfirmPage(stagedFrame, "Unlinking accounts will reset all users.\n(Your saves will be preserved)\n\nIf you had any previously linked NNID account, it will be erased!", false, canUseLed)
        );
        stagedFrame->addStage(
            new WorkerPage(stagedFrame, "Unlinking...", [](){
                unlinkAccount();
            })
        );
        stagedFrame->addStage(
            new ConfirmPage(stagedFrame, "Accounts unlinked!", true, canUseLed)
        );

        brls::Application::pushView(stagedFrame);
        stagedFrame->registerAction("", brls::Key::PLUS, []{return true;}, true);
        stagedFrame->updateActionHint(brls::Key::PLUS, ""); // make the change visible
        stagedFrame->registerAction("", brls::Key::B, []{return true;}, true);
        stagedFrame->updateActionHint(brls::Key::B, ""); // make the change visible
    });
    operationList->addView(unlinkItem);

    struct stat buffer;
    bool backupExists = (stat(RESTORE_FILE_PATH, &buffer) == 0);
    if (backupExists) {
        brls::ListItem* restoreItem = new brls::ListItem("Restore backup");
        restoreItem->getClickEvent()->subscribe([canUseLed](brls::View* view) {
            brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
            stagedFrame->setTitle("Restore backup");

            stagedFrame->addStage(
                new ConfirmPage(stagedFrame, "Restoring this backup WILL overwrite all files!\n(Your saves will be preserved)\n\nMake sure the backup is valid as it will overwrite the console's partition files and might cause your Switch to stop booting!", false, canUseLed)
            );
            stagedFrame->addStage(
                new WorkerPage(stagedFrame, "Restoring...", [](){
                    restoreBackup(RESTORE_FILE_PATH);
                })
            );
            stagedFrame->addStage(
                new ConfirmPage(stagedFrame, "Backup restored!", true, canUseLed)
            );

            brls::Application::pushView(stagedFrame);
            stagedFrame->registerAction("", brls::Key::PLUS, []{return true;}, true);
            stagedFrame->updateActionHint(brls::Key::PLUS, ""); // make the change visible
            stagedFrame->registerAction("", brls::Key::B, []{return true;}, true);
            stagedFrame->updateActionHint(brls::Key::B, ""); // make the change visible
        });
        operationList->addView(restoreItem);
    } else {
        brls::ListItem* dialogItem = new brls::ListItem("Restore backup");
        dialogItem->getClickEvent()->subscribe([](brls::View* view) {
            brls::Dialog* dialog = new brls::Dialog("To restore, please place your backup file into\n" + string(RESTORE_FILE_PATH) + "\n\nMake sure the backup is valid as it will overwrite the console's partition files and might cause your Switch to stop booting!");
            dialog->addButton("Close", [dialog](brls::View* view) {
                dialog->close();
            });
            dialog->open();
            dialog->registerAction("", brls::Key::PLUS, []{return true;}, true);
            dialog->updateActionHint(brls::Key::PLUS, ""); // make the change visible
        });
        operationList->addView(dialogItem);
    }

    stringstream info_ss;
    auto os = getRunningOS();
    if (!os.empty()) {
        info_ss << os << " detected" << endl;
    }

    if (R_SUCCEEDED(accountInitialize(AccountServiceType_System))) {
        s32 acc_count = 0;
        accountGetUserCount(&acc_count);
        accountExit();
        if (acc_count > 0) {
            info_ss << "Found " << acc_count << " account" << (acc_count == 1 ? "" : "s") << endl;
        }
    }

    info_ss << "Restore file " << (backupExists ? "" : "not ") << "found" << endl;
    HardwareType hwType = getHardwareType();
    info_ss << getHardwareName(hwType) << " detected";
    if (hwType != Erista) {
        info_ss << " (reboot to payload disabled)" << endl;
    } else {
        info_ss << endl << "Reboot to payload ";
        if (getPayload().empty()) {
            info_ss << "inactive (" << CUSTOM_PAYLOAD_FILE << " not found)";
        } else {
            info_ss << "active";
        }
    }

    brls::ListItem* backupItem = new brls::ListItem("Create manual backup", info_ss.str());
    backupItem->getClickEvent()->subscribe([canUseLed](brls::View* view) {
        brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
        stagedFrame->setTitle("Create manual backup");

        stagedFrame->addStage(
            new ConfirmPage(stagedFrame, "All linking and unlinking operations will produce a backup before making changes!\n\nYou should only use this option if you want to manually create a backup.", false, canUseLed)
        );
        stagedFrame->addStage(
            new WorkerPage(stagedFrame, "Backing up...", [](){
                manualBackup();
            })
        );
        stagedFrame->addStage(
            new ConfirmPage(stagedFrame, "Backup created!", true, canUseLed)
        );

        brls::Application::pushView(stagedFrame);
        stagedFrame->registerAction("", brls::Key::PLUS, []{return true;}, true);
        stagedFrame->updateActionHint(brls::Key::PLUS, ""); // make the change visible
        stagedFrame->registerAction("", brls::Key::B, []{return true;}, true);
        stagedFrame->updateActionHint(brls::Key::B, ""); // make the change visible
    });
    operationList->addView(backupItem);


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
