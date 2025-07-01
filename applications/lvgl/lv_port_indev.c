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
#include <time.h>
#include "custom_symbol_16.h"

struct ele_ds_uistyle
{
    lv_style_t time; // 时间样式
    lv_style_t date; // 日期样式
    lv_style_t default_chinese; // 中文字体样式
    lv_style_t custom_symbol_16; // 自定义图标字体样式
};
typedef struct ele_ds_uistyle *ele_ds_uistyle_t;

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
    lv_obj_t *date_lab; // 日期标签
    lv_obj_t *message; // 消息图标
};

struct ele_ds_ui
{
    struct ele_ds_uistyle style;
    struct real_time_change_lvobj rtc_lvobj;
    lv_obj_t *screen;
};
typedef struct ele_ds_ui *ele_ds_ui_t;

static struct ele_ds_ui ele_ds_ui = {0};

void lv_port_indev_init(void)
{

}

#define STATUS_BAR_WIDTH_PCT 15
#define STATUS_BAR_CENTER_WIDTH_PCT 70
#define WEATHER_LAYOUT_WIDTH_PCT 50
#define TIME_LABFMT "%H:%M"
#define DATE_LABFMT "%Y-%m-%d"

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

/**
 * @brief 切换导航栏页面
 * @param argc 参数个数
 * @param argv 参数, 一般是切换的页面
 */
