/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-02-15 18:01:01
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-30 15:10:58
 * @FilePath: \ele_ds\applications\main.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#include <board.h>
#include <rtthread.h>
#include <drv_gpio.h>
#ifndef RT_USING_NANO
#include <rtdevice.h>
#endif /* RT_USING_NANO */

#include <ele_ds.h>
#include "ele_ds_pm.h"

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/**
 * @brief 根据tp4056状态判断是否需要进入低功耗
 * @return true: 需要进入低功耗, false: 不需要进入低功耗
 */
static bool need_entry_pwrdown(void)
{
    if (g_ele_ds->device_status.pwrdown_time <= 0)
    {
#if 0
        if (rt_pin_read(TP4056_STDBY_PIN) == PIN_HIGH && rt_pin_read(TP4056_CHARGE_PIN) == PIN_HIGH)
            return true;
        else
            return false;
#else
        return true;
#endif
    }
    else
        return false;
}

/**
 * @brief
 * @param ele_ds 设备数据结构
 * @return 0: 成功, -1: ele_ds == NULL
 */
static int32_t update_scr_time(ele_ds_t ele_ds)
{
    uint32_t sleeptime = 0; // 计划是每分钟刷新一次屏幕, 睡眠时间=60-当前时间的秒数;
    time_t curr_time;
    struct tm p_tm;

    if (ele_ds == NULL)
        return -1;

    curr_time = time(NULL);
    localtime_r(&curr_time, &p_tm);
    sleeptime = 60 - p_tm.tm_sec;
    LOG_D("sleep time: %d", sleeptime);
    scr_alarm_start(&ele_ds->scr_alarm, sleeptime);

    return 0;
}


int main(void)
{
    uint32_t loop_times = 0, event_recved = 0;
    int32_t ret = 0;
    struct ele_ds ele_ds = {0};
    g_ele_ds = &ele_ds;

    ret = devices_init(&ele_ds);
    if (ret != RT_EOK)
    {
        LOG_E("devices_init() failed, ret = %d", ret);
        return -RT_ERROR;
    }

    rt_kprintf("ele_ds init success, date: %s, time: %s\n", __DATE__, __TIME__);
    while (1)
    {
        if (ele_ds.device_status.entry_deepsleep == false)
        {
            // 在插电时不进入低功耗
            if (need_entry_pwrdown())
            {
                ret = rt_event_recv(ele_ds.event, ELE_EVENT_SCRFINISH, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                                    RT_WAITING_NO, &event_recved);
                if (ret == RT_EOK)
                {
                    // 屏幕刷新完成等待一下进入低功耗, 为了让lvgl的功能完成
                    rt_thread_mdelay(100);

                    update_scr_time(&ele_ds);
                    ele_ds.device_status.refresh_scr_act = true;

                    rt_pm_request(PM_SLEEP_MODE_DEEP); // 进入深度睡眠模式
                    rt_pm_release(PM_SLEEP_MODE_NONE); //释放运行模式
                }
            }
            else
            {
                // 允许在活动时实时刷新屏幕
                if (ele_ds.device_status.refresh_scr_act == false)
                {
                    ele_ds.device_status.refresh_scr_act = true;
                    update_scr_time(&ele_ds);
                    // 避免上一次刷新的事件没清除在刷新时进入了低功耗
                    rt_event_recv(ele_ds.event, ELE_EVENT_SCRFINISH, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                                        RT_WAITING_NO, &event_recved);
                }
            }
            // else
            //     LOG_D("in charge or stdby");
            if (loop_times % 100 == 0)
            {
                ele_ds.ops.sensor_data[SENSOR_MAX](&ele_ds); //获取所有开启的传感器数据
                ele_ds.ops.get_curvbat(&ele_ds); //获取当前电压
            }
            if (loop_times % 50 == 0)
            {
                rt_pin_write(LED0_PIN, !rt_pin_read(LED0_PIN));
            }
            if (ele_ds.device_status.pwrdown_time > 0)
            {
                ele_ds.device_status.pwrdown_time -= 50;
                // LOG_D("pwrdown_time: %d", ele_ds.device_status.pwrdown_time);
            }
            loop_times++;
            rt_thread_mdelay(50);
        }
        else
        {
            rt_thread_mdelay(500);
            rt_pm_request(PM_SLEEP_MODE_DEEP); // 进入深度睡眠模式
            rt_pm_release_all(PM_SLEEP_MODE_NONE); //释放运行模式
        }

    }
}
#if 1
static int ota_app_vtor_reconfig(void)
{
    SCB->VTOR = FLASH_BASE | 0x020000;
    return 0;
}
INIT_BOARD_EXPORT(ota_app_vtor_reconfig);
#endif


