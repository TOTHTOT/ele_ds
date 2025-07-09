/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-02-15 22:47:40
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-03-29 09:40:48
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

static rt_err_t write_cmd(gzp6816d_device_t device, struct rt_i2c_bus_device *bus, uint8_t cmd)
{
    RT_ASSERT(device != NULL);
    RT_ASSERT(bus != NULL);
#if 1
    struct rt_i2c_msg msgs;
    rt_uint8_t buf[2];
    buf[0] = cmd;

    msgs.addr = device->addr;
    msgs.flags = RT_I2C_WR;
    msgs.buf = buf;
    msgs.len = 1;

    // if (rt_i2c_transfer(bus, &msgs, 1) == 1)
    if (rt_i2c_transfer(device->i2cdev, &msgs, 1) == 1)
        return RT_EOK;
    else
        return -RT_ERROR;
#else
    struct rt_i2c_bus_device *bus1 = rt_i2c_bus_device_find("i2c1");
    if (bus1 == RT_NULL)
    {
        LOG_E("Can't find i2c1 device");
        return -RT_ERROR;
    }
    if (rt_i2c_master_send(bus1, device->addr, RT_I2C_WR, &cmd, 1) == 1)
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }
#endif
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
    struct rt_i2c_bus_device *bus1 = rt_i2c_bus_device_find("i2c1");
    if (bus1 == RT_NULL)
    {
        LOG_E("Can't find i2c1 device");
        return -RT_ERROR;
    }
    if (rt_i2c_master_recv(bus1, device->addr, RT_I2C_RD, buf, len) == len)
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }
}

/**
 * @description: 反转字节数组
 * @param {void} *data 数据
 * @param {size_t} length 数据长度
 * @return {*}
 */
static inline void reverse_bytes(void *data, size_t length)
{
    uint8_t *byte_data = (uint8_t *)data;  // 将 void* 转换为 uint8_t* 类型
    uint8_t temp;
    size_t i = 0, j = length - 1;

    // 反转字节数组
    while (i < j)
    {
        temp = byte_data[i];
        byte_data[i] = byte_data[j];
        byte_data[j] = temp;

        i++;
        j--;
    }
}

/**
 * @description: 解析读取数据并回传到rawdata
 * @param {uint8_t} *buf 数据缓冲区
 * @param {gzp6816d_data_t} *rawdata 回传的数据
 * @return {rt_err_t} 函数执行结果，RT_EOK表示成功
 */
static rt_err_t cal_rawdata(uint8_t *buf, gzp6816d_data_t *rawdata)
{
    if (buf == NULL || rawdata == NULL)
    {
        return -RT_ERROR;
    }
#ifdef GZP6816D_CALTEST // 测试计算公式
    char str[64] = {0x04, 0x9B, 0xB0, 0xC5, 0x56, 0xAA};
    memcpy(buf, str, 6);
    for (int i = 0; i < 6; i++)
    {
        LOG_I("buf[%d] = %02x", i, buf[i]);
    }
    memset(str, 0, sizeof(str));
#endif /* GZP6816D_CALTEST */

    memcpy(&rawdata->raw_pressure.data, buf, 4);
    reverse_bytes(&rawdata->raw_pressure.data, 4);
    memcpy(&rawdata->raw_temperature.data, buf + 4, 2);
    reverse_bytes(&rawdata->raw_temperature.data, 2);
    rawdata->pressure = CALC_PRESSURE(rawdata->raw_pressure.value & GZP6816D_PRESSURE_VALUE_MASK) * 1.0;
    rawdata->temperature = CALC_TEMPERATURE(rawdata->raw_temperature.value) * 1.0;
#ifdef GZP6816D_CALTEST
    sprintf(str, "pressure = %d %f, temperature = %d %f",
            rawdata->raw_pressure.value & GZP6816D_PRESSURE_VALUE_MASK, rawdata->pressure,
            rawdata->raw_temperature.value, rawdata->temperature);
    LOG_I("%s", str);
#endif /* GZP6816D_CALTEST */
    LOG_I("raw pressure = %d, raw temperature = %d", rawdata->raw_pressure.value & GZP6816D_PRESSURE_VALUE_MASK, rawdata->raw_temperature.value);
    return RT_EOK;
}

/**
 * @description: 读取一次数据
 * @param {gzp6816d_device_t} device 设备结构体实例
 * @param {uint8_t} buf 读取到的数据
 * @return {rt_err_t} 函数执行结果，RT_EOK表示成功
 */
