/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-02-16 19:11:22
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-04-07 13:19:00
 * @FilePath: \ele_ds\applications\ele_ds\ele_ds.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "ele_ds.h"
#include "fal.h"
#include "drv_spi.h"
#include "spi_flash_sfud.h"
#include "dfscfg.h"
#include <rtdevice.h>

#define DBG_TAG "ele_ds"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

// 全局变量
ele_ds_t g_ele_ds = RT_NULL; // 全局设备函数指针

#ifdef PKG_USING_GZP6816D_SENSOR
/**
 * @description: 获取传感器数据
 * @param {void} *para 回传数据
 * @return {rt_err_t} 函数执行结果, RT_EOK表示成功
 */
rt_err_t get_gzp6816d_data(void *para)
{
    RT_ASSERT(para != RT_NULL);
    gzp6816d_data_t *data = (gzp6816d_data_t *)para;
    rt_device_t gzp6816d_dev = rt_device_find("baro_gzp6816d");
    if (gzp6816d_dev == RT_NULL)
    {
        LOG_E("No gzp6816d device");
        return -RT_ERROR;
    }
    if (rt_device_open(gzp6816d_dev, RT_DEVICE_FLAG_RDWR) != RT_EOK)
    {
        LOG_E("Open gzp6816d device failed");
        return -RT_ERROR;
    }
    rt_device_control(gzp6816d_dev, RT_SENSOR_CTRL_SET_ODR, (void *)100);
    rt_err_t ret = rt_device_read(gzp6816d_dev, 0, data, sizeof(gzp6816d_data_t));
    rt_device_close(gzp6816d_dev);
    if (ret > 0)
    {
        // char str[64] = {0};
        // sprintf(str, "Pressure: %f, Temperature: %f", data->pressure, data->temperature);
        // LOG_D("%s", str);
        return RT_EOK;
    }
    else
    {
        LOG_E("Read gzp6816d data failed, %d", ret);
        return ret;
    }
}
#endif /* PKG_USING_GZP6816D_SENSOR */

#ifdef PKG_USING_SGP30
rt_err_t get_sgp30_data(void *para)
{
    RT_ASSERT(para != RT_NULL);
    rt_device_t tvoc_dev = RT_NULL, eco2_dev = RT_NULL;
    struct rt_sensor_data sensor_data[2] = {0};

    tvoc_dev = rt_device_find("tvoc_sg3");
    eco2_dev = rt_device_find("eco2_sg3");

    if (tvoc_dev == NULL || eco2_dev == NULL)
    {
        LOG_E("Can't find TVOC or eco2 device.\n");
        return -RT_ERROR;
    }
    if (rt_device_open(tvoc_dev, RT_DEVICE_FLAG_RDWR))
    {
        LOG_E("Open TVOC device failed.\n");
        return -RT_ERROR;
    }
    if (rt_device_open(eco2_dev, RT_DEVICE_FLAG_RDWR))
    {
        LOG_E("Open eco2 device failed.\n");
        return -RT_ERROR;
    }

    if (1 != rt_device_read(tvoc_dev, 0, &sensor_data[0], 1))
    {
        LOG_E("Read TVOC data failed.\n");
    }
    if (1 != rt_device_read(eco2_dev, 0, &sensor_data[1], 1))
    {
        LOG_E("Read eco2 data failed.\n");
    }
    int32_t data[2] = {0};
    data[0] = sensor_data[0].data.tvoc;
    data[1] = sensor_data[1].data.eco2;

    // 强制类型转换赋值回传数据
    int32_t *para_data = (int32_t *)para;
    para_data[0] = data[0];
    para_data[1] = data[1];
    // LOG_D("TVOC: %d ppb, eCO2: %d ppm\n", *(int32_t *)para, *((int32_t *)para + 1));
    rt_device_close(tvoc_dev);
    rt_device_close(eco2_dev);
    return RT_EOK;
}
#endif /* PKG_USING_SGP30 */

#ifdef PKG_USING_SHT3X
/**
 * @description: 获取SHT30数据
 * @param {void} *para 传入参数为两个float类型数组
 * @return {*}
 */
