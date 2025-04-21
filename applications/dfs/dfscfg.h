/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-04-05 12:21:47
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-04-21 15:56:32
 * @FilePath: \ele_ds\applications\dfs\dfscfg.h
 * @Description: dfs文件系统相关配置
 */
#ifndef __DFSCFG_H__
#define __DFSCFG_H__

#include <dfs_fs.h>
#include <stdint.h>
#include <unistd.h>

#define SYSFILE_PATH "/sysfile"
#define CONFIG_FILE_PATH SYSFILE_PATH "/config"

extern int mnt_init(void);
extern int32_t write_ele_ds_cfg(ele_ds_cfg_t *cfg);
extern int32_t read_ele_ds_cfg(ele_ds_cfg_t *cfg);

#endif /* __DFSCFG_H__ */
