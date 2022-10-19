#pragma once

#include "core/switch_profile.hpp"
#include "constants.hpp"
#include <string>
#include <vector>
#include <iostream>

using namespace std;

class SharedSettings{
private:
    SharedSettings() {}

    string countryCode = DEFAULT_COUNTRY_CODE;
    vector<SwitchProfile> selectedProfiles;

public:
    SharedSettings(const SharedSettings&) = delete;
    SharedSettings& operator=(const SharedSettings &) = delete;
    SharedSettings(SharedSettings &&) = delete;
    SharedSettings & operator=(SharedSettings &&) = delete;

    static auto& instance(){
        static SharedSettings storage;
        return storage;
    }

    inline const string& getCountryCode() {
        return countryCode;
    }

    inline void setCountryCode(const string& code) {
        countryCode = code;
        cout << "new country code: " << code << endl;
    }

    inline const vector<SwitchProfile>& getSelectedProfiles() {
        return selectedProfiles;
    }

    void setSelectedProfiles(const vector<SwitchProfile>& profiles) {
        selectedProfiles.clear();
        selectedProfiles.reserve(profiles.size());
        cout << "new country profile list: " << endl;
        for(auto p: profiles) {
            selectedProfiles.emplace_back(p);
            cout << "[" << p.name << "]" << endl;
        }
    }
};