rt_err_t get_sht30_data(void *para)
{
    RT_ASSERT(para != RT_NULL);
    sht3x_device_t sht3x_dev = (sht3x_device_t)para;
    rt_err_t ret = RT_EOK;
    ret = sht3x_read_singleshot(sht3x_dev);
    if (ret == RT_EOK)
    {
        // LOG_D("sht30: %d, %d", (int)sht3x_dev->humidity, (int)sht3x_dev->temperature);
    }
    else
    {
        LOG_E("get_sht30_data() failed, ret = %d", ret);
    }
    return ret;
}
#endif /* PKG_USING_SHT3X */

/**
 * @description: 获取所有传感器数据
 * @param {void} *para 传入参数为ele_ds_t类型
 * @return {rt_err_t} 函数执行结果, RT_EOK表示成功
 */
rt_err_t get_all_sensor_data(void *para)
{
    RT_ASSERT(para != RT_NULL);

    ele_ds_t ele_ds = (ele_ds_t)para;
    // 没有传感器
    if (SENSOR_MAX == 0)
        return -ENOENT;
    // 传感器未初始化
    if (ele_ds->init_flag == false)
        return -EBUSY;
    int32_t ret = RT_EOK;
#ifdef PKG_USING_GZP6816D_SENSOR
    ret = ele_ds->ops.sensor_data[SENSOR_GZP6816D_INDEX](&ele_ds->sensor_data.gzp6816d);
    if (ret != RT_EOK)
    {
        LOG_E("get_gzp6816d_data() failed, ret = %d", ret);
    }
    else
    {
        // LOG_D("gzp6816d: pressure: %d, temperature: %d", (int32_t)ele_ds->sensor_data.gzp6816d.pressure, (int32_t)ele_ds->sensor_data.gzp6816d.temperature);
    }
#endif /* PKG_USING_GZP6816D_SENSOR */

#ifdef PKG_USING_SHT3X
    RT_ASSERT(ele_ds->devices.sht3x_dev != RT_NULL);
    ret = ele_ds->ops.sensor_data[SENSOR_SHT3X_INDEX](ele_ds->devices.sht3x_dev);
    if (ret != RT_EOK)
    {
        LOG_E("get_sht30_data() failed, ret = %d", ret);
    }
    else
    {
        ele_ds->sensor_data.sht30[0] = (int32_t)ele_ds->devices.sht3x_dev->temperature;
        ele_ds->sensor_data.sht30[1] = (int32_t)ele_ds->devices.sht3x_dev->humidity;
        // LOG_D("sht30: temperature: %d, humidity: %d", (int32_t)ele_ds->sensor_data.sht30[0], (int32_t)ele_ds->sensor_data.sht30[1]);
    }
#endif /* PKG_USING_SHT3X */

#ifdef PKG_USING_SGP30
    RT_ASSERT(ele_ds->ops.sensor_data[SENSOR_SGP30_INDEX] != RT_NULL);
    ret = ele_ds->ops.sensor_data[SENSOR_SGP30_INDEX](ele_ds->sensor_data.sgp30);
    if (ret != RT_EOK)
    {
        LOG_E("get_sgp30_data() failed, ret = %d", ret);
    }
    else
    {
        // LOG_D("sgp30: tvoc: %d, eco2: %d", ele_ds->sensor_data.sgp30[0], ele_ds->sensor_data.sgp30[1]);
    }
#endif /* PKG_USING_SGP30 */
    return RT_EOK;
}

/**
 * @description: 打印设备状态
 * @return {*}
 */
