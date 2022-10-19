#include "views/account_select_view.hpp"
#include "views/account_list_item.hpp"
#include "utils/utils.hpp"
#include <string>
#include <switch.h>
#include <sstream>
#include <string>

using namespace std;
using namespace brls::i18n::literals;

bool isLinked(AccountUid selectedUser) {
    bool linked = false;
    Service baas;
    const auto rc = getBaasAccountAdministrator(selectedUser, &baas);
    if(R_SUCCEEDED(rc)) {
        // IsLinkedWithNintendoAccount service
        serviceDispatchOut(&baas, 250, linked);
        serviceClose(&baas);
    }
    return linked;
}

AccountSelectView::AccountSelectView() : ListItem("translations/account_select_view/title"_i18n)
{
    if (R_SUCCEEDED(accountInitialize(AccountServiceType_Administrator))) {
        AccountUid* uids = new AccountUid[ACC_USER_LIST_SIZE];
        s32 userCount = 0;

        if (R_SUCCEEDED(accountListAllUsers(uids, ACC_USER_LIST_SIZE, &userCount))) {
            this->accountListItems.reserve(userCount);

            for (int i = 0; i < userCount; i++) {
                // Icon data
                u8* iconBuffer;
                u32 imageSize, realSize;
                AccountUid uid = uids[i];
                // Lookup and cache the users details
                AccountProfile profile;
                AccountProfileBase profileBase = {};

                if (R_SUCCEEDED(accountGetProfile(&profile, uid)) && R_SUCCEEDED(accountProfileGet(&profile, nullptr, &profileBase)) && R_SUCCEEDED(accountProfileGetImageSize(&profile, &imageSize)) && (iconBuffer = (u8*)malloc(imageSize)) != NULL && R_SUCCEEDED(accountProfileLoadImage(&profile, iconBuffer, imageSize, &realSize))) {
                    stringstream uid_str;
                    uid_str << endl;
                    uid_str << hex << (uid.uid[0] & 0xffffffff) << "-";
                    uid_str << ((uid.uid[0] >> 32) & 0xffff) << "-" << ((uid.uid[0] >> 48) & 0xffff) << "-";
                    uid_str << (uid.uid[1] & 0xff) << ((uid.uid[1] >> 8) & 0xff) << "-";
                    uid_str << hex << ((uid.uid[1] >> 32) & 0xffffffff) << ((uid.uid[1] >> 16) & 0xffff);

                    accountProfileClose(&profile);

                    auto item = new AccountListItem(SwitchProfile{
                        .id   = uid,
                        .uid_str = uid_str.str(),
                        .name = string(profileBase.nickname, 0x20),
                        .icon = make_pair(iconBuffer, imageSize),
                        .isLinked = isLinked(uid)
                    });
                    this->accountListItems.emplace_back(item);
                }
            }
        }

        delete[] uids;
        accountExit();
    }

    this->getClickEvent()->subscribe([this](brls::View* view) {
        auto accountSelectFrame = new brls::AppletFrame(true, true);
        accountSelectFrame->setTitle("translations/account_select_view/title"_i18n);

        auto accountList = new brls::List();

        for (auto item : this->accountListItems) {
            accountList->addView(item);
        }

        accountSelectFrame->setContentView(accountList);
        brls::Application::pushView(accountSelectFrame);

        accountSelectFrame->registerAction("translations/account_select_view/unselect_all"_i18n, brls::Key::L, [this] {
            for (auto item : this->accountListItems) {
                item->forceState(false);
            }
            return true;
        });
        accountSelectFrame->registerAction("translations/account_select_view/select_all"_i18n, brls::Key::R, [this] {
            for (auto item : this->accountListItems) {
                item->forceState(true);
            }
            return true;
        });
        accountSelectFrame->registerAction("", brls::Key::PLUS, [this]{return true;}, true);
        accountSelectFrame->updateActionHint(brls::Key::PLUS, ""); // make the change visible
    });
}
