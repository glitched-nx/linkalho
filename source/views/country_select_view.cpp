#include "views/country_select_view.hpp"
#include "utils/country_list.hpp"
#include <string>

using namespace std;
using namespace brls::i18n::literals;

CountrySelectView::CountrySelectView() : SelectListItem("translations/main_menu/select_profile_country"_i18n, {"Portugal"}, 0)
{
    vector<string> countryList = {};

    countryList.reserve(COUNTRIES.size());
    for (auto const& c : COUNTRIES) {
        countryList.emplace_back(c.first);
    }
    this->values.swap(countryList);
    auto it = find(this->values.begin(), this->values.end(), "Portugal");
    if (it != this->values.end()) {
        this->setSelectedValue(distance(this->values.begin(), it));
    }
}
