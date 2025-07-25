﻿/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-10-18     Meco Man     The first version
 */
#include <lvgl.h>
#include <stdbool.h>
#include <rtdevice.h>
#include <board.h>
#include <ele_ds.h>
#include <time.h>
#include "custom_symbol_16.h"

void lv_port_indev_init(void)
{

}

#define STATUS_BAR_WIDTH_PCT 15
#define STATUS_BAR_CENTER_WIDTH_PCT 70
#define WEATHER_LAYOUT_WIDTH_PCT 50

struct real_time_change_lvobj
{
    lv_obj_t *tabview; // 底部导航栏
    lv_obj_t *sensor_lab; // 传感器数据标签
    lv_obj_t *memo_lab; // 备忘录标签
    lv_obj_t *weather_icon[2]; // 天气图标
    lv_obj_t *weather_info_lab[2]; // 天气信息标签, 温度
    lv_obj_t *vbat; // 电池图标
    lv_obj_t *wifi; // wifi图标
    lv_obj_t *time_lab; // 时间标签
    lv_obj_t *message; // 消息图标
} rtc_lvobj;

typedef struct 
{
    uint32_t icon_code;
    int8_t temp_max;
    int8_t temp_min;
    int32_t day; // 0 今天, 1 明天, -1 昨天
}epd_show_weather_info_t;


// 设置字体
extern lv_font_t fangsong_8;

/* 样式 */
static lv_style_t style_small;
static lv_style_t style_bold;
static lv_style_t style_large;
static lv_style_t style_font;
static lv_style_t style_custom_symbol_16;

/**
 * @description: 设置电量图标
 * @param {lv_obj_t} *vbat 电量图标对象
 * @param {uint8_t} vbat_percent 电量百分比
 * @return {*}
 */
static void set_vbat_icon(lv_obj_t *vbat, uint8_t vbat_percent)
{
    if (vbat == NULL)
    {
        return;
    }
    if (vbat_percent > 100)
    {
        lv_label_set_text(vbat, LV_SYMBOL_BATTERY_FULL);
    }
    else if (vbat_percent > 75)
    {
        lv_label_set_text(vbat, LV_SYMBOL_BATTERY_3);
    }
    else if (vbat_percent > 50)
    {
        lv_label_set_text(vbat, LV_SYMBOL_BATTERY_2);
    }
    else if (vbat_percent > 25)
    {
        lv_label_set_text(vbat, LV_SYMBOL_BATTERY_1);
    }
    else
    {
        lv_label_set_text(vbat, LV_SYMBOL_BATTERY_EMPTY);
    }
}

/**
 * @description: 设置wifi图标
 * @param {lv_obj_t} *wifi wifi图标对象
 * @param {bool} is_connected 是否连接
 * @return {*}
 */
static void set_wifi_icon(lv_obj_t *wifi, bool is_connected)
{
    if (wifi == NULL)
    {
        return;
    }

    if (is_connected)
    {
        lv_label_set_text(wifi, LV_SYMBOL_WIFI_LINK);
    }
    else
    {
        lv_label_set_text(wifi, LV_SYMBOL_WIFI_UNLINK);
    }
}

static void update_time_cb(lv_timer_t * timer)
{
    time_t curtime = time(NULL);
    struct tm *tm_info = localtime(&curtime);
    char str[100] = {0};
    strftime(str, sizeof(str), "%Y-%m-%d %H:%M", tm_info);
    // lv_obj_align(time_label, LV_ALIGN_CENTER, -10, 0);
    lv_label_set_text(rtc_lvobj.time_lab, str);

    if (rtc_lvobj.sensor_lab != NULL)
    {
        sprintf(str, DEFAULT_SENSOR_LABFMT, g_ele_ds->sensor_data.sht30[0], g_ele_ds->sensor_data.sht30[1], g_ele_ds->sensor_data.gzp6816d.pressure);
        lv_label_set_text(rtc_lvobj.sensor_lab, str);
    }
    if (rtc_lvobj.vbat != NULL)
    {
        set_vbat_icon(rtc_lvobj.vbat, 20);
    }
}

