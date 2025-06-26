/*
 * @Author: TOTHTOT
 * @Date: 25-6-26
 * @LastEditors: TOTHTOT
 * @FilePath:custom_symbol_16.h
 * @Description: None
 */


#ifndef CUSTOM_SYMBOL_16_H
#define CUSTOM_SYMBOL_16_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

//声明字体
LV_FONT_DECLARE(custom_symbol_16);

//宏定义
//被替换的内容，如"\xEE\x9C\x8F"是通过图标的Unicode编码得到其UTF-8编码
//至于格式为什么是这个样子，不清楚
#define LV_SYMBOL_WIFI_LINK "\xEE\x98\x8E"
#define LV_SYMBOL_WIFI_UNLINK "\xEE\x98\xB2"
#define LV_SYMBOL_MESSAGE "\xEE\x98\xAB"


#ifdef __cplusplus
}
#endif

#endif //CUSTOM_SYMBOL_16_H
