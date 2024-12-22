#pragma once

// #define LINKALHO_DEBUG
#define LOCAL_PATH "sdmc:/switch/" APP_TITLE_LOWER "/"
#define BACKUP_PATH LOCAL_PATH "sdmc:/linkalho/backups/"
#define RESTORE_FILE_DIR LOCAL_PATH "sdmc:/linkalho/restore/"
#define RESTORE_FILE_PATH RESTORE_FILE_DIR "restore.zip"

#ifdef LINKALHO_DEBUG
# define ACCOUNT_PATH LOCAL_PATH "debug"
#else
# define ACCOUNT_PATH "account:/su"
#endif

#define CUSTOM_PAYLOAD_FILE "update.bin"
#define CUSTOM_PAYLOAD_FILE_PATH "sdmc:/bootloader/" CUSTOM_PAYLOAD_FILE

#define DEFAULT_COUNTRY "Germany"
#define DEFAULT_COUNTRY_CODE "DE"
#define DEFAULT_LOCALE "de-DE"
#define DEFAULT_TIMEZONE "Europe/Berlin"
