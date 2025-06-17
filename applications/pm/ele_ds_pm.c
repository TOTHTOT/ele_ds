/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-05-20 14:10:51
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-06-04 16:59:06
 * @FilePath: \ele_ds\applications\pm\pm.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "ele_ds_pm.h"
#include "ele_ds.h"
#include <stdbool.h>

#define DBG_TAG "ele_pm"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static rt_alarm_t myalarm = RT_NULL;

void user_alarm_callback(rt_alarm_t myalarm, time_t timestamp)
{
    struct tm p_tm;
    time_t now = timestamp;
	
    localtime_r(&now, &p_tm); // 时间戳转换 
    LOG_D("user alarm callback function.");
    LOG_D("curr time: %04d-%02d-%02d %02d:%02d:%02d", p_tm.tm_year + 1900, p_tm.tm_mon + 1, p_tm.tm_mday, p_tm.tm_hour, p_tm.tm_min, p_tm.tm_sec);  // 打印闹钟中断产生时的时间，和设定的闹钟时间比对，以确定得到的是否是想要的结果
}
void alarm_sample(int argc, char **argv)
{
    time_t curr_time;
    struct tm p_tm, alarm_tm;
    struct rt_alarm_setup setup;
    
    if (argc < 2)
    {
        rt_kprintf("Usage: alarm_sample <interval_in_seconds>\n");
        return;
    }

    uint32_t alarm_time = atoi(argv[1]); // 从命令行参数获取闹钟时间间隔，单位为秒

    curr_time = time(NULL);         // 将闹钟的时间设置为当前时间的往后的 5 秒
    gmtime_r(&curr_time, &p_tm); // 将时间戳转换为本地时间，localtime_r 是线程安全的
    LOG_D("now time: %04d-%02d-%02d %02d:%02d:%02d", p_tm.tm_year + 1900, p_tm.tm_mon + 1, p_tm.tm_mday, p_tm.tm_hour,
          p_tm.tm_min, p_tm.tm_sec); // 打印当前时间
    
    // 当前时间+上报时间间隔，设置闹钟时间, 减去本次启动用时, 确保每次定时都一样
    curr_time += 1 * alarm_time /* - (time(NULL) - g_gas_detection_dev->system_starttime) */;
    gmtime_r(&curr_time, &alarm_tm);

    // 设置闹钟标志, 标志错误闹钟中断不对
    LOG_D("alarm time: %04d-%02d-%02d %02d:%02d:%02d",
          alarm_tm.tm_year + 1900, alarm_tm.tm_mon + 1, alarm_tm.tm_mday, alarm_tm.tm_hour,
          alarm_tm.tm_min, alarm_tm.tm_sec); // 打印闹钟时间
    setup.flag = RT_ALARM_ONESHOT;
    setup.wktime.tm_year = alarm_tm.tm_year;
    setup.wktime.tm_mon = alarm_tm.tm_mon;
    setup.wktime.tm_mday = alarm_tm.tm_mday;
    setup.wktime.tm_wday = alarm_tm.tm_wday;
    setup.wktime.tm_hour = alarm_tm.tm_hour;
    setup.wktime.tm_min = alarm_tm.tm_min;
    setup.wktime.tm_sec = alarm_tm.tm_sec;
    // 避免重复创建alarm, 不能放在回调函数执行, 不然 alarm_update() 会卡死
    if (myalarm != NULL)
    {
        rt_alarm_delete(myalarm);
        myalarm = RT_NULL;
    }

    myalarm = rt_alarm_create(user_alarm_callback, &setup); // 创建一个闹钟并设置回调函数
    if (RT_NULL != myalarm)
    {
        rt_err_t ret = rt_alarm_start(myalarm); // 启动闹钟
        if (ret != RT_EOK)
        {
            LOG_E("rtc alarm start failed");
        }
    }
    else
    {
        LOG_E("rtc alarm create failed");
    }
    extern void rt_alarm_dump(void);
    rt_alarm_dump(); // 打印闹钟的信息
}
MSH_CMD_EXPORT(alarm_sample, alarm sample);


void cmd_entry_sleep_mode(void)
{
    alarm_sample(2, (char *[]){"alarm_sample", "10"});

    LOG_D("Entering sleep mode...");

    // 先申请一个深度睡眠, 再释放PM_SLEEP_MODE_NONE, pm框架优先运行等级高的模式, PM_SLEEP_MODE_NONE 为0时才有机会执行别的模式
    rt_pm_request(PM_SLEEP_MODE_DEEP); // 进入深度睡眠模式
    rt_pm_release(PM_SLEEP_MODE_NONE); //释放运行模式
}
MSH_CMD_EXPORT_ALIAS(cmd_entry_sleep_mode, sleep_mode, enter sleep mode);

