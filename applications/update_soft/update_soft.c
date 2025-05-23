/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-05-23 14:44:02
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-23 17:31:47
 * @FilePath: \ele_ds\applications\update_soft\update_soft.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "update_soft.h"

#include <rtthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#define DBG_TAG "upd_sft"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
/**
 * @brief 从路径中提取版本号
 * @param path 文件完整路径
 * @return 版本号数值，失败返回 0
 */
static uint32_t extract_version_from_path(const char *path)
{
    const char *filename = strrchr(path, '/');
    if (!filename)
        filename = path;
    else
        filename++;

    return strtoul(filename, NULL, 10);
}

/**
 * @brief 获取目录中最新的版本号
 * @param dir_path 目录路径
 * @param[out] latest_version 输出的最新版本号
 * @return 错误码
 */
static version_error_t get_latest_version(const char *dir_path, uint32_t *latest_version)
{
    DIR *dir;
    struct dirent *entry;
    uint32_t max_version = 0;

    *latest_version = 0;

    /* 打开目录 */
    dir = opendir(dir_path);
    if (!dir)
    {
        LOG_W("Failed to open directory: %s", dir_path);
        return VERSION_ERROR_OPEN_DIR;
    }

    /* 遍历目录查找最大版本号 */
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        uint32_t version = extract_version_from_path(entry->d_name);
        if (version > max_version)
        {
            max_version = version;
        }
    }

    closedir(dir);
    *latest_version = max_version;
    return VERSION_SUCCESS;
}

/**
 * @brief 复制文件
 * @param src_path 源文件路径
 * @param dst_path 目标文件路径
 * @return 错误码
 */
static version_error_t copy_file(const char *src_path, const char *dst_path)
{
    int src_fd, dst_fd;
    char buffer[FILE_COPY_BUFFER_SIZE];
    ssize_t bytes;

    /* 打开源文件 */
    src_fd = open(src_path, O_RDONLY);
    if (src_fd < 0)
    {
        LOG_E("Failed to open source file: %s", src_path);
        return VERSION_ERROR_OPEN_FILE;
    }

    /* 创建目标文件 */
    dst_fd = open(dst_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst_fd < 0)
    {
        LOG_E("Failed to create destination file: %s", dst_path);
        close(src_fd);
        return VERSION_ERROR_OPEN_FILE;
    }

    /* 复制文件内容 */
    while ((bytes = read(src_fd, buffer, sizeof(buffer))) > 0)
    {
        if (write(dst_fd, buffer, bytes) != bytes)
        {
            LOG_E("Failed to write file");
            close(src_fd);
            close(dst_fd);
            unlink(dst_path);
            return VERSION_ERROR_WRITE_FILE;
        }
    }

    close(src_fd);
    close(dst_fd);
    return VERSION_SUCCESS;
}

/**
 * @brief 判断是否需要更新app
 * @return true表示需要更新，false表示不需要更新
 */
bool is_need_update(char update_path[VERSION_BUFF_SIZE], char current_path[VERSION_BUFF_SIZE])
{
    uint32_t update_version = 0, current_version = 0;
    version_error_t ret;

    /* 获取update目录中的最新版本 */
    ret = get_latest_version(UPDATE_DIR, &update_version);
    if (ret != VERSION_SUCCESS || update_version == 0)
    {
        LOG_I("No update files found");
        return false;
    }

    /* 获取current目录中的最新版本 */
    ret = get_latest_version(CURRENT_DIR, &current_version);
    if (ret == VERSION_ERROR_OPEN_DIR)
    {
        LOG_I("Current directory not found, need update");
        /* 没有current目录，需要更新 */
    }
    else if (ret != VERSION_SUCCESS)
    {
        LOG_E("Failed to read current directory");
        return false;
    }
		
		/* 需要更新，执行文件复制 */
    rt_snprintf(update_path, VERSION_BUFF_SIZE, "%s/%08u", UPDATE_DIR, update_version);
    rt_snprintf(current_path, VERSION_BUFF_SIZE, "%s/%08u", CURRENT_DIR, update_version);
    LOG_D("Update path: %s", update_path);
    LOG_D("Current path: %s", current_path);
		
    /* 比较版本号 */
    if (update_version <= current_version)
    {
        LOG_I("No need to update: current=%08u, update=%08u", current_version, update_version);
        return false;
    }
    else
    {
        LOG_I("Need to update: current=%08u, update=%08u", current_version, update_version);
        return true;
    }
}

/**
 * @brief 移动文件（重命名）
 * @param src_path 源文件路径
 * @param dst_path 目标文件路径
 * @return 错误码
 */
version_error_t move_file(const char *src_path, const char *dst_path)
{
    /* 尝试重命名文件（相当于移动） */
    if (rename(src_path, dst_path) != 0) {
        LOG_E("Failed to move file from %s to %s", src_path, dst_path);
        return VERSION_ERROR_OPEN_FILE;
    }
    
    LOG_D("File moved successfully: %s -> %s", src_path, dst_path);
    return VERSION_SUCCESS;
}

/**
 * @brief 删除文件
 * @param path 文件路径
 * @return 错误码
 */
version_error_t delete_file(const char *path)
{
    /* 尝试删除文件 */
    if (unlink(path) != 0) {
        LOG_E("Failed to delete file: %s", path);
        return VERSION_ERROR_OPEN_FILE;
    }
    
    LOG_D("File deleted successfully: %s", path);
    return VERSION_SUCCESS;
}
