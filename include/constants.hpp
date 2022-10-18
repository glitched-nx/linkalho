#ifndef __LINKALHO_CONSTANTS_HPP__
#define __LINKALHO_CONSTANTS_HPP__

// #define DEBUG
#define LOCAL_PATH "sdmc:/switch/" APP_TITLE_LOWER "/"
#define BACKUP_PATH LOCAL_PATH "backups/"
#define RESTORE_FILE_DIR LOCAL_PATH "restore/"
#define RESTORE_FILE_PATH RESTORE_FILE_DIR "restore.zip"

#ifdef DEBUG
# define ACCOUNT_PATH LOCAL_PATH "debug"
#else
# define ACCOUNT_PATH "account:/su"
#endif
# define RESTORE_PATH ACCOUNT_PATH

#define CUSTOM_PAYLOAD_FILE "reboot.bin"
#define CUSTOM_PAYLOAD_FILE_PATH LOCAL_PATH CUSTOM_PAYLOAD_FILE

#endif // __LINKALHO_CONSTANTS_HPP__
