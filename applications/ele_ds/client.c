/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-04-30 13:45:33
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-04-30 16:36:08
 * @FilePath: \ele_ds\applications\ele_ds\client.c
 * @Description: 电子卓搭客户端, 和服务器进行数据交互
 */
#include "client.h"
#include "ele_ds.h"
#include "dfscfg.h"

#define DBG_TAG "client"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

// 双缓冲, 写数据前判断是否有数据在接收缓冲区
static char recv_buf[2][2048] = {0};

void threads_communicate_server(void *parameter)
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
    const char *msg = "{\"type\":2,\"sensor_data\":{\"temperature\":25,\"humidity\":60,\"pressure\":101325,\"tvoc\":50,\"co2\":400},\"cfg\":{\"username\":\"test_user\",\"passwd\":\"123456\",\"cityname\":\"Beijing\",\"cityid\":101010100,\"cntserver_interval\":30,\"version\":20240328,\"battery\":85}}";
    ret = send(sock, msg, strlen(msg), 0);
    if (ret < 0)
    {
        LOG_E("send failed, ret = %d", ret);
    }
    
    while (ele_ds->exit_flag == false)
    {
        int len = recv(sock, recv_buf[0], sizeof(recv_buf) - 1, 0);
        if (len > 0)
        {
            recv_buf[len] = '\0';
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
        ele_ds->client_thread = rt_thread_create("th_client", threads_communicate_server, (void *)ele_ds,
                                                 10240, RT_THREAD_PRIORITY_MAX - 2, 20);
        if (ele_ds->client_thread != RT_NULL)
        {
            rt_thread_startup(ele_ds->client_thread);
        }
        return 0;
    }
    else
    {
        LOG_E("esp8266 device register failed, ret = %d", ret);
        return -1;
    }
}
