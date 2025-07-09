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

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

int main(void)
{
    uint32_t loop_times = 0;
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
            if (loop_times % 10 == 0)
            {
                ele_ds.ops.sensor_data[SENSOR_MAX](&ele_ds); //获取所有开启的传感器数据
                ele_ds.ops.get_curvbat(&ele_ds); //获取当前电压
            }
            if (loop_times % 50 == 0)
            {
                rt_pin_write(LED0_PIN, !rt_pin_read(LED0_PIN));
            }
        }
        loop_times++;
        rt_thread_mdelay(50);
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


