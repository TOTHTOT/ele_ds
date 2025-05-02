/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-04-30 13:45:33
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-02 20:59:31
 * @FilePath: \ele_ds\applications\ele_ds\client.c
 * @Description: 电子卓搭客户端, 和服务器进行数据交互
 */
#include "client.h"
#include "ele_ds.h"
#include "dfscfg.h"
#include "cJSON.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dfscfg.h>

#define DBG_TAG "client"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/**
 * @description: 清除客户端信息
 * @param {ele_ds_client_t} *client 客户端指针
 * @return {int32_t} 0表示成功, -1表示失败
 */
static int32_t clear_client_info(ele_ds_client_t *client)
{
    if (client == RT_NULL)
    {
        LOG_E("client is NULL");
        return -1;
    }
    client->recv_info.recv_state = CRS_NONE;
    client->recv_info.curparse_type = EMT_CLIENTMSG_NONE;
    client->recv_info.datalen = 0;
    client->recv_info.recv_len = 0;
    if (client->recv_info.update_file_fd >= 0)
    {
        close(client->recv_info.update_file_fd);
        client->recv_info.update_file_fd = -1;
    }
    return 0;
}

/**
 * @description: 解析json数据, 将数据还原为 ele_msg_t
 * @param {ele_msg_t} *msg  消息结构体指针
 * @param {uint8_t} *buffer 接收数据缓冲区
 * @param {int32_t} len 接收数据长度
 * @return {int32_t} 0表示成功, -1表示数据格式错误
 */
static int32_t parese_json_data(ele_msg_t *msg, uint8_t *buffer, int32_t len)
{
    if (msg == RT_NULL || buffer == RT_NULL || len <= 0 || is_json((char *)buffer, len) == false)
    {
        LOG_E("msg or buffer is NULL or not json, len = %d", len);
        return -1;
    }
    // 解析 json 数据
    cJSON *root = cJSON_Parse((char *)buffer);
    if (root == RT_NULL)
    {
        LOG_E("cJSON_Parse failed, len = %d", len);
        return -2;
    }

    msg->len = cJSON_GetObjectItem(root, "len")->valueint;
    msg->packcnt = cJSON_GetObjectItem(root, "packcnt")->valueint;
    msg->msgtype = (ele_msg_type_t)cJSON_GetObjectItem(root, "msgtype")->valueint;

    switch (msg->msgtype)
    {
    case EMT_SERVERMSG_MEMO:
        msg->data.memo = cJSON_GetObjectItem(root, "message")->valuestring;
        break;
    case EMT_SERVERMSG_WEATHER:
        msg->data.weahterdays = cJSON_GetObjectItem(root, "days")->valueint;
        break;
    case EMT_SERVERMSG_CLIENTUPDATE:
    {
        cJSON *packinfo = cJSON_GetObjectItem(root, "packinfo");
        if (packinfo != RT_NULL)
        {
            cJSON *len = cJSON_GetObjectItem(packinfo, "len");
            cJSON *crc = cJSON_GetObjectItem(packinfo, "cscrc");
            cJSON *version = cJSON_GetObjectItem(packinfo, "version");
            cJSON *buildinfo = cJSON_GetObjectItem(packinfo, "buildinfo");

            if (len && crc && version && buildinfo)
            {
                msg->data.cs_info.len = (uint32_t)len->valueint;
                msg->data.cs_info.crc = (uint32_t)crc->valueint;
                msg->data.cs_info.version = (uint32_t)version->valueint;
                strncpy(msg->data.cs_info.buildinfo, buildinfo->valuestring,
                        sizeof(msg->data.cs_info.buildinfo) - 1);
                msg->data.cs_info.buildinfo[sizeof(msg->data.cs_info.buildinfo) - 1] = '\0';
            }
            else
            {
                LOG_E("missing field in packinfo");
                cJSON_Delete(root);
                return -3;
            }
        }
        else
        {
            LOG_E("no packinfo in json");
            cJSON_Delete(root);
            return -4;
        }
        break;
    }

    default:
        LOG_E("unknown msgtype");
        cJSON_Delete(root);
        return -5;
    }
    return 0;
}

/**
 * @description: 解析消息类型, 不同消息类型会设置不同 recv_state
 * @param {ele_ds_t} ele_ds 设备指针
 * @param {ele_msg_t} *msg 消息结构体指针
 * @return {int32_t} 0表示成功, < 0 失败, > 0 表示消息类型
 */
static int32_t parese_msgtype(ele_ds_t ele_ds, ele_msg_t *msg)
{
    if (ele_ds == RT_NULL || msg == RT_NULL)
    {
        LOG_E("ele_ds or msg is NULL");
        return -1;
    }

    ele_ds_client_t *client = &ele_ds->client;
    client->recv_info.curparse_type = msg->msgtype;
    switch (msg->msgtype)
    {
    case EMT_SERVERMSG_MEMO:
        LOG_I("recv memo: %s", msg->data.memo);
        // 收到备忘录消息后存到配置文件中
        memcpy(ele_ds->device_cfg.memo, msg->data.memo, strlen(msg->data.memo) + 1);
        client->recv_info.recv_state = CRS_END;
        break;
    case EMT_SERVERMSG_WEATHER:
        LOG_I("recv weather days: %d", msg->data.weahterdays);
        client->recv_info.recv_state = CRS_DATA;
        break;
    case EMT_SERVERMSG_CLIENTUPDATE:
    {
        LOG_I("recv client update info: len = %d, crc = %d, version = %d, buildinfo = %s",
              msg->data.cs_info.len, msg->data.cs_info.crc, msg->data.cs_info.version,
              msg->data.cs_info.buildinfo);
        client->recv_info.recv_state = CRS_DATA;
        client->recv_info.datalen = msg->data.cs_info.len;
        char filepath[64] = {0};
        sprintf(filepath, "%s/%d.bin", SOFTWARE_UPDATE_FILE_PATH, msg->data.cs_info.version);
        // 收到数据头后创建文件, 如果文件存在就删除
        client->recv_info.update_file_fd = open(filepath, O_RDWR | O_CREAT | O_TRUNC, 0);
        if (client->recv_info.update_file_fd < 0)
        {
            LOG_E("open file failed, fd = %d, path = %s", client->recv_info.update_file_fd, filepath);
            return -2;
        }
    }
    break;

    default:
        LOG_E("unknown msgtype: %d", msg->msgtype);
        return -3;
    }
    return client->recv_info.curparse_type;
}

