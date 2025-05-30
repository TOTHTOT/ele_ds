<!--
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-02-15 12:25:00
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-22 19:19:00
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

- [x] 接收数据长度不对, 导致服务器发完数据还没退出接收状态; 修改计算文件长度方式解决.

- [x] 接收数据会丢包出现 ` get rb data failed, ret = 0`和`[E/client] rb put failed, ret = 0`, 得想办法加快写入速度, 要不然只能降低esp的波特率实现慢速接收文件; 关闭频繁的文件打开关闭操作提高写入速度就解决了.

## bug修复

- 修复终端不能交互问题, 现象为输入一个字符后就卡住了, 程序还在运行, 原因是接收完数据调用`clear_client_info()`清空接收信息, 这里`update_file_fd`等于0,关闭了标准输入的文件.

```c
static int32_t clear_client_info(ele_ds_client_t *client)
{
    if (client == RT_NULL)
    {
        LOG_E("client is NULL");
        return -1;
    }
    client->recv_info.recv_state = CRS_NONE;
    client->recv_info.curparse_type = EMT_CLIENTMSG_NONE;
    client->recv_info.datalen = 0;
    client->recv_info.recv_len = 0;
    if (client->recv_info.update_file_fd >= 0)// 这里的问题 >= 0
    {
        LOG_I("close update file fd = %d", client->recv_info.update_file_fd);
        close(client->recv_info.update_file_fd);
        client->recv_info.update_file_fd = -1;
    }
    return 0;
}
```

- rt-thread 在stm32f4环境下alarm不触发问题.
  
  - 经排查是设置alarm时没写入年份导致`sec_alarm = timegm(&alarm->wktime);`返回-1不满足if从而导致不执行回调函数.
  
  ```c
    static void alarm_wakeup(struct rt_alarm *alarm, struct tm *now)
    {
  ...
          case RT_ALARM_ONESHOT:
          {
              sec_alarm = timegm(&alarm->wktime);
              sec_now = timegm(now);
              if (((sec_now - sec_alarm) <= RT_ALARM_DELAY) && (sec_now >= sec_alarm))
          }
    }
  ...
  
    time_t timegm(struct tm * const t)
    {
    ...
        if (t->tm_year < 70)
        {
            rt_set_errno(EINVAL);
            return (time_t) -1;
        }
    ...
    }
    
    // 修复方法
    static rt_err_t stm32_rtc_set_alarm(struct rt_rtc_wkalarm *alarm)
    {
    ...
        rtc_device.wkalarm.tm_year = alarm->tm_year;
        rtc_device.wkalarm.tm_mon = alarm->tm_mon;
        rtc_device.wkalarm.tm_mday = alarm->tm_mday;
    ...
    }
  ```
