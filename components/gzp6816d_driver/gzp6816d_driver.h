/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-02-15 22:47:40
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-02-16 10:55:39
 * @FilePath: \ele_ds\components\gzp6816d_driver\gzp6816d_driver.h
 * @Description: gzp6816d 气压驱动头文件
 */
#ifndef __GZP6816D_DRIVER_H__

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <drivers/sensor.h>
#include <stdint.h>
#include <string.h>
#ifdef PKG_USING_GZP6816D_SENSOR

#define GZP6816D_ADDR 0x78 // 设备地址
// 气压计算相关宏定义
#define PMAX 110
#define PMIN 30
#define DMAX 15099494
#define DMIN 1677722
#define OFFSET_PRESSURE 20

// 温度计算相关宏定义
#define MAX_16BITS_UNSIGNED 65536
#define TEMP_MIN -40
#define TEMP_MAX 150

// 定义计算气压的宏
#define CALC_PRESSURE(Dtest) \
    ((PMAX - PMIN) / (double)(DMAX - DMIN) * (Dtest - DMIN) + OFFSET_PRESSURE)

// 定义计算温度的宏
#define CALC_TEMPERATURE(hex_value) \
    ((TEMP_MAX - TEMP_MIN) * ((hex_value) / (double)MAX_16BITS_UNSIGNED * 100) / 100 + TEMP_MIN)

#pragma pack(1)
typedef union
{
#define GZP6816D_PRESSURE_VALUE_MASK 0x00FFFFFF
    uint8_t data[4];
    uint32_t value; // value 与上 GZP6816D_PRESSURE_VALUE_MASK 获取气压值
} gzp6816d_pressure_t; // 气压数据, 状态(1字节)+气压(3字节)
typedef union 
{
    uint8_t data[2];
    uint16_t value;
}gzp6816d_temperature_t; // 温度数据

typedef struct
{
    gzp6816d_pressure_t raw_pressure; // 气压
    float pressure;
    gzp6816d_temperature_t raw_temperature; // 温度
    float temperature;
} gzp6816d_data_t; // 数据
#pragma pack()
typedef struct gzp6816d_device
{
    char *i2cname;                    // i2c名称
    uint8_t addr;                     // 设备地址
    struct rt_i2c_bus_device *i2cdev; // i2c设备
}gzp6816d_device; // 设备结构体
typedef struct gzp6816d_device *gzp6816d_device_t;

extern rt_err_t gzp6816d_init(const char *i2cname, uint8_t addr);

#endif /* PKG_USING_GZP6816D_SENSOR */
#endif /* __GZP6816D_DRIVER_H__ */

