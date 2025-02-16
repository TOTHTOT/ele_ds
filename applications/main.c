/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-02-15 18:01:01
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-02-16 19:22:54
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
    ele_ds_t ele_ds =  devices_init();
    if (ele_ds == NULL)
    {
        LOG_E("No memory for ele_ds");
        return -RT_ERROR;
    }
    gps6816d_data_t data = {0};
    while (1)
    {
        // ele_ds->ops.get_gps6816d_data(&data);
        rt_pin_write(LED0_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_thread_mdelay(500);
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
INIT_BOARD_EXPORT(ota_app_vtor_reconfig);

