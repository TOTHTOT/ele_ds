/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-02-16 19:11:27
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-02-16 19:21:15
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

#include "gzp6816d_driver.h"

#define LED0_PIN    GET_PIN(C, 13)
#define V3_3_PIN    GET_PIN(C, 8)

struct ele_ds_ops 
{
    rt_err_t (*get_gps6816d_data)(gps6816d_data_t *data);
};
typedef struct ele_ds_ops ele_ds_ops_t;

typedef struct 
{
    struct
    {
        rt_device_t gzp6816d_dev;
    }devices;
    struct ele_ds_ops ops;
}ele_ds;
typedef ele_ds *ele_ds_t;

ele_ds_t devices_init(void);

#endif /* __ELE_DS_H__ */
