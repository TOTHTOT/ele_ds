/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-04-20 13:23:44
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-04-20 13:26:23
 * @FilePath: \ele_ds\applications\common\common.h
 * @Description: 常用函数头文件
 */
#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "microtar.h"

#ifdef __RTTHREAD__
#include <rtthread.h>
#include <rtdevice.h>
#include <dfs_fs.h>
#include <dfs_file.h>
#endif /* __RTTHREAD__ */

#define MAX_PATH 256

extern int untar(const char *tar_path, const char *dst_dir);


#endif /* __COMMON_H__ */

