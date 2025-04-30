/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-04-05 12:21:47
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-04-21 17:54:27
 * @FilePath: \ele_ds\applications\dfs\dfscfg.h
 * @Description: dfs文件系统相关配置
 */
#ifndef __DFSCFG_H__
#define __DFSCFG_H__

#include <dfs_fs.h>
#include <stdint.h>
#include <unistd.h>
#include <stdint.h>
#include "weather.h"

#define SYSFILE_PATH "/sysfile"
#define CONFIG_FILE_PATH SYSFILE_PATH "/config"

typedef struct
{
#define CFGFILE_DEFATLT_WIFI_SSID "esp-2.4G"
#define CFGFILE_DEFATLT_WIFI_PASS "12345678.."
    uint8_t wifi_ssid[32]; // wifi ssid
    uint8_t wifi_passwd[32]; // wifi 密码

#define CFGFILE_DEFATLT_SERVER_ADDR "api.seniverse.com"
#define CFGFILE_DEFATLT_SERVER_PORT 24680
    uint8_t server_addr[64]; // 服务器地址
    uint16_t server_port;    // 服务器端口

#define CFGFILE_DEFATLT_CITYID 101230101
    uint32_t cityid;
    weather_type_t curweather;
    weather_info_t weather_info[7];

    uint8_t version[16];
#define CFGFILE_CHECK 0xA5A5A5A5
    uint32_t check;
} ele_ds_cfg_t; // 掉电信息

extern int mnt_init(void);
extern int32_t write_ele_ds_cfg(ele_ds_cfg_t *cfg);
extern int32_t read_ele_ds_cfg(ele_ds_cfg_t *cfg);

#endif /* __DFSCFG_H__ */
