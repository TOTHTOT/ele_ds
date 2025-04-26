/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-04-06 18:08:37
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-04-26 10:02:05
 * @FilePath: \ele_ds\applications\fal\fal_cfg.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
 
#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_
 
#include <rtconfig.h>
#include <board.h>
 
#define NOR_FLASH_DEV_NAME             "norflash0"
 
#define FLASH_SIZE_GRANULARITY_16K   (4 * 16 * 1024)
#define FLASH_SIZE_GRANULARITY_64K   (1 * 64 * 1024)
#define FLASH_SIZE_GRANULARITY_128K  (7 * 128 * 1024)
#define STM32_FLASH_START_ADRESS_16K  STM32_FLASH_START_ADRESS
#define STM32_FLASH_START_ADRESS_64K  (STM32_FLASH_START_ADRESS_16K + FLASH_SIZE_GRANULARITY_16K)
#define STM32_FLASH_START_ADRESS_128K (STM32_FLASH_START_ADRESS_64K + FLASH_SIZE_GRANULARITY_64K)
 
 
/* ===================== Flash device Configuration ========================= */
extern const struct fal_flash_dev stm32_onchip_flash_16k;
extern const struct fal_flash_dev stm32_onchip_flash_64k;
//extern const struct fal_flash_dev stm32_onchip_flash_128k;
 
extern struct fal_flash_dev nor_flash0;
 
/* flash device table */
#define FAL_FLASH_DEV_TABLE                                          \
{                                                                    \
    &stm32_onchip_flash_16k,                                         \
    &stm32_onchip_flash_64k,                                         \
    &nor_flash0,                                                     \
}
/* ====================== Partition Configuration ========================== */
#ifdef FAL_PART_HAS_TABLE_CFG
/* partition table */
#define FAL_PART_TABLE                                                                                  \
{                                                                                                       \
    {FAL_PART_MAGIC_WORD, "bootloader",    "stm32_onchip",         0,  FLASH_SIZE_GRANULARITY_16K, 0},  \
    {FAL_PART_MAGIC_WORD,       "app",     "stm32_onchip",         0,  FLASH_SIZE_GRANULARITY_64K, 0},  \
    {FAL_PART_MAGIC_WORD, "easyflash", NOR_FLASH_DEV_NAME,         0,  1024*1024, 0}, \
    {FAL_PART_MAGIC_WORD,  "download", NOR_FLASH_DEV_NAME, 1024*1024,  1024*1024, 0}, \
}
#endif /* FAL_PART_HAS_TABLE_CFG */
 
#endif /* _FAL_CFG_H_ */