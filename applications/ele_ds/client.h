/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-04-30 13:45:41
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-02 12:13:20
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

typedef struct ele_ds *ele_ds_t;
extern int32_t esp8266_device_init(ele_ds_t ele_ds);

typedef struct ele_ds_client
{
    rt_thread_t recv_thread;     // 终端线程负责和服务器通信
    rt_thread_t parse_thread;    // 终端线程负责解析数据
    struct rt_ringbuffer rb;     // 终端线程和服务器通信的环形缓冲区
    rt_sem_t rb_sem;             // 终端线程和服务器通信的信号量
    uint8_t recv_buf[1500 * 10]; // 终端线程接收数据的缓冲区
} ele_ds_client_t;

#endif /* __ELE_DS_CLIENT_H__ */
