/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-02-16 19:11:22
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-02-16 19:25:39
 * @FilePath: \ele_ds\applications\ele_ds\ele_ds.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "ele_ds.h"

#define DBG_TAG "ele_ds"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/**
 * @description: 获取传感器数据
 * @param {gps6816d_data_t} *data 回传数据
 * @return {rt_err_t} 函数执行结果，RT_EOK表示成功
 */
rt_err_t get_gps6816d_data(gps6816d_data_t *data)
{
    rt_device_t gzp6816d_dev = rt_device_find("baro_gzp6816d");
    if (gzp6816d_dev == RT_NULL)
    {
        LOG_E("No gzp6816d device");
        return -RT_ERROR;
    }
    if (rt_device_open(gzp6816d_dev, RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        LOG_E("Open gzp6816d device failed");
        return -RT_ERROR;
    }
    rt_device_control(gzp6816d_dev, RT_SENSOR_CTRL_SET_ODR, (void *)100);
    rt_err_t ret = rt_device_read(gzp6816d_dev, 0, data, sizeof(gps6816d_data_t));
    rt_device_close(gzp6816d_dev);
    if (ret > 0)
    {
        char str[64] = {0};
        sprintf(str, "Pressure: %f, Temperature: %f", data->pressure, data->temperature);
        LOG_D("%s", str);
        return RT_EOK;
    }
    else
    {
        LOG_E("Read gzp6816d data failed, %d", ret);
        return ret;
    }
}

ele_ds_ops_t ele_ds_ops = 
{
    .get_gps6816d_data = get_gps6816d_data,
};
ele_ds_t devices_init(void)
{
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(V3_3_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(V3_3_PIN, PIN_HIGH);

    ele_ds_t ele_ds = rt_calloc(1, sizeof(ele_ds_t));
    if (ele_ds == RT_NULL)
    {
        LOG_E("No memory for ele_ds");
        return NULL;
    }

    ele_ds->ops = ele_ds_ops;
    return ele_ds;
}


