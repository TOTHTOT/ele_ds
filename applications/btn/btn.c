/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-05-26 16:25:21
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-26 16:39:17
 * @FilePath: \ele_ds\packages\MFBD-latest\examples\btn.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#include <rtthread.h>
#include <board.h>
#include "mfbd.h"
#include "mfbd_sd.h"
#include <drv_gpio.h>

#define MFBD_DEMO_USE_DEFAULT_DEFINE            1       /* set to 1, you can study how to use default define APIs. */

void bsp_btn_value_report(mfbd_btn_code_t btn_value);
unsigned char bsp_btn_check(mfbd_btn_index_t btn_index);

#ifndef BTN_KEY0
    #define BTN_KEY0 GET_PIN(B, 13)
#endif

#ifndef BTN_KEY1
    #define BTN_KEY1 GET_PIN(B, 12)
#endif

#ifndef BTN_KEY2
    #define BTN_KEY2 GET_PIN(C, 13)
#endif

#ifndef BTN_WK_UP
    #define BTN_WK_UP GET_PIN(A, 0)
#endif

enum
{
    MFBD_DOWN_CODE_NAME(test_tbtn)      = 0x1201,
    MFBD_UP_CODE_NAME(test_tbtn)        = 0x1200,
    MFBD_LONG_CODE_NAME(test_tbtn)      = 0x1202,

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

mfbd_btn_code_t MFBD_DOWN_CODES_NAME(test_mbtn)[4] = {0x1501, 0x1511, 0x1521, 0x1531};


/* MFBD_NBTN_DEFAULT_DEFINE(NAME, BTN_INDEX, FILTER_TIME, REPEAT_TIME, LONG_TIME) */
MFBD_NBTN_DEFAULT_DEFINE(test_nbtn, 1, 3, 0, 100);
MFBD_NBTN_DEFAULT_DEFINE(test_nbtn1, 2, 3, 0, 100);
MFBD_NBTN_DEFAULT_DEFINE(test_nbtn2, 3, 3, 0, 100);

MFBD_NBTN_ARRAYLIST(test_nbtn_list, &test_nbtn1, &test_nbtn);

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
    case 3:
        if (rt_pin_read(BTN_KEY1) == 0)
        {
            return MFBD_BTN_STATE_DOWN;
        }
        break;
    case 2:
        if (rt_pin_read(BTN_KEY0) == 0)
        {
            return MFBD_BTN_STATE_DOWN;
        }
        break;
    case 1:
        if (rt_pin_read(BTN_WK_UP) == 0)
        {
            return MFBD_BTN_STATE_DOWN;
        }
        break;
    default:
        break;
    }
    return MFBD_BTN_STATE_UP;
}

void bsp_btn_value_report(mfbd_btn_code_t btn_value)
{
    rt_kprintf("%04x\n", btn_value);
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

    rt_pin_mode(BTN_KEY0, PIN_MODE_INPUT_PULLUP); /* set KEY pin mode to input */
    rt_pin_mode(BTN_KEY1, PIN_MODE_INPUT_PULLUP); /* set KEY pin mode to input */
    // rt_pin_mode(BTN_KEY2, PIN_MODE_INPUT_PULLUP); /* set KEY pin mode to input */
    rt_pin_mode(BTN_WK_UP, PIN_MODE_INPUT_PULLUP); /* set KEY pin mode to input */

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

