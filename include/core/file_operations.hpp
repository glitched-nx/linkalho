#pragma once

#include <filesystem>
#include <vector>

bool initDirs();
void linkAccount();
void unlinkAccount();
void restoreBackup(const std::string& backupFullpath);
void manualBackup();
