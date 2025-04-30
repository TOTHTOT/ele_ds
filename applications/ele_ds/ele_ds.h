/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-02-16 19:11:27
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-04-30 13:42:56
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
        float sht30[2];   // 温湿度传感器数据, 0 温度, 1 湿度
        int32_t sgp30[2]; // 空气质量传感器数据, 0 TVOC, 1 eCO2
#ifdef PKG_USING_GZP6816D_SENSOR
        gzp6816d_data_t gzp6816d;
#endif
    } sensor_data;  // 传感器数据
    bool init_flag; // 系统是否初始化成功, == true 表示初始化成功, == false 表示初始化失败
    
    rt_thread_t client_thread; // 终端线程负责和服务器通信
    bool exit_flag;
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
