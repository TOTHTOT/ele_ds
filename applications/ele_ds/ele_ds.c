/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-02-16 19:11:22
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-02-25 13:31:06
 * @FilePath: \ele_ds\applications\ele_ds\ele_ds.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "ele_ds.h"

#define DBG_TAG "ele_ds"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

// 全局变量
ele_ds_t g_ele_ds = RT_NULL; // 全局设备函数指针

#ifdef PKG_USING_GZP6816D_SENSOR
/**
 * @description: 获取传感器数据
 * @param {void} *para 回传数据
 * @return {rt_err_t} 函数执行结果，RT_EOK表示成功
 */
rt_err_t get_gzp6816d_data(void *para)
{
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
    LOG_D("TVOC: %d ppb, eCO2: %d ppm\n", *(int32_t *)para, *((int32_t *)para + 1));
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
    RT_ASSERT(g_ele_ds->devices.sht3x_dev != RT_NULL);

    rt_err_t ret = RT_EOK;
    float data[2] = {0}; // 温度和湿度
    ret = sht3x_read_singleshot(g_ele_ds->devices.sht3x_dev);
    if (ret == RT_EOK)
    {
        data[0] = g_ele_ds->devices.sht3x_dev->temperature;
        data[1] = g_ele_ds->devices.sht3x_dev->humidity;
        memcpy(para, data, sizeof(data));
    }
    return ret;
}
#endif /* PKG_USING_SHT3X */

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

#ifdef PKG_USING_GZP6816D_SENSOR
    LOG_D("Get gzp6816d data");
    ele_ds->ops.sensor_data[SENSOR_GZP6816D_INDEX](&ele_ds->sensor_data.gzp6816d);
#endif /* PKG_USING_GZP6816D_SENSOR */
#ifdef PKG_USING_SHT3X
    RT_ASSERT(ele_ds->devices.sht3x_dev != RT_NULL);
    LOG_D("Get sht30 data");
    ele_ds->ops.sensor_data[SENSOR_SHT3X_INDEX](ele_ds->sensor_data.sht30);
#endif /* PKG_USING_SHT3X */
#ifdef PKG_USING_SGP30
    RT_ASSERT(ele_ds->ops.sensor_data[SENSOR_SGP30_INDEX] != RT_NULL);
    LOG_D("Get sgp30 data");
    ele_ds->ops.sensor_data[SENSOR_SGP30_INDEX](ele_ds->sensor_data.sgp30);
#endif /* PKG_USING_SGP30 */
    return RT_EOK;
}

/**
 * @description: 打印设备状态
 * @return {*}
 */
void ele_ds_print_status(void)
{
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
    rt_kprintf("TVOC: %d ppb, eCO2: %d ppm\n", g_ele_ds->sensor_data.sgp30[0], g_ele_ds->sensor_data.sgp30[1]);
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

int EPD_test(void)
{
    LOG_D("EPD_2IN7B_V2_test Demo");
    if(DEV_Module_Init()!=0){
        LOG_E("Module Init Failed");
        return -1;
    }
    
    LOG_D("e-Paper Init and Clear...");
    EPD_2IN7B_V2_Init();

    EPD_2IN7B_V2_Clear();
    DEV_Delay_ms(500);

    //Create a new image cache named IMAGE_BW and fill it with white
    UBYTE *BlackImage, *RedImage;
    UWORD Imagesize = ((EPD_2IN7B_V2_WIDTH % 8 == 0)? (EPD_2IN7B_V2_WIDTH / 8 ): (EPD_2IN7B_V2_WIDTH / 8 + 1)) * EPD_2IN7B_V2_HEIGHT;
    if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
        LOG_E("Failed to apply for black memory...");
        return -1;
    }
    if((RedImage = (UBYTE *)malloc(Imagesize)) == NULL) {
        LOG_E("Failed to apply for red memory...");
        return -1;
    }
    LOG_D("NewImage:BlackImage and RedImage");
    Paint_NewImage(BlackImage, EPD_2IN7B_V2_WIDTH, EPD_2IN7B_V2_HEIGHT, 270, WHITE);
    Paint_NewImage(RedImage, EPD_2IN7B_V2_WIDTH, EPD_2IN7B_V2_HEIGHT, 270, WHITE);

    //Select Image
    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);
    Paint_SelectImage(RedImage);
    Paint_Clear(WHITE);

