/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-02-15 18:01:01
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-25 11:56:24
 * @FilePath: \ele_ds\applications\main.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#include <board.h>
#include <rtthread.h>
#include <drv_gpio.h>
#include <fal.h>
#include <rthw.h>
#include <dfs_fs.h>
#include "drv_spi.h"
#include "spi_flash_sfud.h"

#include "common.h"
#include "update_soft.h"

#ifndef RT_USING_NANO
#include <rtdevice.h>
#endif /* RT_USING_NANO */

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define LED0_PIN GET_PIN(C, 13)
#define V3_3_PIN GET_PIN(C, 8)

const struct dfs_mount_tbl mount_table[] =
{
        // {"norflash0", "/", "elm", 0, 0},
        {0},
};

int mnt_init(void)
{
    if (dfs_mount("norflash0", "/", "elm", 0, 0) == 0)  // "norflash0":挂载的设备名称, "/":挂载路径, 这里挂载到跟目录下
    {
        LOG_D("norflash0 mount successful!");
    }
    else
    {
        dfs_mkfs("elm", "norflash0");  // 如果是第一次挂载文件系统必须要先格式化
        if(dfs_mount("norflash0", "/", "elm", 0, 0) != 0)
        {
            LOG_D("norflash0 mount failed!");
        }
        else
        {
            LOG_D("norflash0 mkfs successful!");
            // 第一次挂载文件系统，需要初始化系统文件
        }
    }

    return 0;
}


static int rt_hw_spi_flash_init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    rt_hw_spi_device_attach("spi1", "spi10", GET_PIN(C, 4)); // spi10 表示挂载在 spi3 总线上的 0 号设备,PC0是片选, 这一步就可以将从设备挂在到总线中。

    if (RT_NULL == rt_sfud_flash_probe("norflash0", "spi10")) // 注册块设备, 这一步可以将外部flash抽象为系统的块设备
    {
        return -RT_ERROR;
    };

    return RT_EOK;
}

static void bootloader_init(void)
{
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(V3_3_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(V3_3_PIN, PIN_HIGH);

    fal_init();

    rt_hw_spi_flash_init();
    mnt_init();
#define VERSION_CUSTOM 01 // 自定义版本号
    uint32_t software_version = SOFTWARE_VERSION_DEC(VERSION_CUSTOM);

    rt_kprintf("ele_ds boot loader init success, date: %s, time: %s.version: %08u\n", __DATE__, __TIME__, software_version);
}
#if 1
/* Bootloader中的跳转代码 */
typedef void (*pFunction)(void);

int jump_to_application(uint32_t app_address)
{
    uint32_t JumpAddress;
    pFunction JumpToApp;
    
    /* 检查应用程序是否有效 */
    if (((*(__IO uint32_t*)app_address) & 0x2FFE0000) == 0x20000000)
    {
        /* 禁用所有中断 */
        __disable_irq();
        
        /* 清除所有中断挂起位 */
        SCB->ICSR = SCB_ICSR_PENDSVCLR_Msk | SCB_ICSR_PENDSTCLR_Msk;
        
        /* 获取复位向量地址 */
        JumpAddress = *(__IO uint32_t*)(app_address + 4);
        
        /* 设置主堆栈指针(MSP) */
        __set_MSP(*(__IO uint32_t*)app_address);
        
        /* 跳转到应用程序 */
        JumpToApp = (pFunction)JumpAddress;
        JumpToApp();
    }
		return 0;
}
#else
/**
 * @brief 跳转到应用程序
 * @param app_address APP起始地址
 * @return 成功返回0，失败返回负数错误码
 */
int jump_to_application(uint32_t app_address)
{
    typedef void (*pFunction)(void);
    pFunction JumpToApplication;
    uint32_t JumpAddress;
    
    /* 检查APP地址是否有效 */
    if ((*(uint32_t*)app_address & 0x2FFE0000) != 0x20000000) {
        LOG_E("Invalid stack pointer at 0x%08X", app_address);
        return -RT_ERROR;
    }
    
    if ((*(uint32_t*)(app_address + 4) & 0xFF000000) != 0x08000000) {
        LOG_E("Invalid reset vector at 0x%08X", app_address + 4);
        return -RT_ERROR;
    }
    
    LOG_I("准备跳转到APP: 地址=0x%08X", app_address);
    
    /* 关闭所有中断 */
    __set_PRIMASK(1);
    
    /* 禁用所有外设时钟 */
    RCC->AHB1ENR = 0;
    RCC->AHB2ENR = 0;
    RCC->AHB3ENR = 0;
    RCC->APB1ENR = 0;
    RCC->APB2ENR = 0;
    
    /* 清除所有挂起的中断 */
    SCB->ICSR = SCB_ICSR_PENDSVCLR_Msk | SCB_ICSR_PENDSTCLR_Msk;
    
    /* 获取APP的堆栈指针和复位向量 */
    JumpAddress = *(uint32_t*)(app_address + 4);
    JumpToApplication = (pFunction)JumpAddress;
    
    /* 设置主堆栈指针 */
    __set_MSP(*(uint32_t*)app_address);
    
    /* 跳转到APP */
    JumpToApplication();
    
    /* 跳转成功不会执行到这里 */
    return RT_EOK;
}
#endif
/**
 * @brief 检查并跳转到APP
 * @return 成功跳转返回0，失败返回负数错误码
 */
int check_and_jump_to_app(void)
{
    const uint32_t APP_START_ADDRESS = 0x08020000; /* APP起始地址 */
    
    /* 检查APP是否有效（简单检查：验证堆栈指针和复位向量） */
    if ((*(uint32_t*)APP_START_ADDRESS & 0x2FFE0000) != 0x20000000) {
        LOG_W("APP无效: 堆栈指针校验失败");
        return -RT_ERROR;
    }
    
    if ((*(uint32_t*)(APP_START_ADDRESS + 4) & 0xFF000000) != 0x08000000) {
        LOG_W("APP无效: 复位向量校验失败");
        return -RT_ERROR;
    }
    
    /* 跳转到APP */
    LOG_I("跳转到APP: 地址=0x%08X", APP_START_ADDRESS);
    return jump_to_application(APP_START_ADDRESS);
}

int main(void)
{
    bootloader_init();

    // 检查是否需要更新app
    uint32_t update_version = 0, current_version = 0;
    bool need_update = is_need_update(&update_version, &current_version);
    if (need_update == true)
    {
        LOG_D("need update");
        update_app(update_version, current_version);
    }
    else
    {
        LOG_D("no need update");
    }
#if 0
    while (1) // 测试 bootloader 功能使用, 不跳转到app
#else
    while (check_and_jump_to_app() != RT_EOK)
#endif
    {
        rt_pin_write(LED0_PIN, PIN_HIGH);
        rt_thread_mdelay(150);
        rt_pin_write(LED0_PIN, PIN_LOW);
        rt_thread_mdelay(150);
    }
}
#if 0
static int ota_app_vtor_reconfig(void)
{
 #define NVIC_VTOR_MASK 0x3FFFFF80
 #define RT_APP_PART_ADDR 0x08008000
 /* ????????? */
 SCB->VTOR = RT_APP_PART_ADDR & NVIC_VTOR_MASK;

 return 0;
}
INIT_BOARD_EXPORT(ota_app_vtor_reconfig);
#endif


