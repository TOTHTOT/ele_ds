/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-04-20 13:23:44
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-19 13:35:05
 * @FilePath: \ele_ds\applications\common\common.h
 * @Description: 常用函数头文件
 */
#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "microtar.h"

#ifdef __RTTHREAD__
#include <rtthread.h>
#include <rtdevice.h>
#include <dfs_fs.h>
#include <dfs_file.h>
#endif /* __RTTHREAD__ */

#define MAX_PATH 256
#include <rtthread.h>

#define TIME_START(label) rt_tick_t __start_tick_##label = rt_tick_get()
#define TIME_END(label)                                                                                                    \
    do                                                                                                                     \
    {                                                                                                                      \
        rt_tick_t __end_tick_##label = rt_tick_get();                                                                      \
        rt_kprintf("[TIME] %s: %d ms\n", #label, (__end_tick_##label - __start_tick_##label) * 1000 / RT_TICK_PER_SECOND); \
    } while (0)

extern int untar(const char *tar_path, const char *dst_dir);
extern void print_array_with_prefix(const char *prefix, const uint8_t *array, size_t size);
extern bool is_json(const char *data, size_t len);
extern uint32_t crcfile(int argc, char **argv);
extern char *find_json_start(char *buffer, int32_t len);
extern bool net_islink(void);
extern bool dev_is_charging(void);
#endif /* __COMMON_H__ */

