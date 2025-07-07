#ifndef __ELE_DS_PM_H__
#define __ELE_DS_PM_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <drv_gpio.h>
#include <stdbool.h>

extern void ele_ds_pm_init(void);
extern void pm_clock_init_pwronoff(bool enable);
#endif /* __ELE_DS_PM_H__ */

