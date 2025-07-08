/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-04-30 13:45:41
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-27 11:27:40
 * @FilePath: \ele_ds\applications\ele_ds\client.h
 * @Description: 电子卓搭客户端, 和服务器进行数据交互
 */
#ifndef __ELE_DS_CLIENT_H__
#define __ELE_DS_CLIENT_H__

#include <at_device_esp8266.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <rtthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdint.h>

#define USER_NAME_SIZE 21             // 用户名最大长度, 实际20字节, 最后一个字节是\0
#define USER_PASSWD_SIZE 21           // 密码最大长度
#define CITY_NAME_SIZE 21             // 城市名字最大长度
#define CLIENT_CHEAT_CONTENT_SIZE 256 // 备忘录消息最大长度
#define ENABLE_SAVE_FILE 1            // 是否保存下载的文件, 默认都要开启
#define ENABLE_POWERON_SEND_DEVINFO 1 // 是否开机发送设备信息给服务器, 默认开启, 暂时关闭

typedef struct ele_ds *ele_ds_t;
extern int32_t esp8266_device_init(ele_ds_t ele_ds);

typedef enum
{
    CRS_NONE = 0,      // 没有数据
    CRS_HEAD,          // 头部数据
    CRS_DATA,          // 数据部分
    CRS_FINISH,        // 接收完成数据, 接收状态机在 CRS_END 退出, CRS_FINISH 标记完成在下次循环内跑到 CRS_END 收尾接收流程
    CRS_END,           // 结束
    CSR_MAX,           // 最大数据长度
} client_recv_state_t; // 客户端接收数据状态

/* 此部分内容需要和服务器同步 开始*/
typedef enum
{
    EMT_CLIENTMSG_NONE = -1,   // 客户端消息类型无效
    EMT_CLIENTMSG_SUCCESS = 0, // 客户端成功
    EMT_CLIENTMSG_FAIL,        // 客户端失败
    EMT_CLIENTMSG_INFO,        // 客户端信息
    EMT_CLIENTMSG_CHEAT,       // 客户端发消息给某个客户端

    // 消息类型区分服务器和客户端
    EMT_SERVERMSG_MEMO = 0x80,  // 服务器备忘录消息
    EMT_SERVERMSG_WEATHER, // 服务器天气消息
    EMT_SERVERMSG_CLIENTUPDATE, // 服务器客户端升级消息
    EMT_SERVERMSG_BACKGROUND_IMG, // 服务器客户端屏幕背景图片
    EMT_SERVERMSG_DEFAULT_SYSFILE, // 服务器客户端默认系统文件
    EMT_SERVERMSG_OTHER_FILE, // 其他文件
    EMT_SERVERMSG_MAX, // 服务器消息最大消息类型, 使用时减去
} ele_msg_type_t; // 消息类型, 和上位机同步
#define MAX_SERVERMSG_NUM (EMT_SERVERMSG_MAX - EMT_SERVERMSG_MEMO)
#define MIN_SERVERMSG_NUM (EMT_SERVERMSG_MEMO)


typedef struct
{
    uint8_t temperature;  // 温度
    uint8_t humidity;     // 湿度
    uint32_t pressure;    // 气压
    uint16_t tvoc;         // VOC
    uint16_t co2;          // CO2
} ele_client_sensor_data_t; // 客户端传感器数据结构体, 没用到 采用json发送

typedef struct
{
    char username[USER_NAME_SIZE];
    char passwd[USER_PASSWD_SIZE];
    char cityname[CITY_NAME_SIZE];
    uint32_t cityid;             // 城市ID 和 cityname 对应, 查询天气使用
    uint16_t cntserver_interval; // 连接服务器间隔时间
    uint32_t version;            // 客户端版本号
    uint8_t battery;             // 电池电量
} ele_client_cfg_t;              // 客户端配置结构体, 没用到 采用json发送

typedef struct
{
    ele_client_sensor_data_t sensor_data; // 客户端本地传感器采集到的数据
    ele_client_cfg_t cfg;                 // 客户端配置
} ele_client_info_t;                      // 客户端连接到服务器要发送过来的消息, 没用到 采用json发送

typedef struct
{
    char username[USER_NAME_SIZE];        // 用户名
    char target_username[USER_NAME_SIZE]; // 目标用户名
    char msg[CLIENT_CHEAT_CONTENT_SIZE];  // 消息内容
} ele_client_cheat_t; // 客户端间聊天信息结构体

#pragma pack(1)
typedef struct client_software_updateinfo
{
    uint32_t crc; // 发送的文件 crc
    uint32_t len; // 文件长度
    uint32_t version; // 文件版本号, 升级包用的
    char info[32]; // 文件信息, 包含文件名称
} send_file_info_t; // 客户端升级包结构体
#pragma pack()

typedef struct
{
    union
    {
        char *memo;                           // 备忘录消息
        int8_t weatherdays;                   // 天气消息, 天数
        send_file_info_t file_info; // 客户端升级包信息
        ele_client_cheat_t cheat;             // 客户端间聊天信息
        union
        {
            ele_client_info_t client_info; // 客户端信息
            char *client_info_str;        // 客户端设备信息
        }client_info;
        uint32_t crc; // 文件CRC
    } data;
    uint32_t len;           // 消息长度
    uint32_t packcnt;       // 消息包序号
    ele_msg_type_t msgtype; // 消息类型

} ele_msg_t; // 消息结构体, 和服务器同步

/* 此部分内容需要和服务器同步 结束 */

typedef struct ele_ds_client
{
    struct rt_timer tcp_recv_timer; // tcp接收超时定时器
    rt_thread_t recv_thread;        // 终端线程负责和服务器通信
    rt_thread_t parse_thread;       // 终端线程负责解析数据
    struct rt_ringbuffer rb;        // 终端线程和服务器通信的环形缓冲区
    rt_sem_t rb_sem;                // 终端线程和服务器通信的信号量
    struct
    {
        client_recv_state_t recv_state; // 终端线程接收数据的状态
        ele_msg_type_t curparse_type;   // 当前解析的消息类型
        int32_t datalen;                // CRS_DATA 态时要接收的数据长度
        int32_t recv_len;               // 已经 接收数据长度
        int32_t update_file_fd;         // 升级包文件句柄
        char file_path[256];     // 升级包文件名
        uint32_t crc;       // 升级包crc, 服务器传来的
    } recv_info;                        // 终端线程接收数据的状态, 超时直接复位这个结构体
#define CLIENT_RECV_PACKSIZE 1500
#define CLIENT_RECV_BUFFSIZE (CLIENT_RECV_PACKSIZE * 10)
    uint8_t *recv_buf; // 终端线程接收数据的缓冲区
} ele_ds_client_t;

#endif /* __ELE_DS_CLIENT_H__ */
