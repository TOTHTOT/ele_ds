/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-04-30 13:45:33
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-02 12:24:04
 * @FilePath: \ele_ds\applications\ele_ds\client.c
 * @Description: 电子卓搭客户端, 和服务器进行数据交互
 */
#include "client.h"
#include "ele_ds.h"
#include "dfscfg.h"

#define DBG_TAG "client"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/**
 * @description: 解析接收数据线程
 * @param {void} *parameter 传入参数为ele_ds_t类型
 * @return {*}
 */
static void thread_parse_recv_data(void *parameter)
{
    if (parameter == RT_NULL)
    {
        LOG_E("parameter is NULL");
        return;
    }
    ele_ds_t ele_ds = (ele_ds_t)parameter;
    rt_uint8_t buffer[1500]; // tcp 的mtu一般都是 1500
    int32_t ret = 0;

    while (ele_ds->exit_flag == false)
    {
        ret = rt_sem_take(ele_ds->client.rb_sem, 50);
        if (ret == RT_EOK)
        {
            rt_size_t len = rt_ringbuffer_get(&ele_ds->client.rb, buffer, sizeof(buffer));
            if (len > 0)
            {
                LOG_I("recv len: %d", len);
            }
        }
        else
        {
            LOG_I("recv sem take timeout, ret = %d", ret);
            continue;
        }
    }
    LOG_I("recv parse thread exit");
}

/**
 * @description: 客户端线程
 * @param {void} *parameter 传入参数为ele_ds_t类型
 * @return {*}
 */
static void threads_communicate_server(void *parameter)
{
    if (parameter == RT_NULL)
    {
        LOG_E("parameter is NULL");
        return;
    }
    int ret = 0;
    ele_ds_t ele_ds = (ele_ds_t)parameter;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        LOG_E("socket create failed\n");
        return;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(ele_ds->device_cfg.server_port);
    server_addr.sin_addr.s_addr = inet_addr((char *)ele_ds->device_cfg.server_addr);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        LOG_E("connect failed\n");
        closesocket(sock);
        return;
    }
    // 连接成功发送模拟数据
    const char *msg = "{\"type\":2,\"sensor_data\":{\"temperature\":25,\"humidity\":60,\"pressure\":101325,\"tvoc\":50,\"co2\":400},\"cfg\":{\"username\":\"test_user\",\"passwd\":\"123456\",\"cityname\":\"Beijing\",\"cityid\":101010100,\"cntserver_interval\":30,\"version\":20240328,\"battery\":85}}";
    ret = send(sock, msg, strlen(msg), 0);
    if (ret < 0)
    {
        LOG_E("send failed, ret = %d", ret);
    }
    
    while (ele_ds->exit_flag == false)
    {
        int len = recv(sock, ele_ds->client.recv_buf, sizeof(ele_ds->client.recv_buf), 0);
        if (len > 0)
        {
            rt_ringbuffer_put(&ele_ds->client.rb, ele_ds->client.recv_buf, len);
            rt_sem_release(ele_ds->client.rb_sem);
            LOG_I("Size: %d", len);
        }
        rt_thread_mdelay(20);
    }
    closesocket(sock);
    LOG_I("client thread exit");
}

/**
 * @description: 注册esp
 * @param {ele_ds_t} *ele_ds 配置参数
 * @return {int32_t} 函数执行结果, 0表示成功
 */
int32_t esp8266_device_init(ele_ds_t ele_ds)
{
    int32_t ret = 0;
    if (ele_ds == RT_NULL)
    {
        LOG_E("ele_ds is NULL");
        return -1;
    }
    struct at_device_esp8266 esp0 = {
        "eps0",
        "uart3",
        (char *)ele_ds->device_cfg.wifi_ssid,
        (char *)ele_ds->device_cfg.wifi_passwd,
        3 * 1024,
    };
    memcpy(&ele_ds->devices.esp8266, &esp0, sizeof(struct at_device_esp8266));
    rt_thread_mdelay(1000);
    ret = at_device_register(&(ele_ds->devices.esp8266.device),
                             ele_ds->devices.esp8266.device_name,
                             ele_ds->devices.esp8266.client_name,
                             AT_DEVICE_CLASS_ESP8266,
                             (void *)&ele_ds->devices.esp8266);

    if (ret == 0)
    {
        ele_ds->client.rb_sem = rt_sem_create("rb_sem", 0, RT_IPC_FLAG_FIFO);
        if (ele_ds->client.rb_sem == RT_NULL)
        {
            LOG_E("rb_sem create failed\n");
            return -4;
        }

        ele_ds->client.recv_thread = rt_thread_create("th_client", threads_communicate_server, (void *)ele_ds,
                                                 4096, RT_MAIN_THREAD_PRIORITY - 2, 20);
        if (ele_ds->client.recv_thread != RT_NULL)
        {
            rt_thread_startup(ele_ds->client.recv_thread);
        }
        else
        {
            LOG_E("recv thread create failed\n");
            return -2;
        }

        ele_ds->client.parse_thread = rt_thread_create("th_recv_parse", thread_parse_recv_data, (void *)ele_ds,
                                                 8192, RT_MAIN_THREAD_PRIORITY - 3, 20);
        if (ele_ds->client.parse_thread != RT_NULL)
        {
            rt_thread_startup(ele_ds->client.parse_thread);
        }
        else
        {
            LOG_E("recv parse thread create failed\n");
            return -3;
        }
        return 0;
    }
    else
    {
        LOG_E("esp8266 device register failed, ret = %d", ret);
        return -1;
    }
}
