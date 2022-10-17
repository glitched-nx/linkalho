#include <borealis/application.hpp>
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
#include "generator.hpp"
#include "progress_event.hpp"

using namespace std;
using namespace zipper;

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
#ifndef DEBUG
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
#ifndef DEBUG
    if (commit) {
        fsdevCommitDevice("account");
    }
    fsdevUnmountDevice("account");
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

void linkAccount()
{
    try {
        ProgressEvent::instance().reset();
        ProgressEvent::instance().setTotalSteps(6);

        FsFileSystem acc = mountSaveData();
        ProgressEvent::instance().setStep(1);

        executeBackup("link");
        ProgressEvent::instance().setStep(2);
        cout << endl << endl  << "Linking accounts... ";

        auto baasDir = string(ACCOUNT_PATH) + "/baas";
        // cout << "create=[" << baasDir << "]" << endl;
        filesystem::create_directories(baasDir);
        filesystem::remove_all(baasDir);
        ProgressEvent::instance().setStep(3);

        auto nasDir = string(ACCOUNT_PATH) + "/nas";
        // cout << "create=[" << nasDir << "]" << endl;
        filesystem::create_directories(nasDir);
        filesystem::remove_all(nasDir);
        ProgressEvent::instance().setStep(4);

        for (auto& entry: getDirContents(ACCOUNT_PATH, ".jpg")) {

            auto linkerFile = baasDir+"/"+entry.stem().string()+".dat";
            Generator::instance().writeBaas(linkerFile);

            auto profileDatFilename = nasDir + "/" + Generator::instance().nasIdStr() + ".dat";
            Generator::instance().writeProfileDat(profileDatFilename);

            auto profileJsonFilename = nasDir + "/" + Generator::instance().nasIdStr() + "_user.json";
            Generator::instance().writeProfileJson(profileJsonFilename);
        }
        ProgressEvent::instance().setStep(5);
        cout << "Success!" << endl;

        unmountSaveData(acc, true);
        ProgressEvent::instance().setStep(6);
    }
    catch (exception& e) // Not using filesystem_error since bad_alloc can throw too.
    {
        cout << "Failed! " << e.what() << endl;
        brls::Application::crash(string("Failed! ") + e.what());
    }
}

void unlinkAccount()
{
    try {
        ProgressEvent::instance().reset();
        ProgressEvent::instance().setTotalSteps(5);

        FsFileSystem acc = mountSaveData();
        ProgressEvent::instance().setStep(1);

        executeBackup("unlink");
        ProgressEvent::instance().setStep(2);
        cout << endl << endl  << "Unlinking accounts... ";

        auto baasDir = string(ACCOUNT_PATH) + "/baas";
        filesystem::remove_all(baasDir);
        ProgressEvent::instance().setStep(3);

        auto nasDir = string(ACCOUNT_PATH) + "/nas";
        filesystem::remove_all(nasDir);
        ProgressEvent::instance().setStep(4);

        cout << "Success!" << endl;
        unmountSaveData(acc, true);
        ProgressEvent::instance().setStep(5);
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