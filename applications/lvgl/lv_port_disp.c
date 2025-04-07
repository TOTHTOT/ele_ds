/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-04-07 14:15:53
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-04-07 17:32:31
 * @FilePath: \ele_ds\applications\lvgl\lv_port_disp.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/**
 * @file lv_port_disp_templ.c
 *
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include <lv_conf.h>
#include "lv_port_disp.h"
#include <stdbool.h>
#include "EPD_2in7_V2.h"
#include "DEV_Config.h"
#include "ele_ds.h"
/*********************
 *      DEFINES
 *********************/
#ifndef MY_DISP_HOR_RES
    #warning Please define or replace the macro MY_DISP_HOR_RES with the actual screen width, default value 320 is used for now.
    #define MY_DISP_HOR_RES    240
#endif

#ifndef MY_DISP_VER_RES
    #warning Please define or replace the macro MY_DISP_HOR_RES with the actual screen height, default value 240 is used for now.
    #define MY_DISP_VER_RES    240
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//        const lv_area_t * fill_area, lv_color_t color);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/

    /**
     * LVGL requires a buffer where it internally draws the widgets.
     * Later this buffer will passed to your display driver's `flush_cb` to copy its content to your display.
     * The buffer has to be greater than 1 display row
     *
     * There are 3 buffering configurations:
     * 1. Create ONE buffer:
     *      LVGL will draw the display's content here and writes it to your display
     *
     * 2. Create TWO buffer:
     *      LVGL will draw the display's content to a buffer and writes it your display.
     *      You should use DMA to write the buffer's content to the display.
     *      It will enable LVGL to draw the next part of the screen to the other buffer while
     *      the data is being sent form the first buffer. It makes rendering and flushing parallel.
     *
     * 3. Double buffering
     *      Set 2 screens sized buffers and set disp_drv.full_refresh = 1.
     *      This way LVGL will always provide the whole rendered screen in `flush_cb`
     *      and you only need to change the frame buffer's address.
     */

    /* Example for 1) */
    static lv_disp_draw_buf_t draw_buf_dsc_1;

    /*GCC*/
#if defined ( __GNUC__ )
    static lv_color_t buf_1[MY_DISP_HOR_RES * MY_DISP_HOR_RES / 2] __attribute__((section(".LVGLccm")));                          /*A buffer for 10 rows*/
    /*MDK*/
#elif defined ( __CC_ARM )
    __attribute__((at(0x10000000))) lv_color_t buf_1[MY_DISP_HOR_RES * MY_DISP_VER_RES / 2];
#endif

    lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, MY_DISP_HOR_RES * MY_DISP_VER_RES / 2);   /*Initialize the display buffer*/
    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    static lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = MY_DISP_HOR_RES;
    disp_drv.ver_res = MY_DISP_VER_RES;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;

    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dsc_1;

    /*Required for Example 3)*/
    //disp_drv.full_refresh = 1;

    /* Fill a memory array with a color if you have GPU.
     * Note that, in lv_conf.h you can enable GPUs that has built-in support in LVGL.
     * But if you have a different GPU you can use with this callback.*/
    //disp_drv.gpu_fill_cb = gpu_fill;

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
    extern rt_err_t ele_ds_epaper_init(ele_ds_t ele_ds);
    rt_err_t err = ele_ds_epaper_init(g_ele_ds);
    if (err != RT_EOK)
    {
        rt_kprintf("ele_ds_epaper_init failed\n");
        return;
    }

    // 初始化硬件
    DEV_Module_Init();
    EPD_2IN7_V2_Init();
    // EPD_2IN7_V2_Clear();
}
void lv_user_gui_init(void) {
    // 1. 创建一个屏幕
    lv_obj_t *scr = lv_scr_act();  // 获取当前活动屏幕

    // 2. 添加一个标签
    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "Hello, RT-Thread + LVGL!");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -20);

    // // 3. 添加一个按钮
    // lv_obj_t *btn = lv_btn_create(scr);
    // lv_obj_align(btn, LV_ALIGN_CENTER, 0, 20);

    // lv_obj_t *btn_label = lv_label_create(btn);
    // lv_label_set_text(btn_label, "Click Me");

    // 4. 注册按钮事件回调
    //lv_obj_add_event_cb(btn, btn_event_handler, LV_EVENT_CLICKED, NULL);
}
volatile bool disp_flush_enabled = true;

/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_enable_update(void)
{
    disp_flush_enabled = true;
}

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void)
{
    disp_flush_enabled = false;
}
/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[EPD_2IN7_V2_WIDTH * 10]; // 10行缓冲区

// 等待墨水屏空闲
static void wait_for_idle(void)
{
    while(DEV_Digital_Read(EPD_BUSY_PIN) == 1) {
        rt_thread_mdelay(10);
    }
}

// 显示刷新回调函数
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    // 将LVGL的1-bit颜色数据转换为墨水屏需要的格式
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;
    uint32_t buf_size = ((w + 7) / 8) * h;
    
    // 分配临时缓冲区
    UBYTE *temp_buf = (UBYTE *)lv_mem_alloc(buf_size);
    if(temp_buf == NULL) return;
    
    // 转换颜色数据
    for(uint32_t y = 0; y < h; y++) {
        for(uint32_t x = 0; x < w; x++) {
            uint32_t idx = x + y * w;
            uint32_t byte_idx = (x / 8) + y * ((w + 7) / 8);
            uint8_t bit_mask = 0x80 >> (x % 8);
            
            if(lv_color_to1(color_p[idx]) == 0) { // 黑色
                temp_buf[byte_idx] &= ~bit_mask;
            } else { // 白色
                temp_buf[byte_idx] |= bit_mask;
            }
        }
    }
    LV_LOG_INFO("Display flushing: x1=%d, y1=%d, x2=%d, y2=%d, buf_size=%d", area->x1, area->y1, area->x2, area->y2, buf_size);
    // 部分刷新
    // EPD_2IN7_V2_Display_Partial(temp_buf, area->x1, area->y1, area->x2, area->y2);
    
    // 释放内存
    lv_mem_free(temp_buf);
    
    // 通知LVGL刷新完成
    lv_disp_flush_ready(disp_drv);
}
/*OPTIONAL: GPU INTERFACE*/

/*If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color*/
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//                    const lv_area_t * fill_area, lv_color_t color)
//{
//    /*It's an example code which should be done by your GPU*/
//    int32_t x, y;
//    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/
//
//    for(y = fill_area->y1; y <= fill_area->y2; y++) {
//        for(x = fill_area->x1; x <= fill_area->x2; x++) {
//            dest_buf[x] = color;
//        }
//        dest_buf+=dest_width;    /*Go to the next line*/
//    }
//}


#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
