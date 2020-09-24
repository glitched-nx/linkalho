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
#include "custom_style.hpp"
#include "custom_theme.hpp"
#include "progress_event.hpp"
#include "utils.hpp"

using namespace std;


int main(int argc, char* argv[])
{
    pmshellInitialize();
    bool canUseLed = false;
    if (R_SUCCEEDED(hidsysInitialize())) {
        canUseLed = true;
    }
    initDirs();

    if (!brls::Application::init(APP_TITLE_LOWER, CustomStyle::LinkalhoStyle(), CustomTheme::LinkalhoTheme()))
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
            new ConfirmPage(stagedFrame, "Linking all accounts will overwrite all previous links.\n(Your saves will be preserved)\n\nIf you had any previosly linked NNID account, it will be erased!", false, canUseLed)
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
            new ConfirmPage(stagedFrame, "Unlinking accounts will reset all users.\n(Your saves will be preserved)\n\nIf you had any previosly linked NNID account, it will be erased!", false, canUseLed)
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
    info_ss << "Reboot to payload " << (getPayload().empty() ? "in" : "" ) << "active";

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
