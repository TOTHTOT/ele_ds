/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-04-20 13:22:49
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-23 14:52:37
 * @FilePath: \ele_ds\applications\common\common.c
 * @Description: 常用函数
 */
#include "common.h"

#if 0
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
#endif


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
static unsigned crc32_table[256] = {
    0u, 1996959894u, 3993919788u, 2567524794u,  124634137u, 1886057615u, 3915621685u, 2657392035u,
249268274u, 2044508324u, 3772115230u, 2547177864u,  162941995u, 2125561021u, 3887607047u, 2428444049u,
498536548u, 1789927666u, 4089016648u, 2227061214u,  450548861u, 1843258603u, 4107580753u, 2211677639u,
325883990u, 1684777152u, 4251122042u, 2321926636u,  335633487u, 1661365465u, 4195302755u, 2366115317u,
997073096u, 1281953886u, 3579855332u, 2724688242u, 1006888145u, 1258607687u, 3524101629u, 2768942443u,
901097722u, 1119000684u, 3686517206u, 2898065728u,  853044451u, 1172266101u, 3705015759u, 2882616665u,
651767980u, 1373503546u, 3369554304u, 3218104598u,  565507253u, 1454621731u, 3485111705u, 3099436303u,
671266974u, 1594198024u, 3322730930u, 2970347812u,  795835527u, 1483230225u, 3244367275u, 3060149565u,
1994146192u,   31158534u, 2563907772u, 4023717930u, 1907459465u,  112637215u, 2680153253u, 3904427059u,
2013776290u,  251722036u, 2517215374u, 3775830040u, 2137656763u,  141376813u, 2439277719u, 3865271297u,
1802195444u,  476864866u, 2238001368u, 4066508878u, 1812370925u,  453092731u, 2181625025u, 4111451223u,
1706088902u,  314042704u, 2344532202u, 4240017532u, 1658658271u,  366619977u, 2362670323u, 4224994405u,
1303535960u,  984961486u, 2747007092u, 3569037538u, 1256170817u, 1037604311u, 2765210733u, 3554079995u,
1131014506u,  879679996u, 2909243462u, 3663771856u, 1141124467u,  855842277u, 2852801631u, 3708648649u,
1342533948u,  654459306u, 3188396048u, 3373015174u, 1466479909u,  544179635u, 3110523913u, 3462522015u,
1591671054u,  702138776u, 2966460450u, 3352799412u, 1504918807u,  783551873u, 3082640443u, 3233442989u,
3988292384u, 2596254646u,   62317068u, 1957810842u, 3939845945u, 2647816111u,   81470997u, 1943803523u,
3814918930u, 2489596804u,  225274430u, 2053790376u, 3826175755u, 2466906013u,  167816743u, 2097651377u,
4027552580u, 2265490386u,  503444072u, 1762050814u, 4150417245u, 2154129355u,  426522225u, 1852507879u,
4275313526u, 2312317920u,  282753626u, 1742555852u, 4189708143u, 2394877945u,  397917763u, 1622183637u,
3604390888u, 2714866558u,  953729732u, 1340076626u, 3518719985u, 2797360999u, 1068828381u, 1219638859u,
3624741850u, 2936675148u,  906185462u, 1090812512u, 3747672003u, 2825379669u,  829329135u, 1181335161u,
3412177804u, 3160834842u,  628085408u, 1382605366u, 3423369109u, 3138078467u,  570562233u, 1426400815u,
3317316542u, 2998733608u,  733239954u, 1555261956u, 3268935591u, 3050360625u,  752459403u, 1541320221u,
2607071920u, 3965973030u, 1969922972u,   40735498u, 2617837225u, 3943577151u, 1913087877u,   83908371u,
2512341634u, 3803740692u, 2075208622u,  213261112u, 2463272603u, 3855990285u, 2094854071u,  198958881u,
2262029012u, 4057260610u, 1759359992u,  534414190u, 2176718541u, 4139329115u, 1873836001u,  414664567u,
2282248934u, 4279200368u, 1711684554u,  285281116u, 2405801727u, 4167216745u, 1634467795u,  376229701u,
2685067896u, 3608007406u, 1308918612u,  956543938u, 2808555105u, 3495958263u, 1231636301u, 1047427035u,
2932959818u, 3654703836u, 1088359270u,  936918000u, 2847714899u, 3736837829u, 1202900863u,  817233897u,
3183342108u, 3401237130u, 1404277552u,  615818150u, 3134207493u, 3453421203u, 1423857449u,  601450431u,
3009837614u, 3294710456u, 1567103746u,  711928724u, 3020668471u, 3272380065u, 1510334235u,  755167117u
};

static uint32_t soft_crc32(const uint8_t *buf, size_t len, uint32_t crc_init)
{
    uint32_t crc = crc_init ^ 0xFFFFFFFF;
    while (len--)
        crc = crc32_table[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFF;
}


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

    uint8_t buffer[CRC_BUFFER_SIZE];
    ssize_t read_bytes;
    uint32_t crc_result = 0;

    rt_tick_t start_tick = rt_tick_get();

    while ((read_bytes = read(fd, buffer, CRC_BUFFER_SIZE)) > 0)
    {
        crc_result = soft_crc32(buffer, read_bytes, crc_result);
    }

    rt_tick_t end_tick = rt_tick_get();
    float elapsed_ms = (float)(end_tick - start_tick) * 1000 / RT_TICK_PER_SECOND;

    close(fd);

    rt_kprintf("CRC32 of file '%s': 0x%08X\n", filename, crc_result);
    rt_kprintf("Time used: %d ticks (%.3f ms)\n", end_tick - start_tick, elapsed_ms);

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

