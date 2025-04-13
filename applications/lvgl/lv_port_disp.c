#include <lv_conf.h>
#include "lv_port_disp.h"
#include <stdbool.h>
#include "EPD_2in7_V2.h"
#include "DEV_Config.h"
#include "ele_ds.h"
#include <time.h>
#include "lvgl.h"

#ifndef MY_DISP_HOR_RES
#define MY_DISP_HOR_RES 176
#endif

#ifndef MY_DISP_VER_RES
#define MY_DISP_VER_RES 264
#endif

#define DRAW_BUF_LINES 37
#define DRAW_BUF_SIZE (MY_DISP_HOR_RES * DRAW_BUF_LINES)  // 横向一整行 × 行数
static lv_color_t buf1[DRAW_BUF_SIZE];
static lv_disp_draw_buf_t draw_buf_dsc_1;

static void disp_init(void);
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);

void lv_port_disp_init(void)
{
    disp_init();

    lv_disp_draw_buf_init(&draw_buf_dsc_1, buf1, NULL, DRAW_BUF_SIZE);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = MY_DISP_HOR_RES;
    disp_drv.ver_res = MY_DISP_VER_RES;
    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &draw_buf_dsc_1;
    disp_drv.sw_rotate = 1;
    disp_drv.rotated = LV_DISP_ROT_90;
    lv_disp_drv_register(&disp_drv);
}

static void disp_init(void)
{
    extern rt_err_t ele_ds_epaper_init(ele_ds_t ele_ds);
    rt_err_t err = ele_ds_epaper_init(g_ele_ds);
    if (err != RT_EOK)
    {
        rt_kprintf("ele_ds_epaper_init failed\n");
        return;
    }
    //LV_FONT_DECLARE(字体名);
    LV_FONT_DECLARE(hz_12_4);
    DEV_Module_Init();
    EPD_2IN7_V2_Init();
    EPD_2IN7_V2_Clear();
    EPD_2IN7_V2_Display_Base_color(WHITE);
}

static void wait_for_idle(void)
{
    while (DEV_Digital_Read(EPD_BUSY_PIN) == 1)
    {
        rt_thread_mdelay(10);
    }
}
#if 1
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;
    uint32_t buf_size = ((w + 7) / 8) * h;
    rt_uint32_t total, used, max_used;
    rt_memory_info(&total, &used, &max_used);
    // printf("Heap total: %d, used: %d, max_used: %d\n", total, used, max_used);
    // printf("x1:%d, y1:%d, x2:%d, y2:%d, w:%d, h:%d, buf_size:%d\n", area->x1, area->y1, area->x2, area->y2, w, h, buf_size);
    UBYTE *temp_buf = (UBYTE *)lv_mem_alloc(buf_size);
    if (temp_buf == NULL) 
    {
        printf("lv_mem_alloc failed\n");
        return;
    }
    EPD_2IN7_V2_Init();
    Paint_NewImage(temp_buf, w, h, 0, WHITE);
    Paint_SelectImage(temp_buf);
    Paint_Clear(WHITE);
    memset(temp_buf, 0xFF, buf_size);
    for (uint32_t y = 0; y < h; y++) {
        for (uint32_t x = 0; x < w; x++) {
            uint32_t idx = x + y * w;
            uint32_t byte_idx = (x / 8) + y * ((w + 7) / 8);
            uint8_t bit_mask = 0x80 >> (x % 8);

            if (lv_color_to1(color_p[idx]) == 0) {
                temp_buf[byte_idx] &= ~bit_mask;
            }
        }
    }
#if 0
    // 输出 temp_buf
    printf("\n");
    printf("\n");
    for (uint32_t i = 0; i < buf_size; i++) {
        printf("%02X ", temp_buf[i]);
        if ((i + 1) % ((w + 7) / 8) == 0) {
            printf("\n");
        }
    }
    printf("\n");
    printf("\n");
#endif
    EPD_2IN7_V2_Display_Partial(temp_buf, area->x1, area->y1, area->x2, area->y2);
    wait_for_idle();

    lv_mem_free(temp_buf);
    lv_disp_flush_ready(disp_drv);
}
#else
#define LINE_BUF_WIDTH_BYTE ((MY_DISP_HOR_RES + 7) / 8)
static UBYTE line_buf[LINE_BUF_WIDTH_BYTE];

