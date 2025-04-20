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
    // 获取当前活动的屏幕对象
{
    lv_obj_t *scr = lv_scr_act();

    // 电量初始化
    lv_obj_t *vbat = lv_label_create(scr);
    lv_obj_add_style(vbat, &style_bold, 0);
    uint8_t vbat_percent = (dev->device_status.current_vbat - MIN_VBAT) / (MAX_VBAT - MIN_VBAT) * 100;
    set_vbat_icon(vbat, vbat_percent);
    lv_obj_align(vbat, LV_ALIGN_TOP_LEFT, 4, 2);

    // wifi 信号初始化
    lv_obj_t *wifi = lv_label_create(scr);
    lv_obj_add_style(wifi, &style_bold, 0);
    set_wifi_icon(wifi, dev->device_status.cnt_wifi);
    lv_label_set_text(wifi, LV_SYMBOL_WIFI);
    lv_obj_align_to(wifi, vbat, LV_ALIGN_OUT_RIGHT_MID, 4, 0);

    // 时间初始化
    lv_obj_t *time_label = lv_label_create(scr);
    time_t curtime = time(NULL);
    struct tm *tm_info = localtime(&curtime);
    char time_str[32] = {0};
    strftime(time_str, sizeof(time_str), "%Y-%m-%d ( %a ) %H:%M", tm_info);
    lv_obj_add_style(time_label, &style_bold, 0);
    lv_label_set_text(time_label, time_str);
    lv_obj_align_to(time_label, scr, LV_ALIGN_TOP_MID, 0, 0);
}

void update_weather_info(ele_ds_t dev)
{
    // 使用 open 函数打开文件
    int fd = open("/sysfile/icon/tianqi_48/tianqi-baoxue.bin", O_RDONLY);
    if (fd == -1) {
        printf("Failed to open file: %s\n", strerror(errno));
        return;
    }
    else {
        printf("File opened successfully\n");
    }

     static uint8_t my_img_data[48 * 48] = {0xff};
    read(fd, my_img_data, sizeof(my_img_data));
    close(fd);
    
    static lv_img_dsc_t my_img_dsc = {
        .header.always_zero = 0,
        .header.w = 48,
        .header.h = 48,
        .data_size = 48 * 48 * LV_COLOR_DEPTH / 8,
        .header.cf = LV_IMG_CF_RGB888, /*Set the color format*/
        .data = my_img_data,
    };
    
    // 创建一个图像对象
    lv_obj_t * img = lv_img_create(lv_scr_act());

    // 设置图像文件路径
    lv_img_set_src(img, &my_img_dsc);

    // 将图像对象居中显示
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);


    // 调用 LVGL 任务处理函数以刷新显示
    // lv_task_handler();
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

