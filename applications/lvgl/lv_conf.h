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