static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;
    printf("x1:%d, y1:%d, x2:%d, y2:%d, w:%d, h:%d, buf_size:%d\n", area->x1, area->y1, area->x2, area->y2, w, h, 0);

    if (area->x1 < 0 || area->y1 < 0 || area->x2 >= MY_DISP_HOR_RES || area->y2 >= MY_DISP_VER_RES) {
        lv_disp_flush_ready(disp_drv);
        return;
    }

    for (uint32_t y = 0; y < h; y++) {
        memset(line_buf, 0xFF, LINE_BUF_WIDTH_BYTE);  // 默认白底

        for (uint32_t x = 0; x < w; x++) {
            uint32_t idx = x + y * w;  // color_p 是整个区域的
            int abs_x = area->x1 + x;
            int abs_y = area->y1 + y;

            if (lv_color_to1(color_p[idx]) == 0) {
                line_buf[abs_x / 8] &= ~(0x80 >> (abs_x % 8));
            }
        }

        // 刷新当前行（你也可以批量刷新多行，如果驱动支持）
        EPD_2IN7_V2_Display_Partial(line_buf, area->x1, area->y1 + y, area->x2, area->y1 + y);
        wait_for_idle();  // 如果必要
    }

    lv_disp_flush_ready(disp_drv);
}

#endif
lv_obj_t *time_label;

void update_time_label(void)
{
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);

    char time_str[9];
    strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);
    lv_label_set_text(time_label, time_str);
}

void lv_user_gui_init(void)
{
    lv_obj_t *scr = lv_scr_act();


    /* 样式 */
    static lv_style_t style_small;
    lv_style_init(&style_small);
    lv_style_set_text_font(&style_small, &lv_font_montserrat_10);

    static lv_style_t style_bold;
    lv_style_init(&style_bold);
    lv_style_set_text_font(&style_bold, &lv_font_montserrat_14);

    static lv_style_t style_large;
    lv_style_init(&style_large);
    lv_style_set_text_font(&style_large, &lv_font_montserrat_18);

    /* 1. 状态栏 */
    lv_obj_t *status_bar = lv_label_create(scr);
    lv_obj_add_style(status_bar, &style_small, 0);
    lv_label_set_text(status_bar, "Bat:85%  Sig:▂▄▆  14:20  2025-04-13 (Sun)");

    lv_obj_align(status_bar, LV_ALIGN_TOP_LEFT, 4, 2);

    /* 2. 天气信息 */
    lv_obj_t *weather_label = lv_label_create(scr);
    lv_obj_add_style(weather_label, &style_large, 0);
    lv_label_set_text(weather_label, "多云     温度：24C / 18C");
    lv_obj_align_to(weather_label, status_bar, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 6);
    lv_obj_set_style_text_font(weather_label, &hz_12_4, LV_PART_MAIN);

    /* 3. 传感器数据 - 第一行 */
    lv_obj_t *sensor_row1 = lv_label_create(scr);
    lv_obj_add_style(sensor_row1, &style_bold, 0);
    lv_label_set_text(sensor_row1, "温度: 23.5℃   湿度: 45%   气压: 1012 hPa");
    lv_obj_align_to(sensor_row1, weather_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    /* 4. 传感器数据 - 第二行 */
    lv_obj_t *sensor_row2 = lv_label_create(scr);
    lv_obj_add_style(sensor_row2, &style_bold, 0);
    lv_label_set_text(sensor_row2, "TVOC: 120 ppb    CO₂: 580 ppm");
    lv_obj_align_to(sensor_row2, sensor_row1, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 6);

    /* 5. 底部 LOGO 或品牌字样 */
    lv_obj_t *logo = lv_label_create(scr);
    lv_obj_add_style(logo, &style_small, 0);
    lv_label_set_text(logo, "ELE_DS");
    lv_obj_align(logo, LV_ALIGN_BOTTOM_MID, 0, -2);
}

void disp_enable_update(void) {}
void disp_disable_update(void) {}