static lv_obj_t* create_status_bar(ele_ds_t dev, lv_obj_t *up, lv_obj_t* parent, lv_obj_t* down)
{
    // 状态栏布局
    lv_obj_t* status_bar = lv_obj_create(parent);
    lv_obj_set_flex_flow(status_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_size(status_bar, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(status_bar, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(status_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(status_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(status_bar, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_width(status_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(status_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(status_bar,
                          LV_FLEX_ALIGN_SPACE_BETWEEN, // 主轴两端对齐
                          LV_FLEX_ALIGN_CENTER, // 交叉轴居中对齐
                          LV_FLEX_ALIGN_CENTER); // 行对齐
    lv_obj_align(status_bar, LV_ALIGN_TOP_MID, 0, 0);

    // /* 左侧容器 - 包含电量和WiFi图标 */
    // lv_obj_t* left_cont = lv_obj_create(status_bar);
    // lv_obj_set_flex_flow(left_cont, LV_FLEX_FLOW_ROW);
    // lv_obj_set_flex_align(left_cont,
    //                      LV_FLEX_ALIGN_START,      // 主轴起点对齐
    //                      LV_FLEX_ALIGN_CENTER,     // 交叉轴居中对齐
    //                      LV_FLEX_ALIGN_CENTER);    // 行对齐
    // lv_obj_set_size(left_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    // // 电量初始化
    // rtc_lvobj.vbat = lv_label_create(left_cont);
    // lv_obj_add_style(rtc_lvobj.vbat, &style_bold, 0);
    // set_vbat_icon(rtc_lvobj.vbat, (uint8_t)dev->sensor_data.curvbat_percent);
    //
    // // wifi 初始化
    // rtc_lvobj.wifi = lv_label_create(left_cont);
    // lv_obj_add_style(rtc_lvobj.wifi, &style_custom_symbol_16, 0);
    // set_wifi_icon(rtc_lvobj.wifi, dev->device_status.cnt_wifi);

    // 时间初始化
    rtc_lvobj.time_lab = lv_label_create(status_bar);
    time_t curtime = time(NULL);
    struct tm *tm_info = localtime(&curtime);
    char time_str[32] = {0};
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M", tm_info);
    lv_obj_add_style(rtc_lvobj.time_lab, &style_bold, 0);
    lv_label_set_text(rtc_lvobj.time_lab, time_str);

    // 新消息初始化
    // rtc_lvobj.message = lv_label_create(status_bar);
    // lv_obj_add_style(rtc_lvobj.message, &style_custom_symbol_16, 0);
    // lv_label_set_text(rtc_lvobj.message, LV_SYMBOL_MESSAGE);

    // 创建定时器，周期 20s 更新状态
    lv_timer_create(update_time_cb, 20000, NULL);

    return status_bar;
}

/**
 * @description: 根据当前天气信息返回对应后缀, 都不匹配返回 qing
 * @param {uint32_t} icon_code 当前要显示的天气图标代码
 * @return {*}
 */
static char *get_weather_icon_suffix(uint32_t icon_code)
{
    weather_type_t curweather = get_weather_type(icon_code);
    static const char *weather_icons[] = {
        "qing", // 晴
        "duoyun", // 多云、少云、晴间多云
        "yin", // 阴

        "zhenyu", // 阵雨、强阵雨
        "xiaoyu", // 小雨、毛毛雨
        "zhongyu", // 中雨
        "dayu", // 大雨
        "baoyu", // 暴雨、大暴雨、特大暴雨、极端降雨
        "leiyu", // 雷阵雨、强雷阵雨
        "leibao", // 雷阵雨伴有冰雹
        "yujiayue", // 雨夹雪、雨雪、阵雨夹雪
        "bingjing", // 冻雨

        "xiaoxue", // 小雪
        "zhongxue", // 中雪
        "daxue", // 大雪
        "baoxue", // 暴雪、大到暴雪
        "zhenxue", // 阵雪
        "xuemi", // 雪（模糊/通用）

        "qingwu", // 薄雾
        "zhongwu", // 雾
        "dawu", // 浓雾、大雾、强浓雾、特强浓雾
        "mai", // 霾（中度、重度、严重）

        "yangsha", // 扬沙
        "fuchen", // 浮尘
        "shachenbao", // 沙尘暴、强沙尘暴

        "re", // 热
        "leng", // 冷

        "qing" // 未知
    };

    if (curweather < WEATHER_TYPE_COUNT)
    {
        return (char *) weather_icons[curweather];
    }
    return "qing"; // 未知天气
}

void day_weather(lv_obj_t* parent, epd_show_weather_info_t *weather_info)
{
    lv_obj_t* cont = lv_obj_create(parent);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_size(cont, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER); // 垂直居中对齐
    lv_obj_align_to(cont, parent, LV_ALIGN_LEFT_MID, -5, 0);

    uint8_t days_flag = weather_info->day == 0 ? 0: 1; // 0: 今天, 1: 明天, 标记 weather_icon和lab数组的索引
    // 主标签
    lv_obj_t* label = lv_label_create(cont);
    lv_obj_add_style(label, &style_font, 0);
    lv_label_set_text(label, weather_info->day == 0 ? "今天" : "明天");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    
    // 子布局
    lv_obj_t* sub_layout = lv_obj_create(cont);
    lv_obj_set_flex_flow(sub_layout, LV_FLEX_FLOW_ROW); // 设置为水平排列
    lv_obj_set_size(sub_layout, LV_PCT(100), LV_SIZE_CONTENT); // 宽度铺满，高度根据内容自适应
    lv_obj_set_style_pad_all(sub_layout, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(sub_layout, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(sub_layout, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(sub_layout, LV_SCROLLBAR_MODE_OFF);

    // 设置子布局内元素的对齐方式为居中
    lv_obj_set_flex_align(sub_layout, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 图标
    rtc_lvobj.weather_icon[days_flag] = lv_img_create(sub_layout);
	char iconpath[100] = {0};
    sprintf(iconpath, DEFATLT_WEATHER_ICON_PATH, get_weather_icon_suffix(weather_info->icon_code));
    lv_img_set_src(rtc_lvobj.weather_icon[days_flag], iconpath);
    // lv_label_set_text(icon, LV_SYMBOL_HOME);
    // lv_obj_set_style_text_align(icon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT); // 确保文本居中
    lv_obj_align(rtc_lvobj.weather_icon[days_flag], LV_ALIGN_LEFT_MID, 0, 0);

    // 子子容器
    lv_obj_t* sub_sub_cont = lv_obj_create(sub_layout);
    lv_obj_set_flex_flow(sub_sub_cont, LV_FLEX_FLOW_COLUMN); // 设置为垂直排列
    lv_obj_set_size(sub_sub_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // 宽度和高度根据内容自适应
    lv_obj_set_style_pad_all(sub_sub_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(sub_sub_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(sub_sub_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(sub_sub_cont, LV_SCROLLBAR_MODE_OFF);

    char str[128] = {0};
    // 子标签
    rtc_lvobj.weather_info_lab[days_flag] = lv_label_create(sub_sub_cont);
    lv_obj_add_style(rtc_lvobj.weather_info_lab[days_flag], &style_font, 0);
    sprintf(str, "温度:\n%d-%d度", weather_info->temp_max, weather_info->temp_min);
    lv_label_set_text(rtc_lvobj.weather_info_lab[days_flag], str);
    lv_obj_set_style_text_align(rtc_lvobj.weather_info_lab[days_flag], LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT); // 确保文本居中
}

static lv_obj_t *create_weather_layout(ele_ds_t dev, lv_obj_t *up, lv_obj_t* parent, lv_obj_t* down)
{
    lv_obj_t* cont = lv_obj_create(parent);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_size(cont, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align_to(cont, parent, LV_ALIGN_TOP_LEFT, 0, 0);

    for (int i = 0; i < 2; i++) {
        lv_obj_t* weather_cont = lv_obj_create(cont);
        lv_obj_set_size(weather_cont, LV_PCT(WEATHER_LAYOUT_WIDTH_PCT), LV_SIZE_CONTENT);
        lv_obj_set_style_pad_all(weather_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(weather_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(weather_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_scrollbar_mode(weather_cont, LV_SCROLLBAR_MODE_OFF);
        epd_show_weather_info_t weather= {
            .temp_max = dev->device_cfg.weather_info[0].tempMax,
            .temp_min = dev->device_cfg.weather_info[0].tempMin,
            .icon_code = atoi(dev->device_cfg.weather_info[0].iconDay),
            .day = i,
        };
        day_weather(weather_cont, &weather);
    }
    return cont;
}

void tabview_create(ele_ds_t dev, lv_obj_t *up, lv_obj_t* parent)
{
    rtc_lvobj.tabview = lv_tabview_create(parent, LV_DIR_BOTTOM, LV_DIR_ALL);
    lv_obj_set_size(rtc_lvobj.tabview, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_pad_all(rtc_lvobj.tabview, 0, LV_PART_MAIN | LV_STATE_DEFAULT); // 去掉内边距
    lv_obj_set_style_radius(rtc_lvobj.tabview, 0, LV_PART_MAIN | LV_STATE_DEFAULT); // 去掉圆角
    lv_obj_set_scrollbar_mode(rtc_lvobj.tabview, LV_SCROLLBAR_MODE_OFF); // 禁用滚动条
    lv_obj_set_style_border_width(rtc_lvobj.tabview, 0, LV_PART_MAIN | LV_STATE_DEFAULT); // 去掉边框
    lv_obj_set_style_border_color(rtc_lvobj.tabview, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT); // 设置边框颜色
    lv_obj_align_to(rtc_lvobj.tabview, up, LV_ALIGN_OUT_BOTTOM_MID, 0, 0); // 对齐到屏幕底部
    // 获取 TabView 的按钮容器
    lv_obj_t* tab_btns = lv_tabview_get_tab_btns(rtc_lvobj.tabview);

    // 减小导航栏高度
    lv_obj_set_height(tab_btns, 25);
    lv_obj_set_align(tab_btns, LV_ALIGN_BOTTOM_MID); 
    // 创建样式：选中标签黑底白字
    static lv_style_t style_tab_selected;
    lv_style_init(&style_tab_selected);
    lv_style_set_bg_color(&style_tab_selected, lv_color_hex(0x000000));   // 黑色背景
    lv_style_set_text_color(&style_tab_selected, lv_color_hex(0xFFFFFF)); // 白色文字

    // 应用样式到选中标签
    lv_obj_add_style(tab_btns, &style_tab_selected, LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_add_state(tab_btns, LV_STATE_CHECKED);

    //本地样式修改选中时的文字颜色
    lv_obj_set_style_text_color(tab_btns,lv_color_hex(0x000000), LV_PART_ITEMS | LV_STATE_CHECKED);
	lv_obj_set_style_border_color(tab_btns, lv_color_hex(0x000000), LV_PART_ITEMS | LV_STATE_CHECKED);
    
	//本地样式修改未选中时的文字颜色
    lv_obj_set_style_text_color(tab_btns, lv_color_hex(0x000000), 0);

    // 设置字体样式
    lv_obj_add_style(rtc_lvobj.tabview, &style_font, 0);
    
    // 创建标签页
    lv_obj_t* tab1 = lv_tabview_add_tab(rtc_lvobj.tabview, "天气");
    lv_obj_t* tab2 = lv_tabview_add_tab(rtc_lvobj.tabview, "备忘录");
    lv_obj_t* tab3 = lv_tabview_add_tab(rtc_lvobj.tabview, "背景");
    lv_obj_t* tab4 = lv_tabview_add_tab(rtc_lvobj.tabview, "设置");

    // lv_tabview_set_act(weather_page_lvobj_st.tabview, 1, LV_ANIM_OFF); // 1 表示第二个标签页，LV_ANIM_OFF 表示无动画切换

    char str[50] = {0};
    lv_obj_t *_weather_layout = create_weather_layout(dev, rtc_lvobj.tabview, tab1, NULL);
    rtc_lvobj.sensor_lab = lv_label_create(tab1);
    lv_obj_add_style(rtc_lvobj.sensor_lab, &style_font, 0);
    sprintf(str, DEFAULT_SENSOR_LABFMT, dev->sensor_data.sht30[0], dev->sensor_data.sht30[1], dev->sensor_data.gzp6816d.pressure);
    lv_label_set_text(rtc_lvobj.sensor_lab, str);
    lv_obj_align_to(rtc_lvobj.sensor_lab, _weather_layout, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    rtc_lvobj.memo_lab = lv_label_create(tab2);
    lv_obj_align(rtc_lvobj.memo_lab, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_style(rtc_lvobj.memo_lab, &style_font, 0);
    lv_label_set_text(rtc_lvobj.memo_lab, g_ele_ds->device_cfg.memo);
}

void switch_tabview_cmd(const int argc,char *argv[])
{
    if (rtc_lvobj.tabview == NULL) {
        rt_kprintf("tabview is NULL\n");
        return;
    }
    if (argc < 2) {
        return;
    }
    int tab_index = atoi(argv[1]);
    if (tab_index < 0 || tab_index > 3) {
        return;
    }
    lv_tabview_set_act(rtc_lvobj.tabview, tab_index, LV_ANIM_OFF); // 切换到指定标签页
}
MSH_CMD_EXPORT_ALIAS(switch_tabview_cmd, switch_tabview, Switch tabview page);

lv_obj_t *create_tabview_layout(ele_ds_t dev, lv_obj_t *up, lv_obj_t* parent, lv_obj_t* down)
{
    // 创建底部布局容器
    lv_obj_t* bottom_layout = lv_obj_create(parent);
    lv_obj_set_size(bottom_layout, LV_PCT(100), LV_PCT(87)); // 宽度全屏，高度自适应
    lv_obj_set_style_pad_all(bottom_layout, 0, LV_PART_MAIN | LV_STATE_DEFAULT); // 去掉内边距
    lv_obj_set_style_border_width(bottom_layout, 0, LV_PART_MAIN | LV_STATE_DEFAULT); // 去掉边框
    lv_obj_set_style_border_color(bottom_layout, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT); // 设置边框颜色
    lv_obj_set_style_radius(bottom_layout, 0, LV_PART_MAIN | LV_STATE_DEFAULT); // 去掉圆角
    lv_obj_set_scrollbar_mode(bottom_layout, LV_SCROLLBAR_MODE_OFF); // 禁用滚动条
    lv_obj_align_to(bottom_layout, up, LV_ALIGN_OUT_BOTTOM_MID, 0, 0); // 对齐到屏幕底部

    // // 在底部布局中添加内容（示例）
    // lv_obj_t* label = lv_label_create(bottom_layout);
    // lv_label_set_text(label, "This is the bottom layout.");
    // lv_obj_align(label, LV_ALIGN_CENTER, 0, 0); // 居中对齐

    tabview_create(dev, up, bottom_layout);

    return bottom_layout;
}

void main_page(ele_ds_t dev)
{
    lv_obj_t* screen = lv_scr_act();
    lv_obj_t* screen_layout = lv_obj_create(screen);
    lv_obj_set_flex_flow(screen_layout, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_size(screen_layout, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_pad_all(screen_layout, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(screen_layout, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(screen_layout, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(screen_layout, LV_SCROLLBAR_MODE_OFF);


    lv_obj_t* status_bar = create_status_bar(g_ele_ds, NULL, screen_layout, NULL);
    lv_obj_t *tabview = create_tabview_layout(g_ele_ds, status_bar, screen_layout, NULL);
}
void lv_user_gui_init(void)
{
#if 1
    lv_obj_t *scr = lv_scr_act();
    // 初始化样式
    lv_style_init(&style_small);
    lv_style_set_text_font(&style_small, &lv_font_montserrat_10);
    lv_style_init(&style_bold);
    lv_style_set_text_font(&style_bold, &lv_font_montserrat_16);
    lv_style_init(&style_large);
    //lv_style_set_text_font(&style_large, &lv_font_montserrat_18);
    lv_style_init(&style_font);  
    lv_style_set_text_font(&style_font, &fangsong_8);
    lv_style_init(&style_custom_symbol_16);
    lv_style_set_text_font(&style_custom_symbol_16, &custom_symbol_16);
    main_page(g_ele_ds);
    
    // /* 1. 状态栏 */
    // update_status_bar(g_ele_ds);

    // /* 2. 天气 */
    // update_weather_info(g_ele_ds);

    // /* 2. 天气信息 */
    // lv_obj_t *weather_label = lv_label_create(scr);
    // lv_label_set_text(weather_label, "多云     温度：24℃ / 18C");
    // lv_obj_align_to(weather_label, status_bar, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 6);
    // lv_obj_add_style(weather_label, &style_font, 0);

    // /* 3. 传感器数据 - 第一行 */
    // lv_obj_t *sensor_row1 = lv_label_create(scr);
    // lv_obj_add_style(sensor_row1, &style_font, 0);
    // lv_label_set_text(sensor_row1, "温度: 23.5℃ 湿度: 45% 气压: 1012 hPa");
    // lv_obj_align_to(sensor_row1, weather_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    // /* 4. 传感器数据 - 第二行 */
    // lv_obj_t *sensor_row2 = lv_label_create(scr);
    // lv_obj_add_style(sensor_row2, &style_font, 0);
    // lv_label_set_text(sensor_row2, "TVOC: 120 ppb    CO₂: 580 ppm");
    // lv_obj_align_to(sensor_row2, sensor_row1, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 6);

    // /* 5. 底部 LOGO 或品牌字样 */
    // lv_obj_t *logo = lv_label_create(scr);
    // lv_obj_add_style(logo, &style_small, 0);
    // lv_label_set_text(logo, "ELE_DS");
    // lv_obj_align(logo, LV_ALIGN_BOTTOM_MID, 0, -2);
#else
    {
        // 创建 label
        lv_obj_t *label = lv_label_create(lv_scr_act());
        lv_style_init(&style_font);  
        lv_style_set_text_font(&style_font, &fangsong_8);
        // 设置样式
        lv_obj_add_style(label, &style_font, 0);
        
        // 设置文本
        lv_label_set_text(label, "我是杨逸辉你是刘洁琳");
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    }
#endif
}

