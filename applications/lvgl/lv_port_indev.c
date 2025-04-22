/*
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

void lv_port_indev_init(void)
{

}

// 设置字体
extern lv_font_t fangsong_8;

/* 样式 */
static lv_style_t style_small;
static lv_style_t style_bold;
static lv_style_t style_large;
static lv_style_t style_font;

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
        lv_label_set_text(wifi, LV_SYMBOL_WIFI);
    }
    else
    {
        lv_label_set_text(wifi, LV_SYMBOL_WARNING);
    }
}
void update_status_bar(ele_ds_t dev)
{
    // 状态栏布局
    lv_obj_t* cont = lv_obj_create(lv_scr_act());
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_size(cont, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT); // 去掉容器的内边距
    lv_obj_set_style_radius(cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT); // 去掉圆角
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF); // 禁用滚动条
    lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT); // 去掉边框


    lv_obj_t* sub_cont[3];
    for (int i = 0; i < 3; i++) {
        sub_cont[i] = lv_obj_create(cont);
        lv_obj_set_size(sub_cont[i], LV_PCT(15), LV_SIZE_CONTENT); // 每个子容器宽度为 33%
        lv_obj_set_style_bg_color(sub_cont[i], lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT); // 设置背景为白色
        lv_obj_set_style_border_width(sub_cont[i], 0, LV_PART_MAIN | LV_STATE_DEFAULT); // 去掉边框
        lv_obj_set_style_pad_all(sub_cont[i], 0, LV_PART_MAIN | LV_STATE_DEFAULT); // 去掉子容器的内边距
        lv_obj_set_style_radius(sub_cont[i], 0, LV_PART_MAIN | LV_STATE_DEFAULT); // 去掉圆角
    }
    lv_obj_set_size(sub_cont[1], LV_PCT(70), LV_SIZE_CONTENT);

    // 电量初始化
    lv_obj_t *vbat = lv_label_create(sub_cont[0]);
    lv_obj_add_style(vbat, &style_bold, 0);
    uint8_t vbat_percent = (dev->device_status.current_vbat - MIN_VBAT) / (MAX_VBAT - MIN_VBAT) * 100;
    set_vbat_icon(vbat, vbat_percent);
    lv_obj_align(vbat, LV_ALIGN_LEFT_MID, 3, 0);
    
    // wifi 信号初始化
    lv_obj_t *wifi = lv_label_create(sub_cont[0]);
    lv_obj_add_style(wifi, &style_bold, 0);
    set_wifi_icon(wifi, dev->device_status.cnt_wifi);
    lv_label_set_text(wifi, LV_SYMBOL_WIFI);
    lv_obj_align_to(wifi, vbat, LV_ALIGN_OUT_RIGHT_MID, 4, 0);

    // 时间初始化
    lv_obj_t *time_label = lv_label_create(sub_cont[1]);
    time_t curtime = time(NULL);
    struct tm *tm_info = localtime(&curtime);
    char time_str[32] = {0};
    strftime(time_str, sizeof(time_str), "%Y-%m-%d ( %a ) %H:%M", tm_info);
    lv_obj_add_style(time_label, &style_bold, 0);
    lv_label_set_text(time_label, time_str);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 0);

    // 新消息初始化
    lv_obj_t *newmessage = lv_label_create(sub_cont[2]);
    lv_obj_add_style(newmessage, &style_bold, 0);
    lv_label_set_text(newmessage, LV_SYMBOL_NEW_LINE);
    lv_obj_align(newmessage, LV_ALIGN_CENTER, -3, 0);
}

/**
 * @description: 根据当前天气信息返回对应后缀
 * @param {ele_ds_t} dev
 * @return {*}
 */
static char *get_weather_icon_suffix(ele_ds_t dev)
{
    static const char *weather_icons[] = {
        "baoxue", "baoyu", "bingbao", "bingjing", "bingli", "daxue", "dayu", "dawu",
        "duoyun", "duoyun_ye", "fuchen", "leibao", "leiyu", "mai", "qing", "qing_ye",
        "qingwu", "shachenbao", "xiaoxue", "xiaoyu", "xuemi", "yangsha", "yin",
        "yujiayue", "zhenyu", "zhongwu", "zhongxue", "zhongyu"};

    if (dev->device_cfg.curweather < WEATHER_TYPE_COUNT)
    {
        return (char *)weather_icons[dev->device_cfg.curweather];
    }
    return "unknown"; // 未知天气
}

void update_weather_info(ele_ds_t dev)
{
    lv_obj_t * img = lv_img_create(lv_scr_act());
	char iconpath[100] = {0};
    sprintf(iconpath, "S:/sysfile/icon/tianqi_48/tianqi-%s.bin", get_weather_icon_suffix(dev));
    lv_img_set_src(img, iconpath);

    // 将图像对象居中显示
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
}


void lv_user_gui_init(void)
{
#if 1
    lv_obj_t *scr = lv_scr_act();
    // 初始化样式
    lv_style_init(&style_small);
    lv_style_set_text_font(&style_small, &lv_font_montserrat_10);
    lv_style_init(&style_bold);
    lv_style_set_text_font(&style_bold, &lv_font_montserrat_12);
    lv_style_init(&style_large);
    //lv_style_set_text_font(&style_large, &lv_font_montserrat_18);
    lv_style_init(&style_font);  
    lv_style_set_text_font(&style_font, &fangsong_8);

    /* 1. 状态栏 */
    update_status_bar(g_ele_ds);

    /* 2. 天气 */
    update_weather_info(g_ele_ds);

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

