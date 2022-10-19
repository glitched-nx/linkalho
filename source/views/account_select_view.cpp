#include "views/account_select_view.hpp"
#include "utils/utils.hpp"
#include "core/shared_settings.hpp"
#include "constants.hpp"
#include <switch.h>
#include <string>
#include <sstream>

using namespace std;
using namespace brls::i18n::literals;

bool isLinked(AccountUid selectedUser) {
    bool linked = false;
    Service baas;
    const auto rc = getBaasAccountAdministrator(selectedUser, &baas);
    if(R_SUCCEEDED(rc)) {
        baasAdministrator_isLinkedWithNintendoAccount(&baas, &linked);
        serviceClose(&baas);
    }
    return linked;
}

void AccountSelectView::computeValue() {
    this->setValue(fmt::format("translations/account_select_view/selected_count"_i18n, this->selectionSet.size(), this->switchProfiles.size()), false, false);
    HardwareType hwType = getHardwareType();
    auto reboot_payload_text = "translations/account_select_view/reboot_payload_disabled"_i18n;
    if (hwType == Erista) {
        if (getPayload().empty()) {
            reboot_payload_text = fmt::format("translations/account_select_view/reboot_payload_inactive"_i18n, CUSTOM_PAYLOAD_FILE);
        } else {
            reboot_payload_text = "translations/account_select_view/reboot_payload_active"_i18n;
        }
    }
    this->setDescription(fmt::format("translations/account_select_view/extra_info"_i18n, this->switchProfiles.size(), (this->switchProfiles.size() == 1 ? "" : "s"), reboot_payload_text));
}

void AccountSelectView::updateSharedSettings() {
    vector<SwitchProfile> selected;
    for (auto p: this->switchProfiles) {
        if (this->selectionSet.find(p.uid_str) != this->selectionSet.end()) {
            selected.emplace_back(p);
        }
    }
    SharedSettings::instance().setSelectedProfiles(selected);
}

AccountSelectView::AccountSelectView() : ListItem("translations/account_select_view/title"_i18n)
{
    if (R_SUCCEEDED(accountInitialize(AccountServiceType_Administrator))) {
        AccountUid* uids = new AccountUid[ACC_USER_LIST_SIZE];
        s32 userCount = 0;

        if (R_SUCCEEDED(accountListAllUsers(uids, ACC_USER_LIST_SIZE, &userCount))) {
            this->switchProfiles.reserve(userCount);

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
                    uid_str << hex << (uid.uid[0] & 0xffffffff) << "-";
                    uid_str << ((uid.uid[0] >> 32) & 0xffff) << "-" << ((uid.uid[0] >> 48) & 0xffff) << "-";
                    uid_str << (uid.uid[1] & 0xff) << ((uid.uid[1] >> 8) & 0xff) << "-";
                    uid_str << hex << ((uid.uid[1] >> 32) & 0xffffffff) << ((uid.uid[1] >> 16) & 0xffff);

                    accountProfileClose(&profile);

                    auto item = SwitchProfile {
                        .id   = uid,
                        .uid_str = uid_str.str(),
                        .name = string(profileBase.nickname, 0x20),
                        .icon = make_pair(iconBuffer, imageSize),
                        .isLinked = isLinked(uid)
                    };
                    this->switchProfiles.emplace_back(item);
                    this->selectionSet.insert(item.uid_str);
                }
            }
        }

        delete[] uids;
        accountExit();
    }

    this->getClickEvent()->subscribe([this](brls::View* view) {
        auto accountSelectFrame = new brls::AppletFrame(true, true);
        accountSelectFrame->setTitle("translations/account_select_view/title"_i18n);

        this->accountListItems.clear();
        this->accountListItems.reserve( this->switchProfiles.size());

        auto accountList = new brls::List();
        for (auto p : this->switchProfiles) {
            auto item = new AccountListItem(p, this->selectionSet.find(p.uid_str) != this->selectionSet.end());
            item->getClickEvent()->subscribe([this](View* v){
                auto itemPtr = reinterpret_cast<AccountListItem*>(v);
                if (itemPtr->getToggleState()) {
                    this->selectionSet.insert(itemPtr->getAccountProfile().uid_str);
                } else {
                    this->selectionSet.erase(itemPtr->getAccountProfile().uid_str);
                }
                this->updateSharedSettings();
            });
            this->accountListItems.emplace_back(item);
            accountList->addView(item);
        }
        accountSelectFrame->setContentView(accountList);

        brls::Application::pushView(accountSelectFrame);

        accountSelectFrame->registerAction("translations/account_select_view/unselect_all"_i18n, brls::Key::L, [this] {
            for (auto item: this->accountListItems) {
                item->forceState(false);
            }
            return true;
        });
        accountSelectFrame->registerAction("translations/account_select_view/select_all"_i18n, brls::Key::R, [this] {
            for (auto item: this->accountListItems) {
                item->forceState(true);
            }
            return true;
        });
        accountSelectFrame->registerAction("translations/confirm_view/back"_i18n, brls::Key::B, [this, &accountList] {
            this->computeValue();
            brls::Application::popView();
            return true;
        });
        accountSelectFrame->registerAction("", brls::Key::PLUS, [this]{return true;}, true);
        accountSelectFrame->updateActionHint(brls::Key::PLUS, ""); // make the change visible
    });
    this->computeValue();
    this->updateSharedSettings();
}
