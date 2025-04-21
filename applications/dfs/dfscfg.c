/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-04-07 09:21:50
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-04-21 17:12:11
 * @FilePath: \ele_ds\applications\dfs\dfscfg.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "dfscfg.h"
#include "ele_ds.h"

#define DBG_TAG "ele_ds"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

const struct dfs_mount_tbl mount_table[] =
{
        // {"norflash0", "/", "elm", 0, 0},
        {0},
};

/**
 * @description: 初始化配置文件
 * @param {ele_ds_cfg_t} *cfg
 * @return {*}
 */
void init_ele_ds_cfg(ele_ds_cfg_t *cfg)
{
    memset(cfg, 0, sizeof(ele_ds_cfg_t));
    cfg->check = CFGFILE_CHECK;
    cfg->cityid = CFGFILE_DEFATLT_CITYID;
    cfg->server_port = CFGFILE_DEFATLT_SERVER_PORT;

    strncpy((char *)cfg->version, SOFT_VERSION, sizeof(cfg->version) - 1);
    strncpy((char *)cfg->wifi_ssid, CFGFILE_DEFATLT_WIFI_SSID, sizeof(cfg->wifi_ssid) - 1);
    strncpy((char *)cfg->wifi_pass, CFGFILE_DEFATLT_WIFI_PASS, sizeof(cfg->wifi_pass) - 1);
    strncpy((char *)cfg->server_addr, CFGFILE_DEFATLT_SERVER_ADDR, sizeof(cfg->server_addr) - 1);
}

/**
 * @description: 读取配置文件
 * @param {ele_ds_cfg_t} *cfg
 * @return {*}
 */
int32_t read_ele_ds_cfg(ele_ds_cfg_t *cfg)
{
    int fd = open(CONFIG_FILE_PATH, O_RDONLY);
    if (fd < 0)
    {
        LOG_E("open %s failed", CONFIG_FILE_PATH);
        return -1;
    }
    read(fd, cfg, sizeof(ele_ds_cfg_t));
    close(fd);
    return 0;
}

/**
 * @description: 写入配置文件
 * @param {ele_ds_cfg_t} *cfg
 * @return {*}
 */
int32_t write_ele_ds_cfg(ele_ds_cfg_t *cfg)
{
    int fd = open(CONFIG_FILE_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0);
    if (fd < 0)
    {
        LOG_E("open %s failed", CONFIG_FILE_PATH);
        return -1;
    }
    write(fd, cfg, sizeof(ele_ds_cfg_t));
    close(fd);
    return 0;
}

/**
 * @description: 初始化系统文件, 如果 SYSFILE_PATH 不存在, 则初始化基本文件
 * @param {ele_ds_cfg_t} *cfg
 * @return {*}
 */
static int32_t init_sysfile(ele_ds_cfg_t *cfg)
{
    init_ele_ds_cfg(cfg);
    if (write_ele_ds_cfg(cfg) != 0)
    {
        LOG_E("write config file failed");
        return -1;
    }
    return 0;
}

/**
 * @description: 初始化系统文件, 如果 SYSFILE_PATH 不存在, 则初始化基本文件
 * @return {int32_t} 返回0表示成功, 返回-1表示失败
 */
static int32_t sysfile_init(void)
{
    // 检查文件是否存在
    if (access(CONFIG_FILE_PATH, R_OK) == -1)
    {
        LOG_D("filesystem need init, mkdir %s", SYSFILE_PATH);
        mkdir(SYSFILE_PATH, 0);
        init_sysfile(&g_ele_ds->device_cfg);
    }
    else
    {
        int fd = open(CONFIG_FILE_PATH, O_RDONLY);
        if (fd < 0)
        {
            LOG_E("open %s failed", CONFIG_FILE_PATH);
            return -2;
        }
        read(fd, &g_ele_ds->device_cfg, sizeof(ele_ds_cfg_t));
        close(fd);

        if (g_ele_ds->device_cfg.check != CFGFILE_CHECK)
        {
            LOG_E("config file check failed, need init");
            init_sysfile(&g_ele_ds->device_cfg);
            return -3;
        }
        LOG_D("filesystem already init");
    }
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