#if 0   // show image for array   
    LOG_D("show image for array");
    Paint_SelectImage(BlackImage);
    Paint_DrawBitMap(gImage_2in7b_Black);
    Paint_SelectImage(RedImage);
    Paint_DrawBitMap(gImage_2in7b_Red);
    EPD_2IN7B_V2_Display(BlackImage, RedImage);
    DEV_Delay_ms(4000);
#endif

#if 1   // Drawing on the image
    /*Horizontal screen*/
    //1.Draw black image
    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);
    Paint_DrawPoint(10, 80, BLACK, DOT_PIXEL_1X1, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 90, BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 100, BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);
    Paint_DrawPoint(10, 110, BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);
    Paint_DrawLine(20, 70, 70, 120, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(70, 70, 20, 120, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawRectangle(20, 70, 70, 120, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawRectangle(80, 70, 130, 120, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawString_EN(10, 0, "waveshare", &Font16, BLACK, WHITE);
    // Paint_DrawString_CN(130, 20, "΢ѩ����", &Font24CN, WHITE, BLACK);
    Paint_DrawNum(10, 50, 987654321, &Font16, WHITE, BLACK);

    //2.Draw red image
    Paint_SelectImage(RedImage);
    Paint_Clear(WHITE);
    Paint_DrawCircle(160, 95, 20, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawCircle(210, 95, 20, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawLine(85, 95, 125, 95, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawLine(105, 75, 105, 115, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    // Paint_DrawString_CN(130, 0,"���abc", &Font12CN, BLACK, WHITE);
    Paint_DrawString_EN(10, 20, "hello world", &Font12, WHITE, BLACK);
    Paint_DrawNum(10, 33, 123456789, &Font12, BLACK, WHITE);

    LOG_D("EPD_Display");
    EPD_2IN7B_V2_Display(BlackImage, RedImage);
    DEV_Delay_ms(4000);
#endif

    LOG_D("Clear...");
    EPD_2IN7B_V2_Clear();

    LOG_D("Goto Sleep...");
    EPD_2IN7B_V2_Sleep();
    free(BlackImage);
    BlackImage = NULL;
    DEV_Delay_ms(2000);//important, at least 2s
    // close 5V
    LOG_D("close 5V, Module enters 0 power consumption ...");
    DEV_Module_Exit(); 
    return 0;
}

#ifdef PKG_USING_SGP30
static int rt_hw_sgp30_port(void)
{
    struct rt_sensor_config cfg;
    
    cfg.intf.type = RT_SENSOR_INTF_I2C;
    cfg.intf.dev_name = "i2c1";
    cfg.intf.user_data = (void *)PKG_USING_SGP30_I2C_ADDRESS;
    rt_hw_sgp30_init("sg3", &cfg);
    
    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_sgp30_port);
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
    DEV_Module_Init();
    EPD_test();
    return RT_EOK;
}

ele_ds_t devices_init(void)
{
    rt_err_t err = RT_EOK;
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(V3_3_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(V3_3_PIN, PIN_HIGH);

    ele_ds_t ele_ds = rt_calloc(1, sizeof(ele_ds_t));
    if (ele_ds == RT_NULL)
    {
        LOG_E("No memory for ele_ds");
        return NULL;
    }
#ifdef PKG_USING_SHT3X
    ele_ds->devices.sht3x_dev = sht3x_init("i2c1", SHT3X_ADDR_PD);
#endif /* PKG_USING_SHT3X */

#if 0
    err = ele_ds_epaper_init(ele_ds);
    if (err != RT_EOK)
    {
        LOG_E("ele_ds_epaper_init failed");
        return NULL;
    }
    spi_epaper = ele_ds->devices.epaper_dev;
#endif
#ifdef 0
    rt_hw_sgp30_port();
#endif /* PKG_USING_SGP30 */
    ele_ds->ops = ele_ds_ops;
    ele_ds->init_flag = true;
    g_ele_ds = ele_ds;
    return ele_ds;
}


