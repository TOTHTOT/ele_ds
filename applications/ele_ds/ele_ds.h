/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-02-16 19:11:27
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-28 14:05:43
 * @FilePath: \ele_ds\applications\ele_ds\ele_ds.h
 * @Description: 电子桌搭主要功能
 */
#ifndef __ELE_DS_H__
#define __ELE_DS_H__
#include <board.h>
#include <stdint.h>
#include <rtdevice.h>
#include <drv_gpio.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <dfscfg.h>
#include "gzp6816d_driver.h"
#include "client.h"

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
#define LEFT_KEY GET_PIN(A, 0)
#define MID_KEY GET_PIN(B, 13)
#define RIGHT_KEY GET_PIN(B, 12)

#define BAT_ADC_CH 9 // 电池电压adc通道
#define MAX_VBAT 4.2f
#define MIN_VBAT 3.0f
#define ADC_REFVAL 3.3f
#define ADC_CONVERT_MAXVAL 4096 // 采样最大值

/**
 * 关于ui的一些宏定义
 */
// 默认图标保存位置
#define DEFATLT_WEATHER_ICON_PATH "S:/sysfile/icon/tianqi_48/tianqi-%s.bin"
#define DEFAULT_SHT30_LABFMT "温度:%2.0fC\n湿度:%2.0f%%"
#define DEFAULT_GZP6816_LABFMT "气压:%2.2fhPa"

#define SOFT_VERSION 0x25052801
#define DEFATLT_CITYID 101010100
/**
 * 1: 打开epd测试, 这时不等待 EPD_BUSY_PIN, 切不刷新屏幕只串口打印显存内容, 0: 关闭epd测试
 */
#define EPD_ON_TEST 1

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
    int32_t (*get_curvbat)(ele_ds_t dev);
};
typedef struct ele_ds_ops ele_ds_ops_t;


struct ele_ds
{
    struct
    {
        struct at_device_esp8266 esp8266;
        rt_device_t gzp6816d_dev;
#ifdef PKG_USING_SHT3X
        sht3x_device_t sht3x_dev;
#endif /* PKG_USING_SHT3X */
        struct rt_spi_device *epaper_dev;
    } devices; // 设备
    struct
    {
        float sht30[2]; // 温湿度传感器数据, 0 温度, 1 湿度
#ifdef PKG_USING_SGP30
        int32_t sgp30[2]; // 空气质量传感器数据, 0 TVOC, 1 eCO2
#endif
#ifdef PKG_USING_GZP6816D_SENSOR
        gzp6816d_data_t gzp6816d;
#endif
        float curvbat; // 电池电压
        float curvbat_percent; // 电池电压百分比
    } sensor_data; // 传感器数据
    bool init_flag; // 系统是否初始化成功, == true 表示初始化成功, == false 表示初始化失败
    
    bool exit_flag;
    struct 
    {
        bool cnt_wifi;
        time_t current_time;
        rt_alarm_t alarm;
        bool alarm_stop_beep; // 闹钟启动后退出蜂鸣器循环标志
    }device_status;

    ele_ds_cfg_t device_cfg; // 设备配置
    ele_ds_client_t client; // 终端相关内容
    struct ele_ds_ops ops;
};
typedef struct ele_ds *ele_ds_t;

extern int32_t devices_init(ele_ds_t ele_ds);
extern ele_ds_t g_ele_ds;
extern rt_err_t get_sht30_data(void *para);
extern rt_err_t get_sgp30_data(void *para);
extern int rt_hw_sgp30_port(void);
extern void ele_ds_gpio_init(void);
extern void ele_ds_gpio_deinit(void);

#endif /* __ELE_DS_H__ */
