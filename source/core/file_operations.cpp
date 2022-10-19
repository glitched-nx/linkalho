#include <borealis.hpp>
#include <switch.h>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <filesystem>
#include <sstream>
#include <vector>
#include <fstream>
#include "zipper.h"
#include "unzipper.h"
#include "constants.hpp"
#include "core/generator.hpp"
#include "core/shared_settings.hpp"
#include "core/switch_profile.hpp"
#include "utils/progress_event.hpp"

using namespace std;
using namespace zipper;
using namespace brls::i18n::literals;

void shutdownAccount()
{
    cout << "Attempting to shut down BCAT, account and olsc... ";
    bool success_bcat = R_SUCCEEDED(pmshellTerminateProgram(0x010000000000000C));  // BCAT
    bool success_account = R_SUCCEEDED(pmshellTerminateProgram(0x010000000000001E));  // ACCOUNT
    if (success_account && success_bcat) {
        cout << "Success!" << endl;
    } else {
        cout << "BCAT: " << (success_bcat ? "success!" : "Failed!") << endl;
        cout << "ACCOUNT: " << (success_account ? "success!" : "Failed!") << endl;
    }
    //helps a bit
    pmshellTerminateProgram(0x010000000000003E);  // OLSC
}

vector<filesystem::path> getDirContents(const string& path, const string& extension, bool onlydirs=false)
{
    vector<filesystem::path> contents;
    bool filtered = extension.length() > 0;
    for (auto& entry: filesystem::recursive_directory_iterator(path)) {
        if (!entry.is_regular_file()) {
            if (onlydirs || !filtered) {
                contents.push_back(entry.path());
            }
            continue;
        }
        if (onlydirs || entry.path().extension().string() == "")
            continue;
        if (!filtered || entry.path().extension().string() == extension) {
            contents.push_back(entry.path());
        }
    }
    return contents;
}

void cleanupMacFiles(const string& path)
{
    for (auto& entry: filesystem::recursive_directory_iterator(path)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        if (entry.path().filename().string() == ".DS_Store") {
            filesystem::remove_all(entry.path().string());
        }
    }
}

bool initDirs()
{
    try
    {
        filesystem::create_directories(BACKUP_PATH);
        filesystem::create_directories(RESTORE_FILE_DIR);
        return true;
    }
    catch (exception& e) // Not using filesystem_error since bad_alloc can throw too.
    {
        cout << e.what() << endl;
        brls::Application::crash(string("Failed! ") + e.what());
    }
    return false;
}

FsFileSystem mountSaveData()
{
    FsFileSystem acc;
#ifndef LINKALHO_DEBUG
    shutdownAccount();
    cout << "Attempting to mount savedata... ";
    if(R_SUCCEEDED(fsOpen_SystemSaveData(&acc, FsSaveDataSpaceId_System, 0x8000000000000010, (AccountUid) {0}))) {
        cout << "Succeess!" << endl;
        fsdevMountDevice("account", acc);
    } else {
        cout << "Failed!" << endl;
    }
#endif
    return acc;
}

void unmountSaveData(FsFileSystem& acc, bool commit=false)
{
#ifndef LINKALHO_DEBUG
    if (commit) {
        if (R_SUCCEEDED(fsdevCommitDevice("account"))) {
            cout << "fsdevCommitDevice successful!" << endl;
        } else {
            cout << "fsdevCommitDevice failed!" << endl;
        }
    }
    if (R_SUCCEEDED(fsdevUnmountDevice("account"))) {
        cout << "fsdevUnmountDevice successful!" << endl;
    } else {
        cout << "fsdevUnmountDevice failed!" << endl;
    }
    fsFsClose(&acc);
#endif
}

