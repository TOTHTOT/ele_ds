/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-02-15 18:01:01
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-04-21 16:36:59
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
    int32_t ret = 0;
    struct ele_ds ele_ds = {0};
    ret = devices_init(&ele_ds);
    if (ret != RT_EOK)
    {
        LOG_E("devices_init() failed, ret = %d", ret);
        return -RT_ERROR;
    }
    g_ele_ds = &ele_ds;
    
    // 初始化文件系统并加载系统配置
    mnt_init();
    
    extern int lvgl_thread_init(void);
    lvgl_thread_init();

    rt_kprintf("ele_ds init success, date: %s, time: %s\n", __DATE__, __TIME__);
    rt_uint32_t total, used, max_used;
    rt_memory_info(&total, &used, &max_used);
    rt_kprintf("Heap total: %d, used: %d, max_used: %d\n", total, used, max_used);

    while (1)
    {
        ele_ds.ops.sensor_data[SENSOR_MAX](&ele_ds); //获取所有开启的传感器数据
        rt_pin_write(LED0_PIN, PIN_HIGH);
        rt_thread_mdelay(1500);
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_thread_mdelay(1500);
    }
}
static int ota_app_vtor_reconfig(void)
{
 #define NVIC_VTOR_MASK 0x3FFFFF80
 #define RT_APP_PART_ADDR 0x08008000
 /* ????????? */
 SCB->VTOR = RT_APP_PART_ADDR & NVIC_VTOR_MASK;

 return 0;
}
// INIT_BOARD_EXPORT(ota_app_vtor_reconfig);

