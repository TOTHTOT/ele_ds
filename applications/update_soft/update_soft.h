#ifndef __UPDATE_SOFT_H__
#define __UPDATE_SOFT_H__

#include "common.h"
#include <dfs_fs.h>
#include <rtthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

/* 版本管理模块 - 常量定义 */
#define ROOT_DIR       "/sysfile"
#define SOFT_DIR        ROOT_DIR "/soft"
#define UPDATE_DIR      SOFT_DIR "/update"
#define CURRENT_DIR     SOFT_DIR "/current"
#define BACKUP_DIR      SOFT_DIR "/backup"
#define VERSION_BUFF_SIZE 64
#define FILE_COPY_BUFFER_SIZE 512

/* 版本管理模块 - 错误码定义 */
typedef enum {
    VERSION_SUCCESS = 0,
    VERSION_ERROR_OPEN_DIR = -1,
    VERSION_ERROR_READ_DIR = -2,
    VERSION_ERROR_OPEN_FILE = -3,
    VERSION_ERROR_READ_FILE = -4,
    VERSION_ERROR_WRITE_FILE = -5,
    VERSION_ERROR_CREATE_DIR = -6,
    VERSION_ERROR_PARSE = -7,
} version_error_t;

extern bool is_need_update(uint32_t *update_version, uint32_t *current_version);
extern version_error_t move_file(const char *src_path, const char *dst_path);
extern version_error_t delete_file(const char *path);
extern void update_app(uint32_t update_version, uint32_t current_version);

#endif /* __UPDATE_SOFT_H__ */

