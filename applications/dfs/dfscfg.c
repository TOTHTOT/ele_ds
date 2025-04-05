#include "dfscfg.h"


#define DBG_TAG "ele_ds"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

const struct dfs_mount_tbl mount_table[] =
{
        // {"norflash0", "/", "elm", 0, 0},
        {0},
};

/**
 * @description: 初始化系统文件, 如果 SYSFILE_PATH 不存在, 则初始化基本文件
 * @return {int32_t} 返回0表示成功, 返回-1表示失败
 */
static int32_t sysfile_init(void)
{
    DIR *dir = opendir(SYSFILE_PATH );
    if (dir == NULL)
    {
        LOG_D("filesystem need init, mkdir %s", SYSFILE_PATH);
        mkdir(SYSFILE_PATH, 0);
        dir = opendir(SYSFILE_PATH );
        int fd = open(SYSFILE_PATH "/sysfile.txt", O_WRONLY | O_CREAT | O_TRUNC, 0);
        write(fd, "sysfile", 7);
        close(fd);
    }
    else
    {
        LOG_D("filesystem already init");
    }
    closedir(dir);
    return 0;
}

/**
 * @description: 挂载文件系统
 * @return {int} 挂载成功返回0, 失败返回-1
 */
int mnt_init(void)
{
    if (dfs_mount("norflash0", "/", "elm", 0, 0) == 0)  // "norflash0":挂载的设备名称, "/":挂载路径, 这里挂载到跟目录下
    {
        LOG_D("norflash0 mount successful!");
    }
    else
    {
        dfs_mkfs("elm", "norflash0");  // 如果是第一次挂载文件系统必须要先格式化
        if(dfs_mount("norflash0", "/", "elm", 0, 0) != 0)
        {
            LOG_D("norflash0 mount failed!");
        }
        else
        {
            LOG_D("norflash0 mkfs successful!");
            // 第一次挂载文件系统，需要初始化系统文件
        }
    }
    sysfile_init();

    return 0;
}