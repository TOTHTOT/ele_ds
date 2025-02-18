/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-02-16 19:11:22
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-02-18 23:14:48
 * @FilePath: \ele_ds\applications\ele_ds\ele_ds.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "ele_ds.h"

#define DBG_TAG "ele_ds"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

// 全局变量
ele_ds_t g_ele_ds = RT_NULL; // 全局设备函数指针

/**
 * @description: 获取传感器数据
 * @param {void} *para 回传数据
 * @return {rt_err_t} 函数执行结果，RT_EOK表示成功
 */
rt_err_t get_gzp6816d_data(void *para)
{
    gzp6816d_data_t *data = (gzp6816d_data_t *)para;
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
    rt_err_t ret = rt_device_read(gzp6816d_dev, 0, data, sizeof(gzp6816d_data_t));
    rt_device_close(gzp6816d_dev);
    if (ret > 0)
    {
        // char str[64] = {0};
        // sprintf(str, "Pressure: %f, Temperature: %f", data->pressure, data->temperature);
        // LOG_D("%s", str);
        return RT_EOK;
    }
    else
    {
        LOG_E("Read gzp6816d data failed, %d", ret);
        return ret;
    }
}

/**
 * @description: 获取SHT30数据
 * @param {void} *para 传入参数为两个float类型数组
 * @return {*}
 */
rt_err_t get_sht30_data(void *para)
{
    RT_ASSERT(para != RT_NULL);
    RT_ASSERT(g_ele_ds->devices.sht3x_dev != RT_NULL);

    rt_err_t ret = RT_EOK;
    float data[2] = {0}; // 温度和湿度
    ret = sht3x_read_singleshot(g_ele_ds->devices.sht3x_dev);
    if (ret == RT_EOK)
    {
        data[0] = g_ele_ds->devices.sht3x_dev->temperature;
        data[1] = g_ele_ds->devices.sht3x_dev->humidity;
        memcpy(para, data, sizeof(data));
    }
    return ret;
}

rt_err_t get_all_sensor_data(void *para)
{
    RT_ASSERT(para != RT_NULL);

    ele_ds_t ele_ds = (ele_ds_t)para;
    // 没有传感器
    if (SENSOR_MAX == 0)
        return -ENOENT;
    // 传感器未初始化
    if (ele_ds->init_flag == false)
        return -EBUSY;

#ifdef PKG_USING_GZP6816D_SENSOR
    ele_ds->ops.sensor_data[SENSOR_GZP6816D_INDEX](&ele_ds->sensor_data.gzp6816d);
#endif /* PKG_USING_GZP6816D_SENSOR */
#ifdef PKG_USING_SHT3X
    RT_ASSERT(ele_ds->devices.sht3x_dev != RT_NULL);
    ele_ds->ops.sensor_data[SENSOR_SHT3X_INDEX](ele_ds->sensor_data.sht30);
#endif /* PKG_USING_SHT3X */
    return RT_EOK;
}

/**
 * @description: 打印设备状态
 * @return {*}
 */
void ele_ds_print_status(void)
{
    char str[64] = {0};
    memset(str, 0, sizeof(str));
#ifdef PKG_USING_GZP6816D_SENSOR
    sprintf(str, "[GZP6816D]Pressure: %f, Temperature: %f", g_ele_ds->sensor_data.gzp6816d.pressure, g_ele_ds->sensor_data.gzp6816d.temperature);
    rt_kprintf("%s\n", str);
#endif /* PKG_USING_GZP6816D_SENSOR */

#ifdef PKG_USING_SHT3X
    memset(str, 0, sizeof(str));
    sprintf(str, "[SHT30]Temperature: %f, Humidity: %f", g_ele_ds->sensor_data.sht30[0], g_ele_ds->sensor_data.sht30[1]);
    rt_kprintf("%s\n", str);
#endif /* PKG_USING_SHT3X */
}
MSH_CMD_EXPORT_ALIAS(ele_ds_print_status, status, ele_ds_print_status);

ele_ds_ops_t ele_ds_ops = 
{
    .sensor_data[SENSOR_GZP6816D_INDEX] = get_gzp6816d_data,
    .sensor_data[SENSOR_SHT3X_INDEX] = get_sht30_data,
    .sensor_data[SENSOR_MAX] = get_all_sensor_data,
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
    ele_ds->devices.sht3x_dev = sht3x_init("i2c1", SHT3X_ADDR_PD);
    ele_ds->ops = ele_ds_ops;
    ele_ds->init_flag = true;
    g_ele_ds = ele_ds;
    return ele_ds;
}


