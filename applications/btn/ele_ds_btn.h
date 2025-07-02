/*
 * @Author: TOTHTOT
 * @Date: 25-7-2
 * @LastEditors: TOTHTOT
 * @FilePath:btn.h
 * @Description: None
 */


#ifndef BTN_H
#define BTN_H

#include <stdint.h>

typedef struct
{
    uint16_t btnval;
    uint8_t release_press; // 0 释放, 1 按下
}ele_ds_ui_btn_t; // 按键线程通知ui的消息队列结构体

typedef struct
{
    uint32_t raw_btnval;
    uint32_t ui_btnval;
}ui_btnval_map_t;
extern const ui_btnval_map_t ui_btnval_map[4];
#define UI_BTNVAL_MAP_SIZE sizeof(ui_btnval_map) / sizeof(ui_btnval_map[0])

#endif //BTN_H
