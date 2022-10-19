#pragma once

#include "views/account_list_item.hpp"
#include <borealis.hpp>
#include <vector>
#include <switch.h>

using namespace std;
class AccountSelectView : public brls::ListItem
{
public:
    AccountSelectView();
private:
    std::vector<AccountListItem*> accountListItems;

};