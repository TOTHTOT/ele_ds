/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-04-20 13:22:49
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-06 15:07:12
 * @FilePath: \ele_ds\applications\common\common.c
 * @Description: 常用函数
 */
#include "common.h"
#include "dfscfg.h"

/**
 * @description: 创建中间目录（递归）
 * @param {char} *dir
 * @return {*}
 */
static void mkdir_recursive(const char *dir)
{
    char tmp[MAX_PATH];
    snprintf(tmp, sizeof(tmp), "%s", dir);
    int len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = '\0';

    for (char *p = tmp + 1; *p; p++)
    {
        if (*p == '/')
        {
            *p = 0;
            mkdir(tmp, 0777); // 忽略已存在错误
            *p = '/';
        }
    }
    mkdir(tmp, 0777); // 创建最终路径
}

/**
 * @description: 解压tar文件到指定目录
 * @param {char} *tar_path tar文件路径
 * @param {char} *dst_dir  解压目标目录
 * @return {*}
 */
int untar(const char *tar_path, const char *dst_dir)
{
    mtar_t tar;
    mtar_header_t h;
    int err;

    err = mtar_open(&tar, tar_path, "r");
    if (err != MTAR_ESUCCESS)
    {
        printf("Failed to open tar: %s\n", mtar_strerror(err));
        return err;
    }

    while ((err = mtar_read_header(&tar, &h)) == MTAR_ESUCCESS)
    {
        // 构建目标文件路径
        char full_path[MAX_PATH];
        snprintf(full_path, sizeof(full_path), "%s/%s", dst_dir, h.name);

        if (h.type == MTAR_TREG)
        {
            printf("Extracting: %s -> %s (%d bytes)\n", h.name, full_path, h.size);

            // 创建目录（如果有子目录结构）
            char *last_slash = strrchr(full_path, '/');
            if (last_slash)
            {
                *last_slash = '\0';
                mkdir_recursive(full_path);
                *last_slash = '/';
            }

            FILE *f = fopen(full_path, "wb");
            if (!f)
            {
                printf("Failed to create file: %s\n", full_path);
                return -1;
            }

            int remaining = h.size;
            char buffer[512];
            while (remaining > 0)
            {
                int to_read = remaining > sizeof(buffer) ? sizeof(buffer) : remaining;
                err = mtar_read_data(&tar, buffer, to_read);
                if (err != MTAR_ESUCCESS)
                {
                    printf("Read error: %s\n", mtar_strerror(err));
                    fclose(f);
                    return err;
                }
                fwrite(buffer, 1, to_read, f);
                remaining -= to_read;
            }

            fclose(f);
        }
        else
        {
            // 非普通文件（目录等），跳过数据
            int remaining = h.size;
            char buffer[512];
            while (remaining > 0)
            {
                int to_read = remaining > sizeof(buffer) ? sizeof(buffer) : remaining;
                err = mtar_read_data(&tar, buffer, to_read);
                if (err != MTAR_ESUCCESS)
                {
                    printf("Skip read error: %s\n", mtar_strerror(err));
                    return err;
                }
                remaining -= to_read;
            }
        }

        mtar_next(&tar);
    }

    mtar_close(&tar);
    return 0;
}

#ifdef __RTTHREAD__
void untar_cmd(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage: untar <tar_path> <dst_dir>\n");
        return;
    }
    const char *tar_path = argv[1];
    const char *dst_dir = argv[2];

    if (untar(tar_path, dst_dir) == 0)
    {
        printf("Untar completed successfully.\n");
    }
    else
    {
        printf("Untar failed.\n");
    }
}
MSH_CMD_EXPORT_ALIAS(untar_cmd, untar, untar a tar file to a directory);
#endif /* __RTTHREAD__ */


/**
 * @description: 打印数组内容，带前缀
 * @param {char} *prefix 前缀字符串
 * @param {uint8_t} *array 数组指针
 * @param {size_t} size 数组大小
 * @return {*}
 */
