/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-04-20 13:23:44
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-23 16:46:39
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

/**
 * @brief 将月份名称转换为数字（例如 "Aug" → 8）
 */
#define MONTH_TO_NUM(month)                                                                                                     \
    ((month[0] == 'J' && month[1] == 'a' && month[2] == 'n') ? 1 : (month[0] == 'F' && month[1] == 'e' && month[2] == 'b') ? 2  \
                                                               : (month[0] == 'M' && month[1] == 'a' && month[2] == 'r')   ? 3  \
                                                               : (month[0] == 'A' && month[1] == 'p' && month[2] == 'r')   ? 4  \
                                                               : (month[0] == 'M' && month[1] == 'a' && month[2] == 'y')   ? 5  \
                                                               : (month[0] == 'J' && month[1] == 'u' && month[2] == 'n')   ? 6  \
                                                               : (month[0] == 'J' && month[1] == 'u' && month[2] == 'l')   ? 7  \
                                                               : (month[0] == 'A' && month[1] == 'u' && month[2] == 'g')   ? 8  \
                                                               : (month[0] == 'S' && month[1] == 'e' && month[2] == 'p')   ? 9  \
                                                               : (month[0] == 'O' && month[1] == 'c' && month[2] == 't')   ? 10 \
                                                               : (month[0] == 'N' && month[1] == 'o' && month[2] == 'v')   ? 11 \
                                                               : (month[0] == 'D' && month[1] == 'e' && month[2] == 'c')   ? 12 \
                                                                                                                           : 0)

/**
 * @brief 从 __DATE__ 中提取月份（例如 "Aug" → 8）
 */
#define EXTRACT_MONTH(date) \
    MONTH_TO_NUM(date)

/**
 * @brief 从 __DATE__ 中提取日期（例如 "15" → 15，" 5" → 5）
 */
#define EXTRACT_DAY(date) \
    ((((const char *)date)[4] == ' ') ? (((const char *)date)[5] - '0') : ((((const char *)date)[4] - '0') * 10 + (((const char *)date)[5] - '0')))

/**
 * @brief 从 __DATE__ 中提取年份后两位（例如 "2025" → 25）
 */
#define EXTRACT_YEAR(date) \
    ((((const char *)date)[9] - '0') * 10 + (((const char *)date)[10] - '0'))

/**
 * @brief 生成基于编译时间的十六进制软件版本号（格式：0xAABBCCDD）
 * @param custom 自定义信息（范围：00~FF）
 * @return 组合后的版本号（uint32_t）
 */
#define SOFTWARE_VERSION(custom)                             \
    ((uint32_t)((((EXTRACT_YEAR(__DATE__)) & 0xFF) << 24) |  \
                (((EXTRACT_MONTH(__DATE__)) & 0xFF) << 16) | \
                (((EXTRACT_DAY(__DATE__)) & 0xFF) << 8) |    \
                ((custom) & 0xFF)))

/**
 * @brief 生成基于编译时间的十进制软件版本号（格式：YYMMDDXX）
 * @param custom 自定义信息（范围：00~99）
 * @return 组合后的版本号（uint32_t）
 */
#define SOFTWARE_VERSION_DEC(custom)                 \
    ((uint32_t)((EXTRACT_YEAR(__DATE__) * 1000000) + \
                (EXTRACT_MONTH(__DATE__) * 10000) +  \
                (EXTRACT_DAY(__DATE__) * 100) +      \
                (custom)))

#if 0
extern int untar(const char *tar_path, const char *dst_dir);
#endif
extern void print_array_with_prefix(const char *prefix, const uint8_t *array, size_t size);
extern bool is_json(const char *data, size_t len);
extern uint32_t crcfile(int argc, char **argv);
extern char *find_json_start(char *buffer, int32_t len);

#endif /* __COMMON_H__ */
