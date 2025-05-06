/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-04-20 13:23:44
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-03 09:50:56
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

extern int untar(const char *tar_path, const char *dst_dir);
extern void print_array_with_prefix(const char *prefix, const uint8_t *array, size_t size);
extern bool is_json(const char *data, size_t len);
extern uint32_t crcfile(int argc, char **argv);
extern char *find_json_start(char *buffer, int32_t len);

#endif /* __COMMON_H__ */