void switch_tabview_cmd(const int argc,char *argv[])
{
    if (ele_ds_ui.rtc_lvobj.tabview == NULL) {
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
    lv_tabview_set_act(ele_ds_ui.rtc_lvobj.tabview, tab_index, LV_ANIM_OFF); // 切换到指定标签页
    lv_obj_invalidate(lv_scr_act());
}
MSH_CMD_EXPORT_ALIAS(switch_tabview_cmd, switch_tabview, Switch tabview page);

void tabview_create(ele_ds_ui_t ui, ele_ds_t dev, lv_obj_t *up, lv_obj_t* parent)
{
    ui->rtc_lvobj.tabview = lv_tabview_create(parent, LV_DIR_BOTTOM, LV_DIR_ALL);
    lv_obj_set_size(ui->rtc_lvobj.tabview, LV_PCT(65), LV_PCT(100));
    lv_obj_set_style_pad_all(ui->rtc_lvobj.tabview, 0, LV_PART_MAIN | LV_STATE_DEFAULT); // 去掉内边距
    lv_obj_set_style_radius(ui->rtc_lvobj.tabview, 0, LV_PART_MAIN | LV_STATE_DEFAULT); // 去掉圆角
    lv_obj_set_scrollbar_mode(ui->rtc_lvobj.tabview, LV_SCROLLBAR_MODE_OFF); // 禁用滚动条
    lv_obj_set_style_border_width(ui->rtc_lvobj.tabview, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(ui->rtc_lvobj.tabview, lv_color_black(), LV_PART_MAIN);

    // 获取 TabView 的按钮容器
    lv_obj_t* tab_btns = lv_tabview_get_tab_btns(ui->rtc_lvobj.tabview);

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
    lv_obj_add_style(ui->rtc_lvobj.tabview, &ui->style.default_chinese, 0);

    // 创建标签页
    lv_obj_t* tab1 = lv_tabview_add_tab(ui->rtc_lvobj.tabview, "天气");
    lv_obj_t* tab2 = lv_tabview_add_tab(ui->rtc_lvobj.tabview, "备忘录");
    lv_obj_t* tab3 = lv_tabview_add_tab(ui->rtc_lvobj.tabview, "背景");
    // lv_obj_t* tab4 = lv_tabview_add_tab(ui->rtc_lvobj.tabview, "设置");

    // lv_tabview_set_act(weather_page_lvobj_st.tabview, 1, LV_ANIM_OFF); // 1 表示第二个标签页，LV_ANIM_OFF 表示无动画切换
}

static void update_rtc_labobj_cb(lv_timer_t * timer)
{
    time_t curtime = time(NULL);
    struct tm *tm_info = localtime(&curtime);
    char str[100] = {0};
    ele_ds_ui_t ui = (ele_ds_ui_t)timer->user_data;

    if (ui->rtc_lvobj.date_lab != NULL)
    {
        strftime(str, sizeof(str), DATE_LABFMT, tm_info);
        lv_label_set_text(ui->rtc_lvobj.date_lab, str);
    }
    if (ui->rtc_lvobj.time_lab != NULL)
    {
        strftime(str, sizeof(str), TIME_LABFMT, tm_info);
        lv_label_set_text(ui->rtc_lvobj.time_lab, str);
    }

    // 标记脏屏幕 导致全刷 不然会出现控件错位问题
    lv_obj_invalidate(lv_scr_act());
}

static lv_obj_t *create_devinfo_layout(ele_ds_ui_t ui, ele_ds_t dev, lv_obj_t *up, lv_obj_t* parent)
{
    lv_obj_t* devinfo_layout = lv_obj_create(parent);
    lv_obj_set_flex_flow(devinfo_layout, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_size(devinfo_layout, LV_PCT(35), LV_PCT(100));
    lv_obj_set_style_pad_all(devinfo_layout, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(devinfo_layout, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(devinfo_layout, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_radius(devinfo_layout, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(devinfo_layout, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_align(devinfo_layout, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    time_t curtime = time(NULL);
    struct tm *tm_info = localtime(&curtime);
    char str[100] = {0};

    ui->rtc_lvobj.date_lab = lv_label_create(devinfo_layout);
    lv_obj_add_style(ui->rtc_lvobj.date_lab, &ui->style.date, 0);
    strftime(str, sizeof(str), DATE_LABFMT, tm_info);
    lv_label_set_text(ui->rtc_lvobj.date_lab, str);

    ui->rtc_lvobj.time_lab = lv_label_create(devinfo_layout);
    lv_obj_add_style(ui->rtc_lvobj.time_lab, &ui->style.time, 0);
    strftime(str, sizeof(str), TIME_LABFMT, tm_info);
    lv_label_set_text(ui->rtc_lvobj.time_lab, str);

    lv_timer_create(update_rtc_labobj_cb, 20 * 1000, ui);
    return devinfo_layout;
}

void main_page(ele_ds_ui_t ui, ele_ds_t dev)
{
    lv_obj_t* screen_layout = lv_obj_create(ui->screen);
    lv_obj_set_flex_flow(screen_layout, LV_FLEX_FLOW_ROW);
    lv_obj_set_size(screen_layout, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_pad_all(screen_layout, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_row(screen_layout, 0, LV_PART_MAIN);      // 行间距为 0
    lv_obj_set_style_pad_column(screen_layout, 0, LV_PART_MAIN);   // 列间距为 0
    lv_obj_set_style_border_width(screen_layout, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(screen_layout, 0, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(screen_layout, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_align(screen_layout,
                          LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    create_devinfo_layout(ui, dev, NULL, screen_layout);
    tabview_create(ui, dev, NULL, screen_layout);
}

/**
 * @brief 初始化使用到的样式
 * @param style 样式
 */
static void style_init(ele_ds_uistyle_t style)
{
    lv_style_init(&style->date);
    lv_style_set_text_font(&style->date, &lv_font_montserrat_10);

    lv_style_init(&style->time);
    lv_style_set_text_font(&style->time, &lv_font_montserrat_24);

    lv_style_init(&style->default_chinese);
    lv_style_set_text_font(&style->default_chinese, &fangsong_8);

    lv_style_init(&style->custom_symbol_16);
    lv_style_set_text_font(&style->custom_symbol_16, &custom_symbol_16);
}

void lv_user_gui_init(void)
{
    ele_ds_ui.screen = lv_scr_act();

    style_init(&ele_ds_ui.style);

    main_page(&ele_ds_ui, g_ele_ds);
}

