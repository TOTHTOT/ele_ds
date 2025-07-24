/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-05-26 16:25:21
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-26 20:45:51
 * @FilePath: \ele_ds\packages\MFBD-latest\examples\btn.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#include <rtthread.h>
#include <board.h>
#include "mfbd.h"
#include "mfbd_sd.h"
#include "ele_ds_btn.h"
#include <drv_gpio.h>
#include <beep.h>
#include <stdbool.h>
#include <lvgl.h>
#include "ele_ds.h"

#define MFBD_DEMO_USE_DEFAULT_DEFINE            1       /* set to 1, you can study how to use default define APIs. */

void bsp_btn_value_report(mfbd_btn_code_t btn_value);
unsigned char bsp_btn_check(mfbd_btn_index_t btn_index);

enum
{
    MFBD_DOWN_CODE_NAME(test_nbtn)      = 0x1301,
    MFBD_UP_CODE_NAME(test_nbtn)        = 0x1300,
    MFBD_LONG_CODE_NAME(test_nbtn)      = 0x1302,

    MFBD_DOWN_CODE_NAME(test_nbtn1)     = 0x1401,
    MFBD_UP_CODE_NAME(test_nbtn1)       = 0x1400,
    MFBD_LONG_CODE_NAME(test_nbtn1)     = 0x1402,
    
    MFBD_DOWN_CODE_NAME(test_nbtn2)     = 0x1501,
    MFBD_UP_CODE_NAME(test_nbtn2)       = 0x1500,
    MFBD_LONG_CODE_NAME(test_nbtn2)     = 0x1502,
};

/**
 * @brief 给ui使用的映射表
 */
const ui_btnval_map_t ui_btnval_map[4] = {
    {0x1501, LV_KEY_LEFT},
    {0x1401, LV_KEY_ENTER},
    {0x1301, LV_KEY_RIGHT},
    {0x1402, LV_KEY_ESC},
};

/* MFBD_NBTN_DEFAULT_DEFINE(NAME, BTN_INDEX, FILTER_TIME, REPEAT_TIME, LONG_TIME) */
MFBD_NBTN_DEFAULT_DEFINE(test_nbtn, 1, 3, 0, 100);
MFBD_NBTN_DEFAULT_DEFINE(test_nbtn1, 2, 3, 0, 100);
MFBD_NBTN_DEFAULT_DEFINE(test_nbtn2, 3, 3, 0, 100);

MFBD_NBTN_ARRAYLIST(test_nbtn_list, &test_nbtn1, &test_nbtn, &test_nbtn2);

const mfbd_group_t test_btn_group =
{
    bsp_btn_check,
    bsp_btn_value_report,

#if MFBD_USE_TINY_BUTTON
    test_tbtn_list,
#endif /* MFBD_USE_TINY_BUTTON */

#if MFBD_USE_NORMAL_BUTTON
    test_nbtn_list,
#endif /* MFBD_USE_NORMAL_BUTTON */

#if MFBD_USE_MULTIFUCNTION_BUTTON
    test_mbtn_list,
#endif /* MFBD_USE_MULTIFUCNTION_BUTTON */

#if MFBD_PARAMS_SAME_IN_GROUP

#if MFBD_USE_TINY_BUTTON || MFBD_USE_NORMAL_BUTTON || MFBD_USE_MULTIFUCNTION_BUTTON
    3,
#endif /*  MFBD_USE_TINY_BUTTON || MFBD_USE_NORMAL_BUTTON || MFBD_USE_MULTIFUCNTION_BUTTON */

#if MFBD_USE_NORMAL_BUTTON || MFBD_USE_MULTIFUCNTION_BUTTON
    30,
    150,
#endif /* MFBD_USE_NORMAL_BUTTON || MFBD_USE_MULTIFUCNTION_BUTTON */

#if MFBD_USE_MULTIFUCNTION_BUTTON
    75,
#endif /* MFBD_USE_MULTIFUCNTION_BUTTON */

#endif /*MFBD_PARAMS_SAME_IN_GROUP*/

};


