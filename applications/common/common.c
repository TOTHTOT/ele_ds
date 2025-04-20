/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-04-20 13:22:49
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-04-20 13:30:13
 * @FilePath: \ele_ds\applications\common\common.c
 * @Description: 常用函数
 */
#include "common.h"
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