/**
 * @description: 解析接收到的数据
 * @param {ele_ds_t} ele_ds 设备指针
 * @param {uint8_t} *buffer 接收数据缓冲区
 * @param {int32_t} len 接收数据长度
 * @return {int32_t} 0表示成功, -1表示数据格式错误,
 */
static int32_t parese_recv_data(ele_ds_t ele_ds, uint8_t *buffer, int32_t len)
{
    // 检查输入参数是否有效
    if (ele_ds == RT_NULL || buffer == RT_NULL || len <= 0)
    {
        LOG_E("ele_ds or buffer is NULL, len = %d", len);
        return -1;
    }

    int32_t ret = 0; // 状态变量，记录函数执行结果

    while (ele_ds->client.recv_info.recv_state != CRS_END)
    {
        switch (ele_ds->client.recv_info.recv_state)
        {
        case CRS_NONE: // 默认状态，检查是否是 JSON 数据
            if (is_json((char *)buffer, len) == true)
            {
                ele_ds->client.recv_info.recv_state = CRS_HEAD;
                LOG_I("recv json data, len = %d", len);
            }
            else
            {
                LOG_W("recv data is not json, len = %d", len);
                ele_ds->client.recv_info.recv_state = CRS_NONE;
                ret = -1; // 数据格式错误
                break;
            }
            break;

        case CRS_HEAD:
        {
            /* 解析头部json数据并设置 curparse_type,
               1. 解析成功后推进到 CRS_DATA 并退出函数 ret = 1
               2. 解析失败设置 CRS_NONE */
            int32_t head_len = strlen((char *)buffer);
            ele_msg_t msg = {0};
            int32_t result = parese_json_data(&msg, buffer, head_len);
            if (result == 0)
            {
                LOG_I("msgtype = %d, len = %d, packcnt = %d", msg.msgtype, msg.len, msg.packcnt);
                result = parese_msgtype(ele_ds, &msg);
                if (result < 0)
                {
                    LOG_E("parse msgtype failed, ret = %d", result);
                    ele_ds->client.recv_info.recv_state = CRS_NONE;
                    ret = -2; // 解析失败
                    break;
                }
                else if (result > )

            }
            else
            {
                LOG_E("parse json data failed, ret = %d", result);
                ele_ds->client.recv_info.recv_state = CRS_NONE;
                ret = -2; // 解析失败
                break;
            }
        }
        break;

        case CRS_DATA: // 解析数据部分
            /* 接收数据段
                1. 处理 datalen 长度的数据, 收完设置 CRS_END
                2. 长时间处于 CRS_DATA 且没数据, 使用定时器复位接收状态 */
            if (ele_ds->client.recv_info.datalen > 0)
            {
                if (ele_ds->client.recv_info.curparse_type == EMT_SERVERMSG_WEATHER) // 天气数据写入到配置信息
                {
                    memcpy((char *)ele_ds->device_cfg.weather_info + ele_ds->client.recv_info.recv_len, buffer, len);
                }
                else if (ele_ds->client.recv_info.curparse_type == EMT_SERVERMSG_CLIENTUPDATE) // 升级数据写入到文件
                {
                    int32_t write_len = write(ele_ds->client.recv_info.update_file_fd, buffer, len);
                    if (write_len < 0)
                    {
                        LOG_E("write file failed, ret = %d", write_len);
                        ret = -3; // 写文件失败
                        break;
                    }
                }
                else
                {
                    LOG_E("unknown curparse_type: %d", ele_ds->client.recv_info.curparse_type);
                    ret = -3; // 未知类型
                    break;
                }
                ele_ds->client.recv_info.recv_len += len; // 累加已接收的数据长度
                ele_ds->client.recv_info.datalen -= len; // 减去已接收的数据长度
            }
            else
                ele_ds->client.recv_info.recv_state = CRS_END;
            break;
        case CRS_END: // 接收结束
            clear_client_info(&ele_ds->client);
            break;

        default:
            LOG_E("recv data error, state = %d", ele_ds->client.recv_info.recv_state);
            ret = -2; // 未知状态
            break;
        }

        // 如果发生错误，退出循环
        if (ret != 0)
        {
            break;
        }
    }

    // 返回最终状态
    return ret;
}

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
                int32_t result = parese_recv_data(ele_ds, buffer, len);
                if (result < 0)
                {
                    LOG_E("parse recv data failed, ret = %d", result);
                    clear_client_info(&ele_ds->client);
                }
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
