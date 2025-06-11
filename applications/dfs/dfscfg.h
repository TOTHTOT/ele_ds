/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-04-05 12:21:47
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-27 11:33:23
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
#include "client.h"

#define SYSFILE_PATH "/sysfile"
#define CONFIG_FILE_PATH SYSFILE_PATH "/config"
#define SOFTWARE_UPDATE_FILE_PATH SYSFILE_PATH
typedef struct
{
    ele_client_cfg_t clientcfg; // 设备登录到服务器的配置信息
#define CFGFILE_DEFATLT_WIFI_SSID "esp-2.4G"
#define CFGFILE_DEFATLT_WIFI_PASS "12345678.."
    uint8_t wifi_ssid[32];   // wifi ssid
    uint8_t wifi_passwd[32]; // wifi 密码

#define CFGFILE_DEFATLT_SERVER_ADDR "60.215.128.73"
#define CFGFILE_DEFATLT_SERVER_PORT 12675
    uint8_t server_addr[64]; // 服务器地址
    uint16_t server_port;    // 服务器端口

    weather_type_t curweather;
    weather_info_t weather_info[7];

#define CLIENT_CHEAT_CONTENT_SIZE 256
    char memo[CLIENT_CHEAT_CONTENT_SIZE]; // 备忘录消息

#define CFGFILE_DEFAULT_TCP_TIMEOUT (3 * 1000) // 3s
    uint32_t tcp_timeout; // tcp传输数据软件定时器超时时间

    bool alarm_enable; // 是否启用闹钟, 默认false
#define CFGFILE_DEFAULT_ALARM_TIME 0 // 默认闹钟时间为0
    time_t alarm_time; // 闹钟响铃时间

#define CFGFILE_CHECK 0xA5A5A5A5
    uint32_t check;
} ele_ds_cfg_t; // 掉电信息

extern int mnt_init(void);
extern int32_t write_ele_ds_cfg(ele_ds_cfg_t *cfg);
extern int32_t read_ele_ds_cfg(ele_ds_cfg_t *cfg);

#endif /* __DFSCFG_H__ */