unsigned char bsp_btn_check(mfbd_btn_index_t btn_index)
{
    switch (btn_index)
    {
    case 1:
        if (rt_pin_read(LEFT_KEY) == 0)
        {
            return MFBD_BTN_STATE_DOWN;
        }
        break;
    case 2:
        if (rt_pin_read(MID_KEY) == 0)
        {
            return MFBD_BTN_STATE_DOWN;
        }
        break;
    case 3:
        if (rt_pin_read(RIGHT_KEY) == 0)
        {
            return MFBD_BTN_STATE_DOWN;
        }
        break;
    default:
        break;
    }
    return MFBD_BTN_STATE_UP;
}

static bool btn_click_islong(mfbd_btn_code_t btn_value)
{
    if (btn_value == MFBD_LONG_CODE_NAME(test_nbtn) ||
        btn_value == MFBD_LONG_CODE_NAME(test_nbtn1) ||
        btn_value == MFBD_LONG_CODE_NAME(test_nbtn2))
    {
        return true; // 如果是长按事件
    }
    else
        return false;
}

static bool btn_click_isup(mfbd_btn_code_t btn_value)
{
    if (btn_value == MFBD_UP_CODE_NAME(test_nbtn) ||
        btn_value == MFBD_UP_CODE_NAME(test_nbtn1) ||
        btn_value == MFBD_UP_CODE_NAME(test_nbtn2))
    {
        return true; // 如果是松开事件
    }
    else
        return false;
}

static void btn_curclick_ctrbeep(mfbd_btn_code_t btn_value)
{
    static mfbd_btn_code_t pre_value = 0; // 保存上一次的按键值, 避免长按时多次触发

    if (btn_click_isup(btn_value) || btn_click_islong(btn_value))
    {
        // rt_kprintf("Button value: %d %d\n", pre_value, btn_value);
        if (pre_value != btn_value) // 避免长按时多次触发
        {
            // 上一个是长按本次松开不响应
            if (btn_click_islong(pre_value))
            {
                // rt_kprintf("Button long pressed: %04x\n", btn_value);
            }
            else
                beep(1, 50, 50, 0); // 蜂鸣器响一次
            pre_value = btn_value;
        }
        else // 短按也得响
            beep(1, 50, 50, 0); // 蜂鸣器响一次
    }
}

void bsp_btn_value_report(mfbd_btn_code_t btn_value)
{
    // 只有按键松开蜂鸣器才响
    btn_curclick_ctrbeep(btn_value);
    rt_kprintf("%04x\n", btn_value);
    if (g_ele_ds->init_flag == true)
    {
        // 屏蔽退出低功耗后第一次按键, 不然中断唤醒的情况下会切换页面
        if (g_ele_ds->device_status.pwr_on_firstkey == false)
        {
            g_ele_ds->device_status.pwrdown_time = g_ele_ds->device_cfg.setting.pwrdown_time;
            ele_ds_ui_btn_t btn_msg = {0};
            btn_msg.btnval = btn_value;
            btn_msg.release_press = LV_INDEV_STATE_PR;
            rt_mq_send(g_ele_ds->ui_btn_mq, &btn_msg, sizeof
                       (ele_ds_ui_btn_t));
        }
        else
            g_ele_ds->device_status.pwr_on_firstkey = false;
    }
}

static void mfbd_scan(void *arg)
{
    while (1)
    {
        #if MFBD_USE_SECTION_DEFINITION
        /* use section definition. */

        MFBD_GROUP_SCAN(test_btns);

        #else

        mfbd_group_scan(&test_btn_group); /* scan button group */

        #endif  /*MFBD_USE_SECTION_DEFINITION*/

        rt_thread_mdelay(10); /* scan period: 10ms */
    }
}

static void user_button_init(void)
{
    // rt_pin_mode(LEFT_KEY, PIN_MODE_INPUT_PULLUP); /* set KEY pin mode to input */
    // rt_pin_mode(MID_KEY, PIN_MODE_INPUT_PULLUP); /* set KEY pin mode to input */
    // rt_pin_mode(RIGHT_KEY, PIN_MODE_INPUT_PULLUP); /* set KEY pin mode to input */
}

int mfbd_main(void)
{
    rt_thread_t tid = RT_NULL;

    user_button_init();

    /* Create background ticks thread */
    tid = rt_thread_create("mfbd", mfbd_scan, RT_NULL, 1024, 10, 10);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }

    return 0;
}

INIT_APP_EXPORT(mfbd_main);

