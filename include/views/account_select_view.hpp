#pragma once

#include "core/switch_profile.hpp"
#include "views/account_list_item.hpp"
#include <borealis.hpp>
#include <vector>
#include <map>
#include <string>
#include <set>

using namespace std;
class AccountSelectView : public brls::ListItem
{
public:
    AccountSelectView();

private:
    set<string> selectionSet;
    vector<SwitchProfile> switchProfiles;
    vector<AccountListItem*> accountListItems;
    void computeValue();
    void updateSharedSettings();
};