rt_err_t gzp6816d_readsls(gzp6816d_device_t device, uint8_t buf[])
{
    RT_ASSERT(device != NULL);
    RT_ASSERT(device->i2cdev != NULL);

    if (write_cmd(device, device->i2cdev, 0xac) == RT_EOK)
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
            LOG_W("gzp6816d busy");
            return -RT_EBUSY;
        }
        LOG_I("gzp6816d check result: %02x %02x %02x %02x %02x %02x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
    }
    return RT_EOK;
}

/**
 * @description: 读取数据
 * @param {rt_sensor_device} *sensor 传感器设备
 * @param {void} *buf 数据缓冲区
 * @param {rt_size_t} len 数据长度
 * @return {rt_ssize_t} 读取数据长度, < 0 表示读取失败
 */
rt_ssize_t gzp6816d_fetch_data(struct rt_sensor_device *sensor, void *buf, rt_size_t len)
{
    RT_ASSERT(sensor != NULL);
    RT_ASSERT(buf != NULL);
    RT_ASSERT(sensor->parent.user_data != NULL);

    if (len < sizeof(gzp6816d_data_t))
    {
        LOG_E("buf size is too small");
        return -RT_ERROR;
    }

    gzp6816d_device_t device = (gzp6816d_device_t)sensor->parent.user_data;
    uint8_t readbuf[6] = {0};
    // 读取数据
    rt_err_t ret = RT_ERROR;
    ret = gzp6816d_readsls(device, readbuf);
    // 没问题的话就解析数据
    if (ret == RT_EOK)
    {
        gzp6816d_data_t data = {0};
        cal_rawdata(readbuf, &data);
        memcpy(buf, &data, sizeof(gzp6816d_data_t));
        return sizeof(gzp6816d_data_t);
    }
    else
    {
        return ret;
    }
}
rt_err_t gzp6816d_control(struct rt_sensor_device *sensor, int cmd, void *arg)
{
    return RT_EOK;
}
static struct rt_sensor_ops sensor_ops =
{
    gzp6816d_fetch_data,
    gzp6816d_control
};

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
	rt_sensor_t sensor = NULL;
    gzp6816d_device_t device = NULL;
    // struct rt_sensor_module *module = RT_NULL;
    // module = rt_calloc(1, sizeof(struct rt_sensor_module));
    // if (module == RT_NULL)
    // {
    //     LOG_E("No memory for sensor module");
    //     return -RT_ENOMEM;
    // }

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
    
    // 注册传感器设备
	sensor = rt_calloc(1, sizeof(struct rt_sensor_device));
    if (sensor == NULL)
    {
        LOG_E("No memory for sensor device");
        goto __exit;
    }
    // 填写传感器信息
    sensor->info.type = RT_SENSOR_CLASS_BARO;
    sensor->info.vendor = RT_SENSOR_VENDOR_UNKNOWN;
    sensor->info.model = "gzp6816d";
    sensor->info.intf_type = RT_SENSOR_INTF_I2C;
    sensor->info.unit = RT_SENSOR_UNIT_PA;
    sensor->info.range_max = DMAX;
    sensor->info.range_min = DMIN;
    sensor->info.period_min = 203;
    sensor->ops = &sensor_ops;
    // sensor->module = module;
    // struct rt_sensor_config cfg;
    
    // cfg.intf.type = RT_SENSOR_INTF_I2C;
    // cfg.intf.dev_name = "i2c1";
    // cfg.intf.user_data = (void *)GZP6816D_ADDR;
    // rt_memcpy(&sensor->config, &cfg, sizeof(struct rt_sensor_config));
    if (rt_hw_sensor_register(sensor, "gzp6816d", RT_DEVICE_FLAG_RDWR, device) != RT_EOK)
    {
        LOG_E("gzp6816d register error");
        free(sensor);
        goto __exit;
    }
    // module->sen[0] = sensor;
    // module->sen_num = 1;
#if 0 // 测试读取数据
		uint8_t buf[6] = {0};
    if (gzp6816d_readsls(device, buf) == RT_EOK)
    {
        gzp6816d_data_t data = {0};
        cal_rawdata(buf, &data);
    }
#endif /* 0 */
    return RT_EOK;
__exit:
    // if (module != NULL)
    // {
    //     rt_free(module);
    // }
    if (sensor != NULL)
    {
        
    }
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
int gzp6816d_init_test(void)
{
	return gzp6816d_init("i2c1", GZP6816D_ADDR);
}
// INIT_DEVICE_EXPORT(gzp6816d_init_test);

#endif /* PKG_USING_GZP6816D_SENSOR */
