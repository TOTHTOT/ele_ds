/*
 * @Author: TOTHTOT
 * @Date: 25-6-11
 * @LastEditors: TOTHTOT
 * @FilePath:ele_ds_alarm.c
 * @Description: 闹钟相关功能
 */
#include "ele_ds_alarm.h"
#include "ele_ds.h"
#include "beep.h"

#define DBG_TAG "ele_pm"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/**
 * @brief 闹钟回调函数, 触发后蜂鸣器响起
 * @param myalarm 闹钟句柄
 * @param timestamp 响铃时的时间
 */
static void alarm_cb(rt_alarm_t myalarm, time_t timestamp)
{
    (void)myalarm;

    // 没开启闹钟的话直接退出, 正常在 设置 里面修改了状态会关闭闹钟, 直接修改配置文件是不会关闭的
    if (g_ele_ds->device_cfg.alarm_enable == false)
    {
        LOG_D("alarm is disable, exit alarm callback function.");
        return;
    }

    struct tm p_tm;
    const time_t now = timestamp;
    uint32_t beep_time = 10; // 蜂鸣器最大响铃次数, 超过就退出

    localtime_r(&now, &p_tm); // 时间戳转换
    LOG_D("user alarm callback function.");
    LOG_D("curr time: %04d-%02d-%02d %02d:%02d:%02d", p_tm.tm_year + 1900, p_tm.tm_mon + 1, p_tm.tm_mday, p_tm.tm_hour, p_tm.tm_min, p_tm.tm_sec);  // 打印闹钟中断产生时的时间，和设定的闹钟时间比对，以确定得到的是否是想要的结果
    while (g_ele_ds->device_status.alarm_stop_beep == false && beep_time--)
    {
        beep(1, 500, 50, 0);
        rt_thread_mdelay(500);
    }
    g_ele_ds->device_status.alarm_stop_beep = false;
}

/**
 * @brief 初始化卓搭闹钟功能
 * @param alarm 闹钟句柄
 * @param set_alarm_time 设定的响铃时间 (时间戳)
 * @param enable 是否使能, 不管有没有使能都会初始化 alarm, 不使能只是不开启
 */
int32_t ele_ds_alarm_init(rt_alarm_t alarm, const time_t set_alarm_time, const bool enable)
{
    struct tm alarm_tm;
    struct rt_alarm_setup setup;
    int32_t ret = 0;

    if (alarm == RT_NULL)
    {
        LOG_E("alarm is null");
        ret = -1;
        goto err;
    }
    // 根据时区设置闹钟
    localtime_r(&set_alarm_time, &alarm_tm);

    // 设置闹钟标志, 标志错误闹钟中断不对
    LOG_D("alarm time: %04d-%02d-%02d %02d:%02d:%02d",
          alarm_tm.tm_year + 1900, alarm_tm.tm_mon + 1, alarm_tm.tm_mday, alarm_tm.tm_hour,
          alarm_tm.tm_min, alarm_tm.tm_sec); // 打印闹钟时间
    setup.flag = RT_ALARM_DAILY;
    setup.wktime.tm_year = alarm_tm.tm_year;
    setup.wktime.tm_mon = alarm_tm.tm_mon;
    setup.wktime.tm_mday = alarm_tm.tm_mday;
    setup.wktime.tm_wday = alarm_tm.tm_wday;
    setup.wktime.tm_hour = alarm_tm.tm_hour;
    setup.wktime.tm_min = alarm_tm.tm_min;
    setup.wktime.tm_sec = alarm_tm.tm_sec;

    alarm = rt_alarm_create(alarm_cb, &setup); // 创建一个闹钟并设置回调函数
    if (RT_NULL != alarm)
    {
        if (enable)
        {
            const rt_err_t ret = rt_alarm_start(alarm); // 启动闹钟
            if (ret != RT_EOK)
            {
                LOG_E("rtc alarm start failed");
                ret = -2;
                goto err;
            }
        }
        else
            LOG_D("alarm not enable.");
    }
    else
    {
        LOG_E("rtc alarm create failed");
        ret = -3;
        goto err;
    }

    extern void rt_alarm_dump(void);
    rt_alarm_dump(); // 打印闹钟的信息

    return ret;

err:
    return ret;
}