void ele_ds_print_status(void)
{
    if (g_ele_ds == RT_NULL)
    {
        rt_kprintf("ele_ds is NULL\n");
        return;
    }
    char str[64] = {0};
    memset(str, 0, sizeof(str));
#ifdef PKG_USING_GZP6816D_SENSOR
    sprintf(str, "[GZP6816D]Pressure: %f, Temperature: %f", g_ele_ds->sensor_data.gzp6816d.pressure, g_ele_ds->sensor_data.gzp6816d.temperature);
    rt_kprintf("%s\n", str);
#endif /* PKG_USING_GZP6816D_SENSOR */

#ifdef PKG_USING_SHT3X
    memset(str, 0, sizeof(str));
    sprintf(str, "[SHT30]Temperature: %f, Humidity: %f", g_ele_ds->sensor_data.sht30[0], g_ele_ds->sensor_data.sht30[1]);
    rt_kprintf("%s\n", str);
#endif /* PKG_USING_SHT3X */

#ifdef PKG_USING_SGP30
    rt_kprintf("[SGP30]TVOC: %d ppb, eCO2: %d ppm\n", g_ele_ds->sensor_data.sgp30[0], g_ele_ds->sensor_data.sgp30[1]);
#endif /* PKG_USING_SGP30 */
}
MSH_CMD_EXPORT_ALIAS(ele_ds_print_status, status, ele_ds_print_status);

ele_ds_ops_t ele_ds_ops =
    {
#ifdef PKG_USING_GZP6816D_SENSOR
        .sensor_data[SENSOR_GZP6816D_INDEX] = get_gzp6816d_data,
#endif /* PKG_USING_GZP6816D_SENSOR */
#ifdef PKG_USING_SHT3X
        .sensor_data[SENSOR_SHT3X_INDEX] = get_sht30_data,
#endif /* PKG_USING_SHT3X */
#ifdef PKG_USING_SGP30
        .sensor_data[SENSOR_SGP30_INDEX] = get_sgp30_data,
#endif /* PKG_USING_SGP30 */
        .sensor_data[SENSOR_MAX] = get_all_sensor_data,
};

