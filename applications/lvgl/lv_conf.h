/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-04-07 14:15:53
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-14 17:56:50
 * @FilePath: \ele_ds\applications\lvgl\lv_conf.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef LV_CONF_H
#define LV_CONF_H

#include <rtconfig.h>

#define MY_DISP_HOR_RES 264
#define MY_DISP_VER_RES 176

#define LV_HOR_RES_MAX MY_DISP_HOR_RES
#define LV_VER_RES_MAX MY_DISP_VER_RES
#define LV_COLOR_DEPTH 1 // 1-bit 色深，适合墨水屏
#define LV_USE_BTN 1
#define LV_MEM_SIZE (10U * 1024U) /*[bytes]*/

#define LV_MEM_CUSTOM 1
#define LV_MEM_CUSTOM_ALLOC   rt_malloc
#define LV_MEM_CUSTOM_FREE    rt_free
#define LV_COLOR_DEPTH 1
#define LV_USE_ANIMATION 0
#define LV_FONT_MONTSERRAT_10 1
#define LV_FONT_MONTSERRAT_12 1

// 解决最大刷新行数'38'问题, 增大缓冲区, 10240/MY_DISP_VER_RES = 38, 10240 是默认值
#define LV_DISP_ROT_MAX_BUF (50*1024)

#define LV_USE_FS_POSIX 1
#if LV_USE_FS_POSIX
    #define LV_FS_POSIX_LETTER 'S'     /*Set an upper cased letter on which the drive will accessible (e.g. 'A')*/
    #define LV_FS_POSIX_CACHE_SIZE 1024    /*>0 to cache this number of bytes in lv_fs_read()*/
#endif

// #define LV_USE_PERF_MONITOR 0
#define LV_COLOR_16_SWAP 0
// #define LV_USE_LOG 1
// #define LV_LOG_LEVEL LV_LOG_LEVEL_TRACE

#ifdef PKG_USING_LV_MUSIC_DEMO
/* music player demo */
#define LV_HOR_RES_MAX MY_DISP_HOR_RES
#define LV_VER_RES_MAX MY_DISP_VER_RES
#define LV_USE_DEMO_RTT_MUSIC 1
#define LV_DEMO_RTT_MUSIC_AUTO_PLAY 1
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_COLOR_SCREEN_TRANSP 1

#endif
#define LV_FONT_CUSTOM 1  // 启用自定义字体

// #define LV_USE_DEMO_BENCHMARK 1

// #define LV_USE_DEMO_WIDGETS         1

// #define LV_USE_DEMO_MUSIC           1

#endif
