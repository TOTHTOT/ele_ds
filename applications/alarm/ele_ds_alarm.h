/*
 * @Author: TOTHTOT
 * @Date: 25-6-11
 * @LastEditors: TOTHTOT
 * @FilePath:ele_ds_alarm.h
 * @Description: 闹钟相关功能
 */


#ifndef ELE_DS_ALARM_H
#define ELE_DS_ALARM_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <stdbool.h>

extern int32_t ele_ds_alarm_init(rt_alarm_t alarm, const time_t set_alarm_time, const bool enable);

#endif //ELE_DS_ALARM_H
