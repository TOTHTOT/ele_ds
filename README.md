<!--
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-02-15 12:25:00
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-07 17:50:39
 * @FilePath: \ele_ds\README.md
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
-->

# 电子桌搭

## 字体替换

- 使用`LvglFontTool`编译字库, 更新`__user_font_getd()`函数, `fangsong_8.bin`通过`ymodem`写入到`w25q128`中.

```c
static uint8_t __g_font_buf[30];//如bin文件存在SPI FLASH可使用此buff

static uint8_t *__user_font_getdata(int offset, int size){
    //如字模保存在SPI FLASH, SPIFLASH_Read(__g_font_buf,offset,size);
    //如字模已加载到SDRAM,直接返回偏移地址即可如:return (uint8_t*)(sdram_fontddr+offset);
    int fd = open("/sysfile/font/fangsong_8.bin", O_RDONLY);
    if (fd < 0) {
        printf("打开文件失败！\n");
        return NULL;
    }
    lseek(fd, offset, SEEK_SET);
    read(fd, __g_font_buf, size);
#if 0
    printf("offset:%d, size:%d\n", offset, size);
    for (size_t i = 0; i < size; i++)
    {
        printf("%02X ", __g_font_buf[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
#endif
    close(fd);
    return __g_font_buf;
}

// 使用方式
static lv_style_t style;
lv_style_init(&style);  // 初始化 style

// 设置字体
extern lv_font_t fangsong_8;
lv_style_set_text_font(&style, &fangsong_8);

// 创建 label
lv_obj_t *label = lv_label_create(lv_scr_act());

// 设置样式
lv_obj_add_style(label, &style, 0);

// 设置文本
lv_label_set_text(label, "你好世界");
lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
```

## 备注

- sht30 中`sht3x_read_singleshot()`发完命令后延迟要加大到50ms.

## 存在问题

- [ ] 接收数据长度不对, 导致服务器发完数据还没退出接收状态;

- [ ] 接收数据会丢包出现 ` get rb data failed, ret = 0`和`[E/client] rb put failed, ret = 0`, 得想办法加快写入速度, 要不然只能降低esp的波特率实现慢速接收文件;
