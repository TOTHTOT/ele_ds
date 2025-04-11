#include <lv_conf.h>
#include "lv_port_disp.h"
#include <stdbool.h>
#include "EPD_2in7_V2.h"
#include "DEV_Config.h"
#include "ele_ds.h"
#include <time.h>
#include "lvgl.h"

#ifndef MY_DISP_HOR_RES
#define MY_DISP_HOR_RES 264
#endif

#ifndef MY_DISP_VER_RES
#define MY_DISP_VER_RES 176
#endif
#define HORBUF_SIZE (MY_DISP_HOR_RES * MY_DISP_VER_RES / 2)
static lv_disp_draw_buf_t draw_buf_dsc_1;
static lv_color_t buf1[HORBUF_SIZE]; // 10 行缓冲区

static void disp_init(void);
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);

void lv_port_disp_init(void)
{
    disp_init();

    lv_disp_draw_buf_init(&draw_buf_dsc_1, buf1, NULL, HORBUF_SIZE);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = MY_DISP_HOR_RES;
    disp_drv.ver_res = MY_DISP_VER_RES;
    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &draw_buf_dsc_1;
    disp_drv.sw_rotate = 1;   // add for rotation
    // disp_drv.rotated = LV_DISP_ROT_90; // add for rotation
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

    DEV_Module_Init();
    EPD_2IN7_V2_Init();
    EPD_2IN7_V2_Clear();
}

static void wait_for_idle(void)
{
    while (DEV_Digital_Read(EPD_BUSY_PIN) == 1)
    {
        rt_thread_mdelay(10);
    }
}
#if 1
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;
    uint32_t buf_size = ((w + 7) / 8) * h;
    UBYTE *temp_buf = (UBYTE *)lv_mem_alloc(buf_size);
    if (temp_buf == NULL) return;
    EPD_2IN7_V2_Init();

    Paint_NewImage(temp_buf, w, h, 0, WHITE);
    printf("Drawing, w = %d, h = %d, size = %d\r\n", w, h, buf_size);
    printf("Partial refresh\r\n");
    Paint_SelectImage(temp_buf);
    Paint_SetScale(2);
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
    // EPD_2IN7_V2_Display(temp_buf);
    wait_for_idle();

    lv_mem_free(temp_buf);
    lv_disp_flush_ready(disp_drv);
}
#else
static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;

    // 旋转后，宽高互换
    uint32_t rw = w;
    uint32_t rh = h;

    uint32_t buf_size = ((rw + 7) / 8) * rh;
    UBYTE *temp_buf = (UBYTE *)lv_mem_alloc(buf_size);
    if (temp_buf == NULL)
        return;
    rt_kprintf("x1 = %d, y1 = %d, x2 = %d, y2 = %d\n", area->x1, area->y1, area->x2, area->y2);
    rt_kprintf("rw = %d, rh = %d, buf_size = %d\n", rw, rh, buf_size);
    EPD_2IN7_V2_Init();
    Paint_NewImage(temp_buf, rw, rh, 0, WHITE);
    Paint_SelectImage(temp_buf);
    // Paint_SetScale(2);
    Paint_Clear(WHITE);
    memset(temp_buf, 0xFF, buf_size); // 默认全白

    for (uint32_t y = 0; y < h; y++)
    {
        for (uint32_t x = 0; x < w; x++)
        {
            uint32_t idx = x + y * w;

            // 旋转90度后坐标：(x, y) → (y, w - 1 - x)
            uint32_t new_x = y;
            uint32_t new_y = w - 1 - x;

            uint32_t byte_idx = (new_x / 8) + new_y * ((rw + 7) / 8);
            uint8_t bit_mask = 0x80 >> (new_x % 8);

            if (lv_color_to1(color_p[idx]) == 0)
            {
                temp_buf[byte_idx] &= ~bit_mask;
            }
        }
    }
#if 1
    // 输出验证
    printf("Drawing rotated, w = %d, h = %d, size = %d\n", rw, rh, buf_size);
    printf("Partial refresh\n");
    for (uint32_t i = 0; i < buf_size; i++) {
        printf("%02X ", temp_buf[i]);
        if ((i + 1) % ((rw + 7) / 8) == 0) {
            printf("\n");
        }
    }
#endif

    // 旋转后的实际坐标也要处理：
    uint32_t new_x1 = area->y1;
    uint32_t new_y1 = MY_DISP_HOR_RES - 1 - area->x2;
    uint32_t new_x2 = area->y2;
    uint32_t new_y2 = MY_DISP_HOR_RES - 1 - area->x1;

    EPD_2IN7_V2_Display_Partial(temp_buf, new_x1, new_y1, new_x2 - 1, new_y2 - 1);
    wait_for_idle();

    lv_mem_free(temp_buf);
    lv_disp_flush_ready(disp_drv);
}
#endif
lv_obj_t *time_label;  // 声明一个全局标签，用来显示时间

void update_time_label(void)
{
    // 获取当前时间
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);

    // 将时间格式化为字符串
    char time_str[9];  // 格式为 HH:MM:SS
    strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);

    // 更新时间标签的文本
    lv_label_set_text(time_label, time_str);
}

void lv_user_gui_init(void)
{
    printf("lv_user_gui_init\n");

    lv_obj_t *scr = lv_scr_act();

    // 创建一个标签
    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "tr");
    lv_obj_align(label, LV_ALIGN_TOP_RIGHT, 0, 0);

    lv_obj_t *label2 = lv_label_create(scr);
    lv_label_set_text(label2, "tf");
    lv_obj_align(label2, LV_ALIGN_TOP_LEFT, 0, 0);
    
    // lv_obj_t *label3 = lv_label_create(scr);
    // lv_label_set_text(label3, "tm");
    // lv_obj_align(label3, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *label4 = lv_label_create(scr);
    lv_label_set_text(label, "br");
    lv_obj_align(label4, LV_ALIGN_BOTTOM_RIGHT, 0, 0);

    lv_obj_t *label5 = lv_label_create(scr);
    lv_label_set_text(label5, "bf");
    lv_obj_align(label5, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    // lv_obj_t *label6 = lv_label_create(scr);
    // lv_label_set_text(label6, "bm");
    // lv_obj_align(label6, LV_ALIGN_BOTTOM_MID, 0, 0);
    
    // lv_obj_t *label7 = lv_label_create(scr);
    // lv_label_set_text(label7, "center");
    // lv_obj_align(label7, LV_ALIGN_CENTER, 0, 0);


    // 创建一个时间标签
    // time_label = lv_label_create(scr);
    // //lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 0);  // 将时间标签放在屏幕底部

    // // 每秒钟更新一次时间
	// lv_timer_create(update_time_label, 5000, NULL);
    //lv_task_create((lv_task_cb_t)update_time_label, 1000, LV_TASK_PRIO_LOW, NULL);

    printf("lv_user_gui_init end\n");
}


void disp_enable_update(void)
{
    // 可用于控制刷新开关
}

void disp_disable_update(void)
{
    // 可用于控制刷新开关
}
