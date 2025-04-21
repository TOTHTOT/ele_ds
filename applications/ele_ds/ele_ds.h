/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-02-16 19:11:27
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-04-21 16:32:53
 * @FilePath: \ele_ds\applications\ele_ds\ele_ds.h
 * @Description: 电子桌搭主要功能
 */
#ifndef __ELE_DS_H__

#include <board.h>
#include <stdint.h>
#include <rtdevice.h>
#include <drv_gpio.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <dfscfg.h>
#include "gzp6816d_driver.h"
#ifdef PKG_USING_SHT3X
#include "sht3x.h"
#endif /* PKG_USING_SHT3X */
#ifdef PKG_USING_SGP30
#include "sgp30.h"
#endif /* PKG_USING_SHT3X */
#include "DEV_Config.h"
#include "EPD_Test.h"
#include "EPD_2in7_V2.h"
#include "dfscfg.h"
#include "common.h"
/* 参数配置 */
#define LED0_PIN GET_PIN(C, 13)
#define V3_3_PIN GET_PIN(C, 8)
#define MAX_VBAT 4.2f
#define MIN_VBAT 3.6f
#define SOFT_VERSION "V1.0.0"

typedef rt_err_t (*get_sensor_data)(void *data);

typedef enum
{
#ifdef PKG_USING_GZP6816D_SENSOR
    SENSOR_GZP6816D_INDEX = 0,
#endif /* PKG_USING_GZP6816D_SENSOR */

#ifdef PKG_USING_SHT3X
    SENSOR_SHT3X_INDEX,
#endif /* PKG_USING_SHT3X */

#ifdef PKG_USING_SGP30
    SENSOR_SGP30_INDEX,
#endif /* PKG_USING_SGP30 */
    SENSOR_MAX
} sensor_index_t;
struct ele_ds_ops
{
    get_sensor_data sensor_data[SENSOR_MAX + 1];
};
typedef struct ele_ds_ops ele_ds_ops_t;

typedef enum
{
    WEATHER_BAOXUE,
    WEATHER_BAOYU,
    WEATHER_BINGBAO,
    WEATHER_BINGJING,
    WEATHER_BINGLI,
    WEATHER_DAXUE,
    WEATHER_DAYU,
    WEATHER_DAWU,
    WEATHER_DUOYUN,
    WEATHER_DUOYUN_YE,
    WEATHER_FUCHEN,
    WEATHER_LEIBAO,
    WEATHER_LEIYU,
    WEATHER_MAI,
    WEATHER_QING,
    WEATHER_QING_YE,
    WEATHER_QINGWU,
    WEATHER_SHACHENBAO,
    WEATHER_XIAOXUE,
    WEATHER_XIAOYU,
    WEATHER_XUEMI,
    WEATHER_YANGSHA,
    WEATHER_YIN,
    WEATHER_YUJIAXUE,
    WEATHER_ZHENYU,
    WEATHER_ZHONGWU,
    WEATHER_ZHONGXUE,
    WEATHER_ZHONGYU,

    WEATHER_TYPE_COUNT
} weather_type_t; // 天气类型

// 天气信息
typedef struct weather_info
{
    char fxDate[11];        // 日期 (如 "2025-03-20")
    char sunrise[6];        // 日出时间 (如 "06:19")
    char sunset[6];         // 日落时间 (如 "18:26")
    char moonrise[6];       // 月出时间 (允许为空)
    char moonset[6];        // 月落时间 (如 "08:53")
    char moonPhase[16];     // 月相描述 (如 "亏凸月")
    char moonPhaseIcon[4];  // 月相图标 ID (如 "805")
    int8_t tempMax;         // 最高温度 (如 "24")
    int8_t tempMin;         // 最低温度 (如 "10")
    char iconDay[4];        // 白天图标 ID (如 "100")
    char textDay[16];       // 白天天气描述 (如 "晴")
    char iconNight[4];      // 夜间图标 ID (如 "150")
    char textNight[16];     // 夜间天气描述 (如 "晴")
    uint16_t wind360Day;    // 白天风向角度 (如 "315")
    char windDirDay[16];    // 白天风向 (如 "西北风")
    char windScaleDay[8];   // 白天风力等级 (如 "1-3")
    uint8_t windSpeedDay;   // 白天风速 (如 "3")
    uint16_t wind360Night;  // 夜间风向角度 (如 "270")
    char windDirNight[16];  // 夜间风向 (如 "西风")
    char windScaleNight[8]; // 夜间风力等级 (如 "1-3")
    uint8_t windSpeedNight; // 夜间风速 (如 "3")
    uint8_t humidity;       // 湿度 (如 "21")
    float precip;           // 降水量 (如 "0.0")
    uint16_t pressure;      // 气压 (如 "1009")
    uint8_t vis;            // 能见度 (如 "25")
    uint8_t cloud;          // 云量 (如 "0")
    uint8_t uvIndex;        // 紫外线指数 (如 "5")
}weather_info_t; // 天气信息

typedef struct
{
#define CFGFILE_DEFATLT_WIFI_SSID "esp32-2.4G"
#define CFGFILE_DEFATLT_WIFI_PASS "12345678.."
    uint8_t wifi_ssid[32]; // wifi ssid
    uint8_t wifi_pass[32]; // wifi 密码

#define CFGFILE_DEFATLT_SERVER_ADDR "api.seniverse.com"
#define CFGFILE_DEFATLT_SERVER_PORT 24680
    uint8_t server_addr[64]; // 服务器地址
    uint16_t server_port;    // 服务器端口

#define CFGFILE_DEFATLT_CITYID 101230101
    uint32_t cityid;
    weather_type_t curweather;
    weather_info_t weather_info;

    uint8_t version[16];
#define CFGFILE_CHECK 0xA5A5A5A5
    uint32_t check;
} ele_ds_cfg_t; // 掉电信息

struct ele_ds
{
    struct
    {
        rt_device_t gzp6816d_dev;
#ifdef PKG_USING_SHT3X
        sht3x_device_t sht3x_dev;
#endif /* PKG_USING_SHT3X */
        struct rt_spi_device *epaper_dev;
    } devices; // 设备
    struct
    {
        float sht30[2];   // 温湿度传感器数据, 0 温度, 1 湿度
        int32_t sgp30[2]; // 空气质量传感器数据, 0 TVOC, 1 eCO2
#ifdef PKG_USING_GZP6816D_SENSOR
        gzp6816d_data_t gzp6816d;
#endif
    } sensor_data;  // 传感器数据
    bool init_flag; // 系统是否初始化成功, == true 表示初始化成功, == false 表示初始化失败

    struct 
    {
        bool cnt_wifi;
        float current_vbat;
        time_t current_time;
    }device_status;
    
    ele_ds_cfg_t device_cfg; // 设备配置
    
    struct ele_ds_ops ops;
};
typedef struct ele_ds *ele_ds_t;

extern int32_t devices_init(ele_ds_t ele_ds);
extern ele_ds_t g_ele_ds;
extern rt_err_t get_sht30_data(void *para);
extern rt_err_t get_sgp30_data(void *para);
extern int rt_hw_sgp30_port(void);
#endif /* __ELE_DS_H__ */