void executeBackup(const string& reason)
{
    cleanupMacFiles(RESTORE_PATH);

    time_t t = time(nullptr);
    tm tm = *localtime(&t);
    stringstream backupFile;
    backupFile << BACKUP_PATH << "backup_profiles_" << put_time(&tm, "%Y%m%d_%H%M%S") << "[" << reason << "].zip";
    cout << "Creating backup..." << endl << endl;
    Zipper zipper(backupFile.str());
    //auto flags = (Zipper::zipFlags::Better | Zipper::zipFlags::SaveHierarchy);
    zipper.add(string(ACCOUNT_PATH)+"/registry.dat");
    zipper.add(string(ACCOUNT_PATH)+"/avators");
    zipper.add(string(ACCOUNT_PATH)+"/baas");
    zipper.add(string(ACCOUNT_PATH)+"/nas");
    zipper.close();
    cout << "Backup created in " << BACKUP_PATH << endl;
}

void restoreBackup(const string& backupFullpath)
{
    try {
        ProgressEvent::instance().reset();
        ProgressEvent::instance().setTotalSteps(7);

        FsFileSystem acc = mountSaveData();
        ProgressEvent::instance().setStep(1);

        executeBackup("restore");
        ProgressEvent::instance().setStep(2);
        cout << endl << endl  << "Restoring backup... ";

        auto baasDir = string(RESTORE_PATH) + "/baas";
        filesystem::remove_all(baasDir);
        ProgressEvent::instance().setStep(3);

        auto nasDir = string(RESTORE_PATH) + "/nas";
        filesystem::remove_all(nasDir);
        ProgressEvent::instance().setStep(4);

        auto avatorsDir = string(RESTORE_PATH) + "/avators";
        for (auto& jpgFile: getDirContents(avatorsDir, ".jpg")) {
            filesystem::remove_all(jpgFile.string());
        }
        ProgressEvent::instance().setStep(5);

        Unzipper unzipper(backupFullpath);
        unzipper.extract(RESTORE_PATH);
        unzipper.close();
        ProgressEvent::instance().setStep(6);
        cout << "Success!" << endl;

        unmountSaveData(acc, true);
        ProgressEvent::instance().setStep(7);
    }
    catch (exception& e) // Not using filesystem_error since bad_alloc can throw too.
    {
        cout << "Failed! " << e.what() << endl;
        brls::Application::crash(string("Failed! ") + e.what());
    }
}

void unlinkSingleAccount(const SwitchProfile& profile) {
    // first we open the baas file if exists
    string baas_file = string(ACCOUNT_PATH) + "/baas/" + profile.uid_str + ".dat";
    if (!filesystem::exists(baas_file)) {
        cout << "Baas file not found: " << baas_file << endl;
        return;
    }

    // then we read and extract the nasId
    std::ifstream baas_ifs(baas_file, std::ios::binary);
    baas_ifs.ignore(16);

    uint64_t count;
    baas_ifs.read((char*)&count, 16);
    baas_ifs.close();

    stringstream nasId_ss;
    nasId_ss << std::setfill('0') << std::setw(16) << std::hex << count;

    std::error_code ec;
    auto nasDatFile = string(ACCOUNT_PATH) + "/nas/" + nasId_ss.str() + ".dat";
    if (filesystem::exists(nasDatFile)) {
        if (!filesystem::remove(nasDatFile, ec)) {
            cout << "Error removing nasDatFile: " << nasDatFile << " error: " << ec << endl;
            return;
        }
    }
    auto nasJsonFile = string(ACCOUNT_PATH) + "/nas/" + nasId_ss.str() + "_user.json";
    if (filesystem::exists(nasJsonFile)) {
        if (!filesystem::remove(nasJsonFile, ec)) {
            cout << "Error removing nasJsonFile: " << nasJsonFile << " error: " << ec << endl;
            return;
        }
    }

    if (!filesystem::remove(baas_file, ec)) {
        cout << "Error removing baas_file: " << baas_file << " error: " << ec << endl;
        return;
    }
}

