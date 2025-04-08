/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-04-07 14:15:53
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-04-08 17:40:09
 * @FilePath: \ele_ds\applications\lvgl\lv_conf.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2021-10-18     Meco Man      First version
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <rtconfig.h>

#define MY_DISP_HOR_RES          176
#define MY_DISP_VER_RES          264
#define LV_COLOR_DEPTH 1       // 1-bit 色深，适合墨水屏
#define LV_USE_BTN        1

// #define LV_USE_PERF_MONITOR 0
#define LV_COLOR_16_SWAP 0
// #define LV_USE_LOG 1
// #define LV_LOG_LEVEL LV_LOG_LEVEL_TRACE

#ifdef PKG_USING_LV_MUSIC_DEMO
/* music player demo */
#define LV_HOR_RES_MAX              MY_DISP_HOR_RES
#define LV_VER_RES_MAX              MY_DISP_VER_RES
#define LV_USE_DEMO_RTT_MUSIC       1
#define LV_DEMO_RTT_MUSIC_AUTO_PLAY 1
#define LV_FONT_MONTSERRAT_12       1
#define LV_FONT_MONTSERRAT_16       1
#define LV_COLOR_SCREEN_TRANSP      1

#endif


#define LV_USE_DEMO_BENCHMARK       1


//#define LV_USE_DEMO_WIDGETS         1


//#define LV_USE_DEMO_MUSIC           1

#endif
