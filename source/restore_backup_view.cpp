#include "restore_backup_view.hpp"
#include <string>
#include <sys/stat.h>
#include "confirm_page.hpp"
#include "worker_page.hpp"
#include "file_operations.hpp"
#include "constants.hpp"

using namespace std;
using namespace brls::i18n::literals;

RestoreBackupView::RestoreBackupView(bool canUseLed) : ListItem("translations/restore_view/title"_i18n)
{
    struct stat buffer;
    bool backupExists = (stat(RESTORE_FILE_PATH, &buffer) == 0);

    if (backupExists)
    {
        this->getClickEvent()->subscribe([canUseLed](brls::View* view) {
            brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
            stagedFrame->setTitle("translations/restore_view/title"_i18n);

            stagedFrame->addStage(
                new ConfirmPage(stagedFrame, "translations/restore_view/restore_warning"_i18n, false, canUseLed)
            );
            stagedFrame->addStage(
                new WorkerPage(stagedFrame, "translations/restore_view/restoring"_i18n, [](){
                    restoreBackup(RESTORE_FILE_PATH);
                })
            );
            stagedFrame->addStage(
                new ConfirmPage(stagedFrame, "translations/restore_view/restore_complete"_i18n, true, canUseLed)
            );

            brls::Application::pushView(stagedFrame);
            stagedFrame->registerAction("", brls::Key::PLUS, []{return true;}, true);
            stagedFrame->updateActionHint(brls::Key::PLUS, ""); // make the change visible
            stagedFrame->registerAction("", brls::Key::B, []{return true;}, true);
            stagedFrame->updateActionHint(brls::Key::B, ""); // make the change visible
        });
    } else {
        this->getClickEvent()->subscribe([](brls::View* view) {
            brls::Dialog* dialog = new brls::Dialog(fmt::format("translations/restore_view/not_found_dialog_text"_i18n, RESTORE_FILE_PATH));

            dialog->addButton(fmt::format("translations/restore_view/not_found_dialog_btn"_i18n), [dialog](brls::View* view) {
                dialog->close();
            });
            dialog->open();
            dialog->registerAction("", brls::Key::PLUS, []{return true;}, true);
            dialog->updateActionHint(brls::Key::PLUS, ""); // make the change visible
        });
        this->setValue(fmt::format("translations/restore_view/not_found_dialog_hint"_i18n), true, false);
    }
}