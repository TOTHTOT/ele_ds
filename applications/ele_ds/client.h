/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-04-30 13:45:41
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-04-30 13:46:24
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


#endif /* __ELE_DS_CLIENT_H__ */