void print_array_with_prefix(const char *prefix, const uint8_t *array, size_t size)
{
    if (prefix == NULL || array == NULL || size == 0)
    {
        printf("Invalid input.\n");
        return;
    }

    printf("%s: [", prefix);
    for (size_t i = 0; i < size; i++)
    {
        printf("%x", array[i]);
        if (i < size - 1)
        {
            printf(", ");
        }
    }
    printf("]\n");
}

/**
 * @description: 判断字符串是否为json格式
 * @param {char} *data 字符串数据
 * @param {size_t} len 字符串长度
 * @return {bool} true表示是json格式，false表示不是
 */
bool is_json(const char *data, size_t len)
{
    if (len < 2 || data == NULL)
    {
        return false; // 长度不足，不可能是 JSON
    }
    // 检查是否以 { } 或 [ ] 包裹
    if ((data[0] == '{' && data[len - 1] == '}') ||
        (data[0] == '[' && data[len - 1] == ']'))
    {
        return true;
    }
    return false;
}


#define CRC_BUFFER_SIZE 512  // 每次读取文件的块大小
/**
 * @description: 计算文件的CRC32值
 * @param {int} argc 参数数量, 程序内调用传入2
 * @param {char} * argv[] 参数数组, 程序内调用传入文件路径 argv[0]为程序名, argv[1]为文件路径
 * @return {uint32_t} CRC32值
 */
uint32_t crcfile(int argc, char **argv)
{
    if (argc != 2)
    {
        rt_kprintf("Usage: crcfile <filename>\n");
        return 0;
    }

    const char *filename = argv[1];
    int fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        rt_kprintf("Failed to open file: %s\n", filename);
        return 0;
    }

    // 创建 CRC 上下文
    struct rt_hwcrypto_ctx *ctx;
    struct hwcrypto_crc_cfg cfg =
    {
        .last_val = 0xFFFFFFFF,
        .poly     = 0x04C11DB7,
        .width    = 32,
        .xorout   = 0x00000000,
        .flags    = 0, // 默认无反转；如需支持反转可添加 RT_HWCRYPTO_CRC_FLAG_REVERSE_IN / OUT
    };

    ctx = rt_hwcrypto_crc_create(rt_hwcrypto_dev_default(), HWCRYPTO_CRC_CRC32);
    if (ctx == RT_NULL)
    {
        rt_kprintf("Failed to create CRC context\n");
        close(fd);
        return 0;
    }

    rt_hwcrypto_crc_cfg(ctx, &cfg);
    
    rt_tick_t start_tick = rt_tick_get();
    // 分块读取并计算 CRC
    uint8_t buffer[CRC_BUFFER_SIZE];
    ssize_t read_bytes;
    rt_uint32_t crc_result = 0;

    while ((read_bytes = read(fd, buffer, CRC_BUFFER_SIZE)) > 0)
    {
        crc_result = rt_hwcrypto_crc_update(ctx, buffer, read_bytes);
    }

		rt_tick_t end_tick = rt_tick_get();
		
    if (read_bytes < 0)
    {
        rt_kprintf("Error reading file\n");
    }
    else
    {
        rt_uint32_t elapsed_tick = end_tick - start_tick;
        float elapsed_ms = (float)elapsed_tick * 1000 / RT_TICK_PER_SECOND;
        rt_kprintf("CRC32 of file '%s': 0x%08X\n", filename, crc_result);
        rt_kprintf("Time used: %d ticks (%.3f ms)\n", elapsed_tick, elapsed_ms);

    }

    close(fd);
    rt_hwcrypto_crc_destroy(ctx);

    return crc_result;
}
MSH_CMD_EXPORT(crcfile, Calculate CRC32 of a file using hardware CRC);

/**
 * @description: 查找json在字符串中的起始位置
 * @param {char} *buffer 接收数据缓冲区
 * @param {int32_t} len 接收数据长度
 * @return {char} * 返回json起始位置指针
 */
char *find_json_start(char *buffer, int32_t len)
{
    char *start = NULL;
    for (int i = 0; i < len; i++)
    {
        if (buffer[i] == '{')
        {
            start = &buffer[i];
            break;
        }
    }
    return start;
}

