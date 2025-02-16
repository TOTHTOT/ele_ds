/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-02-15 22:47:40
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-02-16 10:54:24
 * @FilePath: \ele_ds\components\gzp6816d_driver\gzp6816d_driver.c
 * @Description: gzp6816d 气压驱动, 读数据时要根据测量频率间隔读数, 默认203ms
 */
#include <gzp6816d_driver.h>
#include <drivers/sensor.h>
#include "board.h"

#ifdef PKG_USING_GZP6816D_SENSOR

#define DBG_TAG "gzp6x"
#ifdef RT_GZP6818D_SENSOR_DEBUG
#define DBG_LVL DBG_INFO
#else
#define DBG_LVL DBG_ERROR
#endif
#include <rtdbg.h>

static rt_err_t write_cmd(gzp6816d_device_t device, uint8_t cmd)
{
    RT_ASSERT(device != NULL);
    RT_ASSERT(device->i2cdev != NULL);
    struct rt_i2c_msg msgs;
    rt_uint8_t buf[2] ;
    buf[0] = cmd ;

    msgs.addr = device->addr;
    msgs.flags = RT_I2C_WR;
    msgs.buf = buf;
    msgs.len = 1;

    if (rt_i2c_transfer(device->i2cdev, &msgs, 1) == 1)
        return RT_EOK;
    else
        return -RT_ERROR;
}


/**
 * @description: 读取数据
 * @param {gzp6816d_device_t} device 设备结构体实例
 * @param {uint8_t *} buf 读入数据缓冲区
 * @param {uint8_t} len 读入数据长度
 * @return {rt_errt_t} 函数执行结果，RT_EOK表示成功
 */
static rt_err_t read_bytes(gzp6816d_device_t device, uint8_t * buf, uint8_t len)
{
    RT_ASSERT(device != NULL);
    RT_ASSERT(device->i2cdev != NULL);
    if (rt_i2c_master_recv(device->i2cdev, device->addr, RT_I2C_RD, buf, len) == len)
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }
}

static rt_err_t cal_rawdata(uint8_t *buf, gps6816d_data_t *rawdata)
{
    if (buf == NULL || rawdata == NULL)
    {
        return -RT_ERROR;
    }
    memcpy(&rawdata->pressure, buf, 4);
    memcpy(&rawdata->temperature, buf + 4, 2);
    rawdata->pressure.value = CALC_PRESSURE(rawdata->pressure.value & GZP6816D_PRESSURE_VALUE_MASK);
    rawdata->temperature.value = CALC_TEMPERATURE(rawdata->temperature.value);
    LOG_I("pressure = %d, temperature = %d",
          rawdata->pressure.value,
          rawdata->temperature.value);
    return RT_EOK;
}

/**
 * @description: 读取一次数据
 * @param {gzp6816d_device_t} device 设备结构体实例
 * @return {rt_err_t} 函数执行结果，RT_EOK表示成功
 */
rt_err_t gzp6816d_readsls(gzp6816d_device_t device, uint8_t buf[6])
{
    RT_ASSERT(device != NULL);
    RT_ASSERT(device->i2cdev != NULL);

    if (write_cmd(device, 0xac) == RT_EOK)
    {
        rt_thread_mdelay(250);
        if (read_bytes(device, buf, 6) != RT_EOK)
        {
            LOG_E("gzp6816d read error");
            return -RT_ERROR;
        }
        // 如果status的bit5是1那就是读太快了要加大延迟
        if (buf[0] & 0x20)
        {
            LOG_E("gzp6816d busy");
            return -RT_EBUSY;
        }
        LOG_I("gzp6816d check result: %02x %02x %02x %02x %02x %02x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
    }
    return RT_EOK;
}

/**
 * @description: 初始化设备
 * @param {char} *i2cname i2c名称
 * @param {uint8_t} addr 设备地址, 芯片地址 0x78
 * @return {rt_err_t} 函数执行结果，RT_EOK表示成功
 */
rt_err_t gzp6816d_init(const char *i2cname, uint8_t addr)
{
    if (i2cname == NULL)
    {
        LOG_E("No i2c name for gzp6816d");
        return -RT_ERROR;
    }
    uint8_t buf[6] = {0};
    gzp6816d_device_t device = NULL;
    device = rt_calloc(1, sizeof(gzp6816d_device_t));
    if (device == NULL)
    {
        LOG_E("No memory for gzp6816d device");
        return -RT_ENOMEM;
    }
    device->addr = addr;
    device->i2cname = rt_strdup(i2cname);
    device->i2cdev = rt_i2c_bus_device_find(i2cname);
    if (device->i2cdev == NULL)
    {
        LOG_E("No i2c device %s", i2cname);
        goto __exit;
    }
    if (gzp6816d_readsls(device, buf) == RT_EOK)
    {
        gps6816d_data_t data = {0};
        cal_rawdata(buf, &data);
    }
    return RT_EOK;
__exit:
    if (device->i2cname != NULL)
    {
        rt_free(device->i2cname);
    }
    if (device != NULL)
    {
        rt_free(device);
    }
    return -RT_ERROR;
}

/**
 * @description: 释放设备
 * @param {gzp6816d_device_t} device 设备结构体实例
 * @return {*}
 */
void gzp6816d_deinit(gzp6816d_device_t device)
{
    RT_ASSERT(device != NULL);
    rt_free(device->i2cname);
    rt_free(device);
}

/**
 * @description: gzp6816d初始化测试函数
 * @return {*}
 */
void gzp6816d_init_test(void)
{
    gzp6816d_init("i2c1", GZP6816D_ADDR);
}
MSH_CMD_EXPORT_ALIAS(gzp6816d_init_test, gzp6816d, gzp6816d init test);

#endif /* PKG_USING_GZP6816D_SENSOR */