UBYTE BlackImage[5808];
int EPD_test(void)
{
    printf("EPD_2IN7_V2_test Demo\r\n");
    if (DEV_Module_Init() != 0)
    {
        return -1;
    }

    printf("e-Paper Init and Clear...\r\n");
    EPD_2IN7_V2_Init();

    EPD_2IN7_V2_Clear();

    // Create a new image cache

    //    UWORD Imagesize = ((EPD_2IN7_V2_WIDTH % 8 == 0)? (EPD_2IN7_V2_WIDTH / 8 ): (EPD_2IN7_V2_WIDTH / 8 + 1)) * EPD_2IN7_V2_HEIGHT;
    //    if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
    //        printf("Failed to apply for black memory...\r\n");
    //        return -1;
    //    }
    printf("Paint_NewImage\r\n");
    Paint_NewImage(BlackImage, EPD_2IN7_V2_WIDTH, EPD_2IN7_V2_HEIGHT, 90, WHITE);
    Paint_Clear(WHITE);

#if 0 // Fast Drawing on the image
    // Fast refresh
    printf("This is followed by a quick refresh demo\r\n");
    printf("First, clear the screen\r\n");
    EPD_2IN7_V2_Init();
    EPD_2IN7_V2_Clear();

    printf("e-Paper Init Fast\r\n");
    EPD_2IN7_V2_Init_Fast();
		Paint_NewImage(BlackImage, EPD_2IN7_V2_WIDTH, EPD_2IN7_V2_HEIGHT, 90, WHITE);  	
    printf("Drawing\r\n");
    //1.Select Image
    Paint_SelectImage(BlackImage);

    // 2.Drawing on the image
    Paint_Clear(WHITE);
    printf("Drawing:BlackImage\r\n");
    Paint_DrawPoint(10, 80, BLACK, DOT_PIXEL_1X1, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 90, BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 100, BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);

    Paint_DrawLine(20, 70, 70, 120, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(70, 70, 20, 120, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    Paint_DrawRectangle(20, 70, 70, 120, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawRectangle(80, 70, 130, 120, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);

    Paint_DrawCircle(45, 95, 20, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawCircle(105, 95, 20, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);

    Paint_DrawLine(85, 95, 125, 95, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawLine(105, 75, 105, 115, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);

    Paint_DrawString_EN(10, 0, "waveshare", &Font16, BLACK, WHITE);
    Paint_DrawString_EN(10, 20, "hello world", &Font12, WHITE, BLACK);

    Paint_DrawNum(10, 33, 123456789, &Font12, BLACK, WHITE);
    Paint_DrawNum(10, 50, 987654321, &Font16, WHITE, BLACK);

    Paint_DrawString_CN(130, 0,"���abc", &Font12CN, BLACK, WHITE);
    Paint_DrawString_CN(130, 20, "΢ѩ����", &Font24CN, WHITE, BLACK);

    EPD_2IN7_V2_Display_Fast(BlackImage);
    DEV_Delay_ms(3000);

#endif

#if 1 // show bmp

    printf("show window BMP-----------------\r\n");
    EPD_2IN7_V2_Init();
    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);
    // Paint_DrawBitMap(gImage_2in7);
    EPD_2IN7_V2_Display(BlackImage);
    DEV_Delay_ms(3000);

#endif

#if 1 // Drawing on the image
    Paint_NewImage(BlackImage, EPD_2IN7_V2_WIDTH, EPD_2IN7_V2_HEIGHT, 90, WHITE);
    printf("Drawing\r\n");
    // 1.Select Image
    EPD_2IN7_V2_Init();
    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);

    // 2.Drawing on the image
    printf("Drawing:BlackImage\r\n");
    Paint_DrawPoint(10, 80, BLACK, DOT_PIXEL_1X1, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 90, BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 100, BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);

    Paint_DrawLine(20, 70, 70, 120, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(70, 70, 20, 120, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    Paint_DrawRectangle(20, 70, 70, 120, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawRectangle(80, 70, 130, 120, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);

    Paint_DrawCircle(45, 95, 20, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawCircle(105, 95, 20, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);

    Paint_DrawLine(85, 95, 125, 95, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawLine(105, 75, 105, 115, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);

    Paint_DrawString_EN(10, 0, "waveshare", &Font16, BLACK, WHITE);
    Paint_DrawString_EN(10, 20, "hello world", &Font12, WHITE, BLACK);

    Paint_DrawNum(10, 33, 123456789, &Font12, BLACK, WHITE);
    Paint_DrawNum(10, 50, 987654321, &Font16, WHITE, BLACK);

    // Paint_DrawString_CN(130, 0, "���abc", &Font12CN, BLACK, WHITE);
    // Paint_DrawString_CN(130, 20, "΢ѩ����", &Font24CN, WHITE, BLACK);

    EPD_2IN7_V2_Display_Base(BlackImage);
    DEV_Delay_ms(3000);
#endif

#if 1 // Partial refresh, example shows time
    // If you didn't use the EPD_2IN7_V2_Display_Base() function to refresh the image before,
    // use the EPD_2IN7_V2_Display_Base_color() function to refresh the background color,
    // otherwise the background color will be garbled
    EPD_2IN7_V2_Init();
    // EPD_2IN7_V2_Display_Base_color(WHITE);
    Paint_NewImage(BlackImage, 50, 120, 90, WHITE);

    printf("Partial refresh\r\n");
    Paint_SelectImage(BlackImage);
    Paint_SetScale(2);
    Paint_Clear(WHITE);

    PAINT_TIME sPaint_time;
    sPaint_time.Hour = 12;
    sPaint_time.Min = 34;
    sPaint_time.Sec = 56;
    UBYTE num = 15;
    for (;;)
    {
        sPaint_time.Sec = sPaint_time.Sec + 1;
        if (sPaint_time.Sec == 60)
        {
            sPaint_time.Min = sPaint_time.Min + 1;
            sPaint_time.Sec = 0;
            if (sPaint_time.Min == 60)
            {
                sPaint_time.Hour = sPaint_time.Hour + 1;
                sPaint_time.Min = 0;
                if (sPaint_time.Hour == 24)
                {
                    sPaint_time.Hour = 0;
                    sPaint_time.Min = 0;
                    sPaint_time.Sec = 0;
                }
            }
        }

        Paint_Clear(WHITE);
        Paint_DrawString_EN(10, 20, "hello world", &Font12, WHITE, BLACK);
        Paint_DrawRectangle(1, 1, 120, 50, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
        Paint_DrawTime(10, 15, &sPaint_time, &Font20, WHITE, BLACK);

        num = num - 1;
        if (num == 0)
        {
            break;
        }
        printf("Part refresh...\r\n");
        EPD_2IN7_V2_Display_Partial(BlackImage, 60, 134, 110, 254); // Xstart must be a multiple of 8
        DEV_Delay_ms(500);
    }
#endif

#if 0 // show image for array
		free(BlackImage);
    printf("show Gray------------------------\r\n");
//    Imagesize = ((EPD_2IN7_V2_WIDTH % 4 == 0)? (EPD_2IN7_V2_WIDTH / 4 ): (EPD_2IN7_V2_WIDTH / 4 + 1)) * EPD_2IN7_V2_HEIGHT;
//    if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
//        printf("Failed to apply for black memory...\r\n");
//        return -1;
//    }
    EPD_2IN7_V2_Init_4GRAY();
    printf("4 grayscale display\r\n");
    Paint_NewImage(BlackImage, EPD_2IN7_V2_WIDTH, EPD_2IN7_V2_HEIGHT, 90, WHITE);
    Paint_SetScale(4);
    Paint_Clear(0xff);
    
    Paint_DrawPoint(10, 80, GRAY4, DOT_PIXEL_1X1, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 90, GRAY4, DOT_PIXEL_2X2, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 100, GRAY4, DOT_PIXEL_3X3, DOT_STYLE_DFT);
    Paint_DrawLine(20, 70, 70, 120, GRAY4, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(70, 70, 20, 120, GRAY4, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawRectangle(20, 70, 70, 120, GRAY4, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawRectangle(80, 70, 130, 120, GRAY4, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawCircle(45, 95, 20, GRAY4, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawCircle(105, 95, 20, GRAY2, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawLine(85, 95, 125, 95, GRAY4, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawLine(105, 75, 105, 115, GRAY4, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawString_EN(10, 0, "waveshare", &Font16, GRAY4, GRAY1);
    Paint_DrawString_EN(10, 20, "hello world", &Font12, GRAY3, GRAY1);
    Paint_DrawNum(10, 33, 123456789, &Font12, GRAY4, GRAY2);
    Paint_DrawNum(10, 50, 987654321, &Font16, GRAY1, GRAY4);
    Paint_DrawString_CN(150, 0,"���abc", &Font12CN, GRAY4, GRAY1);
    Paint_DrawString_CN(150, 20,"���abc", &Font12CN, GRAY3, GRAY2);
    Paint_DrawString_CN(150, 40,"���abc", &Font12CN, GRAY2, GRAY3);
    Paint_DrawString_CN(150, 60,"���abc", &Font12CN, GRAY1, GRAY4);
    Paint_DrawString_CN(10, 130, "΢ѩ����", &Font24CN, GRAY1, GRAY4);
    EPD_2IN7_V2_4GrayDisplay(BlackImage);
    DEV_Delay_ms(3000);
		
		Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);
    Paint_DrawBitMap(gImage_2in7_4Gray);
    EPD_2IN7_V2_4GrayDisplay(BlackImage);
    DEV_Delay_ms(3000);

#endif

    printf("Clear...\r\n");
    EPD_2IN7_V2_Init();
    EPD_2IN7_V2_Clear();

    printf("Goto Sleep...\r\n");
    EPD_2IN7_V2_Sleep();
    // free(BlackImage);
    //    BlackImage = NULL;
    DEV_Delay_ms(2000); // important, at least 2s
    // close 5V
    printf("close 5V, Module enters 0 power consumption ...\r\n");
    DEV_Module_Exit();
    return 0;
}

#ifdef PKG_USING_SGP30
int rt_hw_sgp30_port(void)
{
    struct rt_sensor_config cfg;

    cfg.intf.type = RT_SENSOR_INTF_I2C;
    cfg.intf.dev_name = "i2c1";
    cfg.intf.user_data = (void *)PKG_USING_SGP30_I2C_ADDRESS;
    rt_hw_sgp30_init("sg3", &cfg);

    return RT_EOK;
}
// 这里注册会导致堆栈溢出 可能是处理注册函数的那部分功能分配的堆栈不够导致运行到获取数据的函数时报错
// (rt_object_get_type(&mutex->parent.parent) == RT_Object_Class_Mutex) assertion failed at function:_rt_mutex_take, line number:rt_backtrace is not implemented
// INIT_COMPONENT_EXPORT(rt_hw_sgp30_port);
#endif /* PKG_USING_SGP30 */

rt_err_t ele_ds_epaper_init(ele_ds_t ele_ds)
{
    RT_ASSERT(ele_ds != RT_NULL);
    rt_err_t err = RT_EOK;
    err = rt_hw_spi_device_attach("spi1", EPAPER_DEVNAME, GET_PIN(A, 4));
    // err = rt_spi_bus_attach_device(ele_ds->devices.epaper_dev, EPAPER_DEVNAME, "spi1", ele_ds);
    if (err)
    {
        LOG_E("attach %s error", EPAPER_DEVNAME);
        return -err;
    }
    ele_ds->devices.epaper_dev = (struct rt_spi_device *)rt_device_find(EPAPER_DEVNAME);
    if (ele_ds->devices.epaper_dev == RT_NULL)
    {
        LOG_E("find %s error", EPAPER_DEVNAME);
        return -RT_ERROR;
    }
    spi_epaper = ele_ds->devices.epaper_dev;
    struct rt_spi_configuration cfg = {
        .mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB,
        .data_width = 8,
        .max_hz = 1 * 1000 * 1000};
    err = rt_spi_configure(ele_ds->devices.epaper_dev, &cfg);
    if (err != RT_EOK)
    {
        LOG_E("configure %s error", EPAPER_DEVNAME);
        return -err;
    }

    rt_pin_mode(EPD_RST_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(EPD_DC_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(EPD_CS_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(EPD_BUSY_PIN, PIN_MODE_INPUT);
    DEV_Module_Init();
    EPD_test();
    return RT_EOK;
}

static int rt_hw_spi_flash_init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    rt_hw_spi_device_attach("spi1", "spi10", GET_PIN(C, 4)); // spi10 表示挂载在 spi3 总线上的 0 号设备,PC0是片选, 这一步就可以将从设备挂在到总线中。

    if (RT_NULL == rt_sfud_flash_probe("norflash0", "spi10")) // 注册块设备, 这一步可以将外部flash抽象为系统的块设备
    {
        return -RT_ERROR;
    };

    return RT_EOK;
}

int32_t devices_init(ele_ds_t ele_ds)
{
    rt_err_t err = RT_EOK;
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(V3_3_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(V3_3_PIN, PIN_HIGH);

    if (ele_ds == RT_NULL)
    {
        LOG_E("ele_ds is NULL");
        return -1;
    }
    memset(ele_ds, 0, sizeof(ele_ds_t));
    // fal_init();
    rt_hw_spi_flash_init();
#if 1
    err = ele_ds_epaper_init(ele_ds);
    if (err != RT_EOK)
    {
        LOG_E("ele_ds_epaper_init failed");
        return -2;
    }
#endif

#ifdef PKG_USING_SGP30
    if (rt_hw_sgp30_port() != RT_EOK)
    {
        LOG_E("sgp30 port init failed");
        return -3;
    }
#endif /* PKG_USING_SGP30 */

#ifdef PKG_USING_GZP6816D_SENSOR
    if (gzp6816d_init("i2c1", GZP6816D_ADDR) != RT_EOK)
    {
        LOG_E("gzp6816d init failed");
        return -4;
    }
#endif /* PKG_USING_GZP6816D_SENSOR */
#ifdef PKG_USING_SHT3X
    ele_ds->devices.sht3x_dev = sht3x_init("i2c1", SHT3X_ADDR_PD);
#endif /* PKG_USING_SHT3X */
    mnt_init();
    ele_ds->ops = ele_ds_ops;
    ele_ds->init_flag = true;
    return 0;
}