void linkAccounts()
{
    auto profileCount = SharedSettings::instance().getSelectedProfiles().size();
    if (profileCount == 0) {
        brls::Application::notify("translations/errors/no_accounts_selected"_i18n);
        brls::Application::popView();
        return;
    }
    try {
        ProgressEvent::instance().reset();
        ProgressEvent::instance().setTotalSteps(4+profileCount);

        FsFileSystem acc = mountSaveData();
        ProgressEvent::instance().setStep(1);

        executeBackup("link");
        cout << endl << endl  << "Linking accounts... ";

        auto baasDir = string(ACCOUNT_PATH) + "/baas";
        cout << "create=[" << baasDir << "]" << endl;
        filesystem::create_directories(baasDir);

        auto nasDir = string(ACCOUNT_PATH) + "/nas";
        cout << "create=[" << nasDir << "]" << endl;
        filesystem::create_directories(nasDir);

        ProgressEvent::instance().setStep(2);

        int c = 3;
        for (const auto& p: SharedSettings::instance().getSelectedProfiles()) {
            unlinkSingleAccount(p);

            Generator gen;
            cout << "Generating data for " << p.name << ": " << p.uid_str << std::endl;
            auto linkerFile = baasDir+"/"+p.uid_str+".dat";
            gen.writeBaas(linkerFile);

            auto profileDatFilename = nasDir + "/" + gen.nasIdStr() + ".dat";
            gen.writeProfileDat(profileDatFilename);

            auto profileJsonFilename = nasDir + "/" + gen.nasIdStr() + "_user.json";
            gen.writeProfileJson(profileJsonFilename);

            ProgressEvent::instance().setStep(c++);
        }

        cout << "Listing " << baasDir << endl;
        for (auto& entry: getDirContents(baasDir, ".dat")) {
            cout << "[" <<  entry.string() << "]" << endl;
        }
        cout << "Listing " << nasDir << endl;
        for (auto& entry: getDirContents(nasDir, ".dat")) {
            cout << "[" <<  entry.string() << "]" << endl;
        }
        for (auto& entry: getDirContents(nasDir, ".json")) {
            cout << "[" <<  entry.string() << "]" << endl;
        }

        ProgressEvent::instance().setStep(c++);
        cout << "Success!" << endl;

        unmountSaveData(acc, true);
        ProgressEvent::instance().setStep(c++);
    }
    catch (exception& e) // Not using filesystem_error since bad_alloc can throw too.
    {
        cout << "Failed! " << e.what() << endl;
        brls::Application::crash(string("Failed! ") + e.what());
    }
}

void unlinkAccounts()
{
    auto profileCount = SharedSettings::instance().getSelectedProfiles().size();
    if (profileCount == 0) {
        brls::Application::notify("translations/errors/no_accounts_selected"_i18n);
        brls::Application::popView();
        return;
    }
    try {
        ProgressEvent::instance().reset();
        ProgressEvent::instance().setTotalSteps(3+profileCount);

        FsFileSystem acc = mountSaveData();
        ProgressEvent::instance().setStep(1);

        executeBackup("unlink");
        ProgressEvent::instance().setStep(2);
        cout << endl << endl  << "Unlinking accounts... ";

        int c = 3;
        for (const auto& p: SharedSettings::instance().getSelectedProfiles()) {
            unlinkSingleAccount(p);
            ProgressEvent::instance().setStep(c++);
        }

        cout << "Success!" << endl;
        unmountSaveData(acc, true);
        ProgressEvent::instance().setStep(c);
    }
    catch (exception& e) // Not using filesystem_error since bad_alloc can throw too.
    {
        cout << "Failed! " << e.what() << endl;
        brls::Application::crash(string("Failed! ") + e.what());
    }
}

void manualBackup()
{
    try {
        ProgressEvent::instance().reset();
        ProgressEvent::instance().setTotalSteps(3);

        FsFileSystem acc = mountSaveData();
        ProgressEvent::instance().setStep(1);

        executeBackup("manual");
        cout << "Success!" << endl;

        unmountSaveData(acc, true);
        ProgressEvent::instance().setStep(3);
    }
    catch (exception& e) // Not using filesystem_error since bad_alloc can throw too.
    {
        cout << "Failed! " << e.what() << endl;
        brls::Application::crash(string("Failed! ") + e.what());
    }
}