static void gas_detection_entey_stadnby(void)
{
    __HAL_RCC_PWR_CLK_ENABLE(); // 使能PWR时钟
    HAL_SuspendTick();
    CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU); // 清除Wake_UP标志
    HAL_PWR_EnterSTANDBYMode();        // 进入待机模式
    // HAL_ResumeTick(); // 使用会导致不能正常进入待机模式
}

/**
 * @description: 低功耗时钟管理, gpio 使能或禁用
 * @param {bool} enable 使能或禁用时钟
 * @return {*}
 */
static void pm_clock_init_pwronoff(bool enable)
{
    if (enable)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();
        __HAL_RCC_GPIOE_CLK_ENABLE();
        __HAL_RCC_GPIOF_CLK_ENABLE();
        __HAL_RCC_GPIOG_CLK_ENABLE();
        ele_ds_gpio_init();
    }
    else
    {
        ele_ds_gpio_deinit();
        __HAL_RCC_GPIOA_CLK_DISABLE();
        __HAL_RCC_GPIOB_CLK_DISABLE();
        __HAL_RCC_GPIOC_CLK_DISABLE();
        __HAL_RCC_GPIOD_CLK_DISABLE();
        __HAL_RCC_GPIOE_CLK_DISABLE();
        __HAL_RCC_GPIOF_CLK_DISABLE();
        __HAL_RCC_GPIOG_CLK_DISABLE();
    }
}

static void pm_entry_func(struct rt_pm *pm, uint8_t mode)
{
    static uint32_t entry_stop_cnt = 0; // 进入待机模式需要执行这个函数才能正常进入 不知道为什么
    if (mode != 0)
        LOG_D("Enter sleep mode %d", mode);

    if (rt_interrupt_get_nest() > 0)
    {
        LOG_E("Cannot enter low power mode in interrupt context");
        return;
    }

    // 根据不同模式处理
    switch (mode)
    {
    case PM_SLEEP_MODE_NONE:
        break;

    case PM_SLEEP_MODE_IDLE:
        __WFI(); // Wait For Interrupt 指令，CPU 进入空闲模式
        break;

    case PM_SLEEP_MODE_LIGHT:
        HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI); // 进入轻度睡眠模式
        break;

    case PM_SLEEP_MODE_DEEP:
    {
        entry_stop_cnt++;
        LOG_D("Enter DEEP mode");

        pm_clock_init_pwronoff(false);
        
        // 使能PWR时钟
        __HAL_RCC_PWR_CLK_ENABLE();

        // 停止SysTick定时器
        HAL_SuspendTick();

        // 清除唤醒标志
        __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

        // 进入STOP模式（主调节器保持开启）
        __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
        HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFI);

        // 恢复SysTick定时器
        if (entry_stop_cnt >= 2)
        {
            // 这里要重新初始化时钟, 可能还需要重新初始化外部设备
            SystemClock_Config();
            entry_stop_cnt = 0;
            HAL_ResumeTick();
            pm_clock_init_pwronoff(true);
            LOG_D("exit DEEP mode");
            rt_pm_release(PM_SLEEP_MODE_DEEP); // 退出深度睡眠模式
            rt_pm_request(PM_SLEEP_MODE_NONE); // 退出深度睡眠模式, 进入正常运行模式
        }

        break;
    }

    case PM_SLEEP_MODE_STANDBY:
        // 进入一次会打印两次 应该是没有及时进入停机模式导致的
        gas_detection_entey_stadnby();
        break;

    case PM_SLEEP_MODE_SHUTDOWN:
        // HAL_PWREx_EnterSHUTDOWNMode();  // 进入关闭模式
        break;

    default:
        RT_ASSERT(0); // 出现未知模式时，抛出异常
        break;
    }
}

void ele_ds_pm_init(void)
{
    static const struct rt_pm_ops _ops = {
        pm_entry_func,
        NULL,
        NULL,
        NULL,
        NULL,
    };

    rt_uint8_t timer_mask = 0;

    /* initialize timer mask */
    // timer_mask = 1UL << PM_SLEEP_MODE_DEEP;

    /* initialize system pm module */
    rt_system_pm_init(&_ops, timer_mask, RT_NULL);
}
