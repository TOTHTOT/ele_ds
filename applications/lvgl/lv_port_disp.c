#include <lv_conf.h>
#include "lv_port_disp.h"
#include <stdbool.h>
#include "EPD_2in7_V2.h"
#include "DEV_Config.h"
#include "ele_ds.h"
#include <time.h>
#include "lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <dfs_fs.h>
#include <stdint.h>

#ifndef MY_DISP_HOR_RES
#define MY_DISP_HOR_RES 176
#endif

#ifndef MY_DISP_VER_RES
#define MY_DISP_VER_RES 264
#endif

#define DRAW_BUF_LINES MY_DISP_VER_RES
#define DRAW_BUF_SIZE (MY_DISP_HOR_RES * DRAW_BUF_LINES)  // 横向一整行 × 行数
CCMRAM static lv_color_t buf1[DRAW_BUF_SIZE];
static lv_disp_draw_buf_t draw_buf_dsc_1;

static void disp_init(void);
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);

void lv_port_disp_init(void)
{
    disp_init();

    lv_disp_draw_buf_init(&draw_buf_dsc_1, buf1, NULL, DRAW_BUF_SIZE);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 264;
    disp_drv.ver_res = 176;
    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &draw_buf_dsc_1;
    // disp_drv.sw_rotate = 1;
    // disp_drv.rotated = LV_DISP_ROT_90;
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
    // LV_FONT_DECLARE(hz_12_4);
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

void debug_print_color_p(const lv_color_t *color_p, uint32_t w, uint32_t h)
{
    for (uint32_t y = 0; y < h; y++)
    {
        for (uint32_t x = 0; x < w; x++)
        {
            uint32_t idx = x + y * w;
            // lv_color_to1() 会自动处理色深
            if (lv_color_to1(color_p[idx]) == 0)
                printf("█");  // 黑色像素
            else
                printf(" ");  // 白色像素
        }
        printf("\n");
    }
}


void debug_print_1bit_buffer(const uint8_t *buf, uint32_t w, uint32_t h) {
    uint32_t bytes_per_row = (w + 7) / 8;

    for (uint32_t y = 0; y < h; y++) {
        for (uint32_t x = 0; x < w; x++) {
            uint32_t byte_idx = y * bytes_per_row + x / 8;
            uint8_t bit_mask = 0x80 >> (x % 8);
            uint8_t bit = buf[byte_idx] & bit_mask;

            if (bit == 0)
                printf("█");
            else
                printf(" ");
        }
        printf("\n");
    }
}

void cmd_clean_screen(void)
{
    EPD_2IN7_V2_Init();
    EPD_2IN7_V2_Clear();
}
MSH_CMD_EXPORT_ALIAS(cmd_clean_screen, clscr, clean screen);
#if 1
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;
    uint32_t buf_size = ((w + 7) / 8) * h;
    rt_uint32_t total, used, max_used;
    rt_memory_info(&total, &used, &max_used);
    // printf("Heap total: %d, used: %d, max_used: %d\n", total, used, max_used);
    printf("x1:%d, y1:%d, x2:%d, y2:%d, w:%d, h:%d, buf_size:%d\n", area->x1, area->y1, area->x2, area->y2, w, h, buf_size);
    UBYTE *temp_buf = (UBYTE *)lv_mem_alloc(buf_size);
    if (temp_buf == NULL) 
    {
        printf("lv_mem_alloc failed\n");
        return;
    }
    EPD_2IN7_V2_Init();
    Paint_NewImage(temp_buf, h, w, 270, WHITE);
    Paint_SelectImage(temp_buf);
    Paint_Clear(WHITE);
    memset(temp_buf, 0xFF, buf_size);
    debug_print_color_p(color_p, w, h);
#if 1
    // uint32_t tmp = w;
    // w = h;
    // h = tmp;
    for (uint32_t y = 0; y < h; y++)
    {
        for (uint32_t x = 0; x < w; x++)
        {
            uint32_t idx = x + y * w;
            uint32_t byte_idx = (x / 8) + y * ((w + 7) / 8);
            uint8_t bit_mask = 0x80 >> (x % 8);

            if (lv_color_to1(color_p[idx]) == 0)
            {
                Paint_SetPixel(x, y, BLACK);
            }
            else
            {
                Paint_SetPixel(x, y, WHITE);
            }
        }
    }
#else
    Paint_DrawNum(10, 33, 12, &Font12, BLACK, WHITE);
    Paint_DrawNum(210, 33, 34, &Font12, BLACK, WHITE);
    Paint_DrawNum(210, 133, 56, &Font12, BLACK, WHITE);
#endif
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
    debug_print_1bit_buffer(temp_buf, w, h);
    EPD_2IN7_V2_Display_Partial(temp_buf, area->y1, area->x1, area->y2, area->x2);
    wait_for_idle();

    lv_mem_free(temp_buf);
    lv_disp_flush_ready(disp_drv);
}
#else
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;
    uint32_t buf_size = ((h + 7) / 8) * w; // 注意：因为我们旋转了图像，X<->Y
    printf("x1:%d, y1:%d, x2:%d, y2:%d, w:%d, h:%d, buf_size:%d\n", area->x1, area->y1, area->x2, area->y2, w, h, buf_size);

    UBYTE *temp_buf = (UBYTE *)lv_mem_alloc(buf_size);
    if (temp_buf == NULL) 
    {
        printf("lv_mem_alloc failed\n");
        return;
    }

    memset(temp_buf, 0xFF, buf_size); // 默认全白

    // 屏幕显示为顺时针旋转了90度 -> 我们写入 temp_buf 时需逆时针旋转
    for (uint32_t y = 0; y < h; y++) {
        for (uint32_t x = 0; x < w; x++) {
            // 原图像坐标 (x, y)
            uint32_t src_idx = x + y * w;

            // 顺时针旋转90度后的坐标：(new_x = y, new_y = w - 1 - x)
            uint32_t new_x = y;
            uint32_t new_y = w - 1 - x;

            uint32_t byte_idx = (new_x / 8) + new_y * ((h + 7) / 8); // 因为现在 width = h
            uint8_t bit_mask = 0x80 >> (new_x % 8);

            if (lv_color_to1(color_p[src_idx]) == 0) {
                temp_buf[byte_idx] &= ~bit_mask;  // 黑色像素
            }
        }
    }

    // 显示旋转后图像（如果EPD使用AM=1 Y方向递增）
    EPD_2IN7_V2_Display_Partial(temp_buf, area->x1, area->y1, area->x2, area->y2);
    wait_for_idle();

    lv_mem_free(temp_buf);
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


void disp_enable_update(void) {}
void disp_disable_update(void) {}
