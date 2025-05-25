/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-05-23 14:44:02
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-25 11:56:12
 * @FilePath: \ele_ds\applications\update_soft\update_soft.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "update_soft.h"
#include <fal.h>

#define DBG_TAG "upd_sft"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

const struct fal_partition *app_part = NULL;

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

#if 0 // 没用到 但是一直警告
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
#endif

/**
 * @brief 判断是否需要更新app
 * @return true表示需要更新，false表示不需要更新
 */
bool is_need_update(uint32_t *update_version, uint32_t *current_version)
{
    version_error_t ret;

    /* 获取update目录中的最新版本 */
    ret = get_latest_version(UPDATE_DIR, update_version);
    if (ret != VERSION_SUCCESS || *update_version == 0)
    {
        LOG_I("No update files found");
        return false;
    }

    /* 获取current目录中的最新版本 */
    ret = get_latest_version(CURRENT_DIR, current_version);
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
		
		
    /* 比较版本号 */
    if (*update_version <= *current_version)
    {
        LOG_I("No need to update: current=%08u, update=%08u", *current_version, *update_version);
        return false;
    }
    else
    {
        LOG_I("Need to update: current=%08u, update=%08u", *current_version, *update_version);
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
#if 0
/**
* @brief 向 APP 分区写入数据（需先擦除）
* @param offset 偏移地址（相对于分区起始地址）
* @param buffer 数据缓冲区
* @param size 写入字节数
* @return 成功返回 0，失败返回错误码
*/
static int fal_app_write(uint32_t offset, const uint8_t *buffer, uint32_t size) {
    if (!app_part || !buffer || offset + size > app_part->len)
    {
        LOG_E("Invalid parameters: offset=0x%x, size=%d", offset, size);
        return -RT_ERROR;
    }

    // 检查目标地址是否已擦除（Flash 写入前需擦除为全 1

）
    uint8_t check_buf[4] = {0};
    if (fal_read(app_part, offset, check_buf, sizeof(check_buf)) == RT_EOK)
    {
        if (check_buf[0] != 0xFF)
        {
            LOG_W("Target address not erased, need to erase first!");
            // 擦除示例：按页擦除（需根据实际 Flash 页大小调整）
            uint32_t page_size = app_part->flash_device->page_size;
            uint32_t start_erase = offset & ~(page_size - 1);
            uint32_t end_erase = offset + size + (page_size - 1);
            end_erase = end_erase > app_part->len ? app_part->len : end_erase;
            if (fal_partition_read(app_part, start_erase, end_erase - start_erase) != RT_EOK)
            {
                LOG_E("Erase APP partition failed");
                return -RT_ERROR;
            }
        }
    }

    // 调用 FAL 写入接口
    if (fal_partition_write(app_part, offset, buffer, size) != RT_EOK)
    {
        LOG_E("Write APP partition failed at 0x%x", offset);
        return -RT_ERROR;
    }
    LOG_D("Write APP partition success: offset=0x%x, size=%d", offset, size);
    return RT_EOK;
}
#else
/**
 * @brief 分块写入 APP 分区
 * @param offset 起始偏移地址
 * @param buffer 数据缓冲区
 * @param size 总写入字节数
 * @return 成功返回写入的字节数，失败返回负数错误码
 */
int fal_app_write(uint32_t offset, const uint8_t *buffer, uint32_t size)
{
    if (!app_part || !buffer || offset + size > app_part->len)
    {
        LOG_E("Invalid parameters: offset=0x%x, size=%d", offset, size);
        return -RT_ERROR;
    }

    LOG_I("Start writing large file: total_size=0x%x", size);

    // 写入当前块
    if (fal_partition_write(app_part, offset, buffer, size) < 0)
    {
        LOG_E("Write failed at offset 0x%x, block_size=0x%x", offset, size);
        return -RT_ERROR;
    }

    LOG_I("Large file write success: written=0x%x", size);
    return size;
}
#endif

/**
 * @brief 分块读取 APP 分区（支持大文件）
 * @param offset 起始偏移地址
 * @param buffer 数据缓冲区（需足够大）
 * @param size 总读取字节数
 * @return 成功返回读取的字节数，失败返回负数错误码
 */
int fal_app_read_large(uint32_t offset, uint8_t *buffer, uint32_t size) {
    if (!app_part || !buffer || offset + size > app_part->len) {
        LOG_E("Invalid parameters: offset=0x%x, size=%d", offset, size);
        return -RT_ERROR;
    }

    const uint32_t block_size = 4096;  // 每次读取 4KB
    uint32_t total_read = 0;
    uint32_t remaining = size;
    uint32_t current_offset = offset;
    uint8_t *current_buffer = buffer;

    LOG_I("Start reading large file: total_size=0x%x", size);

    // 按块循环读取
    while (remaining > 0) {
        uint32_t read_size = (remaining > block_size) ? block_size : remaining;
        
        // 读取当前块
        if (fal_partition_read(app_part, current_offset, current_buffer, read_size) != RT_EOK)
        {
            LOG_E("Read failed at offset 0x%x, block_size=0x%x, total_read = %d, remaining = %d",
                  current_offset, read_size, total_read, remaining);
            return -RT_ERROR;
        }

        // 更新进度
        total_read += read_size;
        remaining -= read_size;
        current_offset += read_size;
        current_buffer += read_size;
        
        LOG_D("Read block: progress=%d%%", (total_read * 100) / size);
    }

    LOG_I("Large file read success: read=0x%x", total_read);
    return total_read;
}

/**
 * @brief 从文件系统读取文件并写入 APP 分区
 * @param filename 文件系统中的文件名
 * @param flash_offset Flash 写入偏移
 * @return 成功返回0，失败返回负数错误码
 */
int flash_write_file(const char *filename, uint32_t flash_offset)
{
    int32_t ret = 0;
    int fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        LOG_E("Failed to open file: %s", filename);
        return -RT_ERROR;
    }

    // 获取文件大小
    struct stat st;
    if (fstat(fd, &st) != 0)
    {
        LOG_E("Failed to get file size");
        close(fd);
        return -RT_ERROR;
    }
    uint32_t file_size = st.st_size;
    LOG_I("File size: 0x%x bytes", file_size);
    
    app_part = fal_partition_find("app");
    if (!app_part)
    {
        LOG_E("Failed to find app partition");
        close(fd);
        return -RT_ERROR;
    }
    ret = fal_partition_erase_all(app_part); // 擦除分区
    if (ret < 0)
    {
        LOG_E("Failed to erase app partition");
        close(fd);
        return ret;
    }
    
    // 分配缓冲区（分块读取，避免内存不足）
    const uint32_t buffer_size = 4096; // 4KB 缓冲区
    uint8_t *buffer = rt_malloc(buffer_size);
    if (!buffer)
    {
        LOG_E("Failed to allocate memory");
        close(fd);
        return -RT_ENOMEM;
    }

    uint32_t total_written = 0;
    uint32_t remaining = file_size;
    uint32_t current_flash_offset = flash_offset;
    int result = RT_EOK;

    // 分块读取文件并写入Flash
    while (remaining > 0)
    {
        uint32_t read_size = (remaining > buffer_size) ? buffer_size : remaining;
        int bytes = read(fd, buffer, read_size);
        if (bytes <= 0)
        {
            LOG_E("Failed to read file at offset 0x%x", total_written);
            result = -RT_ERROR;
            break;
        }

        // 写入Flash
        int write_result = fal_app_write(current_flash_offset, buffer, bytes);
        if (write_result < 0)
        {
            LOG_E("Failed to write to flash at offset 0x%x, block_size=0x%x, total_written = %d, remaining = %d, fail result = %d",
                current_flash_offset, read_size, total_written, remaining, write_result);
            result = write_result;
            break;
        }

        // 更新进度
        total_written += bytes;
        remaining -= bytes;
        current_flash_offset += bytes;
        LOG_D("Progress: %d%%", (total_written * 100) / file_size);
    }

    rt_free(buffer);
    close(fd);
    return result;
}

/**
 * @description: 更新应用程序
 * @param {uint32_t} update_version 更新版本号
 * @param {uint32_t} current_version 当前版本号
 * @return {*}
 */ 
void update_app(uint32_t update_version, uint32_t current_version)
{
    char update_path[VERSION_BUFF_SIZE] = {0};
    char current_path[VERSION_BUFF_SIZE] = {0};
    char old_path[VERSION_BUFF_SIZE] = {0};
    char backup_path[VERSION_BUFF_SIZE] = {0};

    /* 需要更新，执行文件复制 */
    rt_snprintf(update_path, VERSION_BUFF_SIZE, "%s/%08u", UPDATE_DIR, update_version);
    rt_snprintf(current_path, VERSION_BUFF_SIZE, "%s/%08u", CURRENT_DIR, update_version);
    rt_snprintf(old_path, VERSION_BUFF_SIZE, "%s/%08u", CURRENT_DIR, current_version);
    rt_snprintf(backup_path, VERSION_BUFF_SIZE, "%s/%08u", BACKUP_DIR, current_version);

    LOG_D("Update path: %s", update_path);
    LOG_D("Current path: %s", current_path);
    LOG_D("Old path: %s", old_path);
    LOG_D("Backup path: %s", backup_path);

    // 删除旧版本目录后移动旧版本到backup, 删除backup目录是为了保障backup目录下只有一份旧版本
    rmdir(BACKUP_DIR);
    mkdir(BACKUP_DIR, 0);
// rt_thread_mdelay(100);
#if 1
    move_file(old_path, backup_path);
    
    // 移动更新包到current, 然后烧写到flash
    move_file(update_path, current_path);
    // 写入到flash
    flash_write_file(current_path, 0); // 从app分区的0地址开始写入
#else // 测试烧写到flash是否正常
    flash_write_file(update_path, 0); // 从app分区的0地址开始写入
#endif
}


void create_test_file(void)
{
    char path[VERSION_BUFF_SIZE] = {0};

    // 删除 SOFT_DIR 
    unlink(SOFT_DIR);
    
    // 创建文件夹
    mkdir(ROOT_DIR, 0777);
    mkdir(SOFT_DIR, 0777);
    mkdir(UPDATE_DIR, 0777);
    mkdir(CURRENT_DIR, 0777);
    mkdir(BACKUP_DIR, 0777);

    // 创建文件
    sprintf(path, "%s/%s", UPDATE_DIR, "00000001");
    int32_t fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    write(fd, "00000001", 8);
    sprintf(path, "%s/%s", CURRENT_DIR, "00000000");
    int32_t fd2 = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    write(fd2, "00000000", 8);
    close(fd);
    close(fd2);
}
MSH_CMD_EXPORT_ALIAS(create_test_file, create_test_file, create test file in bootloader);

