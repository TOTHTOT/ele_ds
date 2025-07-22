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
 * @brief 根据tp4056状态以及电量判断是否需要进入低功耗
 * @return true: 需要进入低功耗, false: 不需要进入低功耗
 */
static bool need_entry_pwrdown(const ele_ds_t dev)
{
    if (dev->device_status.pwrdown_time <= 0)
    {
#if 1
        // 不处于插电状态且电量低于设定值就要进入低功耗
        if (dev_is_charging() == false && dev->sensor_data.curvbat_percent < CFGFILE_DEFAULT_PWRDOWN_VBAT_PERCENT)
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
            // 判断是否进入低功耗
            if (need_entry_pwrdown(&ele_ds))
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
                else
                    LOG_E("event recv failed, ret = %d");
            }
            else
            {
#if 0 // 好像用不上
                // 避免在需要进入低功耗但是, 手动按按键唤醒了设备 need_entry_pwrdown() 返回了 false 错误开启网卡使能问题
                if (dev_is_charging() == true || ele_ds.sensor_data.curvbat_percent >
                    CFGFILE_DEFAULT_PWRDOWN_VBAT_PERCENT)
                    net_dev_ctrl(&ele_ds, true);
                else
                    net_dev_ctrl(&ele_ds, false);
#else
                ret = rt_event_recv(ele_ds.event, ELE_EVENT_CHARGING, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                                    RT_WAITING_NO, &event_recved);
                if (ret == RT_EOK &&
                    /**
                     * 1. 退出标志被置起说明已经被解除初始化了, 这时候可以重新初始化,
                     * 避免在不需要进入低功耗时的插电导致重复初始化网卡
                     */
                    ele_ds.client.exit_flag == true)
                {
                    net_dev_ctrl(&ele_ds, true);
                }
#endif
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
                rt_pin_write(LED0_PIN, !rt_pin_read(LED0_PIN));
            if (ele_ds.device_status.pwrdown_time > 0)
                ele_ds.device_status.pwrdown_time -= 50;
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


