/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-02-16 19:11:27
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-02-18 23:10:20
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
#include<stdbool.h>

#include "gzp6816d_driver.h"
#include "sht3x.h"

/* 参数配置 */
#define LED0_PIN    GET_PIN(C, 13)
#define V3_3_PIN    GET_PIN(C, 8)

typedef rt_err_t (*get_sensor_data)(void *data);

typedef enum
{
#ifdef PKG_USING_GZP6816D_SENSOR
    SENSOR_GZP6816D_INDEX = 0,
#endif /* PKG_USING_GZP6816D_SENSOR */

#ifdef PKG_USING_SHT3X
    SENSOR_SHT3X_INDEX,
#endif /* PKG_USING_SHT3X */
    SENSOR_MAX
}sensor_index_t;
struct ele_ds_ops 
{
    get_sensor_data sensor_data[SENSOR_MAX + 1];
};
typedef struct ele_ds_ops ele_ds_ops_t;

typedef struct 
{
    struct
    {
        rt_device_t gzp6816d_dev;
        sht3x_device_t sht3x_dev;
    }devices; // 设备
    struct 
    {
        float sht30[2]; // 温湿度传感器数据, 0 温度, 1 湿度
        gzp6816d_data_t gzp6816d;
    }sensor_data; // 传感器数据
    bool init_flag; // 系统是否初始化成功, == true 表示初始化成功, == false 表示初始化失败
    struct ele_ds_ops ops;
}ele_ds;
typedef ele_ds *ele_ds_t;

ele_ds_t devices_init(void);

#endif /* __ELE_DS_H__ */
