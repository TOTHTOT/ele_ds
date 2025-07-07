/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-04-07 09:21:50
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-28 14:12:03
 * @FilePath: \ele_ds\applications\dfs\dfscfg.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "dfscfg.h"
#include "ele_ds.h"
#include "cJSON.h"

#define DBG_TAG "ele_ds"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

const struct dfs_mount_tbl mount_table[] =
{
        // {"norflash0", "/", "elm", 0, 0},
        {0},
};

void init_ele_ds_cfg(ele_ds_cfg_t *cfg)
{
    memset(cfg, 0, sizeof(ele_ds_cfg_t));
    memset(cfg->weather_info, 0, sizeof(cfg->weather_info));
    cfg->check = CFGFILE_CHECK;
    cfg->server_port = CFGFILE_DEFATLT_SERVER_PORT;
    cfg->tcp_timeout = CFGFILE_DEFAULT_TCP_TIMEOUT;
    cfg->alarm_enable = false;
    cfg->alarm_time = CFGFILE_DEFAULT_ALARM_TIME;
    cfg->clientcfg.cityid = DEFATLT_CITYID;
	cfg->clientcfg.version = SOFT_VERSION;
	cfg->time_zone = CFGFILE_DEFAULT_TIMEZONE;
    strncpy((char *)cfg->memo, CFGFILE_DEFAULT_MEMO_CONTENT, sizeof(cfg->memo) - 1);
    strncpy((char *)cfg->wifi_ssid, CFGFILE_DEFATLT_WIFI_SSID, sizeof(cfg->wifi_ssid) - 1);
    strncpy((char *)cfg->wifi_passwd, CFGFILE_DEFATLT_WIFI_PASS, sizeof(cfg->wifi_passwd) - 1);
    strncpy((char *)cfg->server_addr, CFGFILE_DEFATLT_SERVER_ADDR, sizeof(cfg->server_addr) - 1);
}

static cJSON *ele_ds_cfg_to_json(const ele_ds_cfg_t *cfg)
{
    cJSON *root = cJSON_CreateObject();
    if (root == NULL)
    {
        LOG_E("cJSON_CreateObject failed");
        return NULL;
    }
    cJSON *clientcfg = cJSON_CreateObject();
    if (clientcfg == NULL)
    {
        LOG_E("cJSON_CreateObject for clientcfg failed");
        cJSON_Delete(root);
        return NULL;
    }
    cJSON_AddStringToObject(clientcfg, "username", (const char *)cfg->clientcfg.username);
    cJSON_AddStringToObject(clientcfg, "passwd", (const char *)cfg->clientcfg.passwd);
    cJSON_AddStringToObject(clientcfg, "cityname", (const char *)cfg->clientcfg.cityname);
    cJSON_AddNumberToObject(clientcfg, "cityid", cfg->clientcfg.cityid);
    cJSON_AddNumberToObject(clientcfg, "cntserver_interval", cfg->clientcfg.cntserver_interval);
    cJSON_AddNumberToObject(clientcfg, "version", cfg->clientcfg.version);
    cJSON_AddNumberToObject(clientcfg, "battery", cfg->clientcfg.battery);
    cJSON_AddItemToObject(root, "clientcfg", clientcfg);

    cJSON_AddStringToObject(root, "wifi_ssid", (const char *)cfg->wifi_ssid);
    cJSON_AddStringToObject(root, "wifi_passwd", (const char *)cfg->wifi_passwd);
    cJSON_AddStringToObject(root, "server_addr", (const char *)cfg->server_addr);
    cJSON_AddNumberToObject(root, "server_port", cfg->server_port);
    cJSON_AddNumberToObject(root, "tcp_timeout", cfg->tcp_timeout);
    cJSON_AddNumberToObject(root, "alarm_enable", cfg->alarm_enable);
    cJSON_AddNumberToObject(root, "alarm_time", cfg->alarm_time);
    cJSON_AddNumberToObject(root, "time_zone", cfg->time_zone);
    cJSON_AddStringToObject(root, "memo", cfg->memo);

    cJSON *weather_array = cJSON_CreateArray();
    for (int i = 0; i < 7; i++)
    {
        cJSON *item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "iconDay", cfg->weather_info[i].iconDay);
        cJSON_AddStringToObject(item, "textDay", cfg->weather_info[i].textDay);
        cJSON_AddNumberToObject(item, "tempMax", cfg->weather_info[i].tempMax);
        cJSON_AddNumberToObject(item, "tempMin", cfg->weather_info[i].tempMin);
        cJSON_AddItemToArray(weather_array, item);
    }
    cJSON_AddItemToObject(root, "weather_info", weather_array);

    return root;
}

static void json_to_ele_ds_cfg(ele_ds_cfg_t *cfg, const cJSON *json)
{
    memset(cfg, 0, sizeof(ele_ds_cfg_t));

    cJSON *clientcfg = cJSON_GetObjectItem(json, "clientcfg");
    if (clientcfg == NULL)
    {
        LOG_E("clientcfg not found in json");
        return;
    }
    strcpy((char *)cfg->clientcfg.username, cJSON_GetObjectItem(clientcfg, "username")->valuestring);
    strcpy((char *)cfg->clientcfg.passwd, cJSON_GetObjectItem(clientcfg, "passwd")->valuestring);
    strcpy((char *)cfg->clientcfg.cityname, cJSON_GetObjectItem(clientcfg, "cityname")->valuestring);
    cfg->clientcfg.cityid = cJSON_GetObjectItem(clientcfg, "cityid")->valueint;
    cfg->clientcfg.cntserver_interval = cJSON_GetObjectItem(clientcfg, "cntserver_interval")->valueint;
    cfg->clientcfg.version = cJSON_GetObjectItem(clientcfg, "version")->valueint;
    cfg->clientcfg.battery = cJSON_GetObjectItem(clientcfg, "battery")->valueint;

    strcpy((char *)cfg->wifi_ssid, cJSON_GetObjectItem(json, "wifi_ssid")->valuestring);
    strcpy((char *)cfg->wifi_passwd, cJSON_GetObjectItem(json, "wifi_passwd")->valuestring);
    strcpy((char *)cfg->server_addr, cJSON_GetObjectItem(json, "server_addr")->valuestring);
    cfg->server_port = cJSON_GetObjectItem(json, "server_port")->valueint;
    cfg->tcp_timeout = cJSON_GetObjectItem(json, "tcp_timeout")->valueint;
    cfg->alarm_enable = cJSON_GetObjectItem(json, "alarm_enable")->valueint;
    cfg->alarm_time = cJSON_GetObjectItem(json, "alarm_time")->valueint;
    cfg->time_zone = cJSON_GetObjectItem(json, "time_zone")->valueint;
    cfg->clientcfg.version = cJSON_GetObjectItem(json, "version")->valueint;
    strcpy(cfg->memo, cJSON_GetObjectItem(json, "memo")->valuestring);

    cJSON *weather_array = cJSON_GetObjectItem(json, "weather_info");
    for (int i = 0; i < cJSON_GetArraySize(weather_array) && i < 7; i++)
    {
        cJSON *item = cJSON_GetArrayItem(weather_array, i);
        strcpy(cfg->weather_info[i].textDay, cJSON_GetObjectItem(item, "textDay")->valuestring);
        strcpy(cfg->weather_info[i].iconDay, cJSON_GetObjectItem(item, "iconDay")->valuestring);
        cfg->weather_info[i].tempMax = cJSON_GetObjectItem(item, "tempMax")->valueint;
        cfg->weather_info[i].tempMin = cJSON_GetObjectItem(item, "tempMin")->valueint;
    }

    cfg->check = CFGFILE_CHECK;
}


int32_t write_ele_ds_cfg(ele_ds_cfg_t *cfg)
{
    cJSON *json = ele_ds_cfg_to_json(cfg);
    char *json_str = cJSON_Print(json);

    int fd = open(CONFIG_FILE_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0);
    if (fd < 0)
    {
        LOG_E("open %s failed", CONFIG_FILE_PATH);
        cJSON_Delete(json);
        free(json_str);
        return -1;
    }

    write(fd, json_str, strlen(json_str));
    close(fd);

    cJSON_Delete(json);
    free(json_str);
    return 0;
}

int32_t read_ele_ds_cfg(ele_ds_cfg_t *cfg)
{
    int fd = open(CONFIG_FILE_PATH, O_RDONLY);
    if (fd < 0)
    {
        LOG_E("open %s failed", CONFIG_FILE_PATH);
        return -1;
    }
    int32_t size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    char *buf = rt_calloc(1, size + 1);
    int len = read(fd, buf, size);
    close(fd);
    if (len <= 0)
    {
        LOG_E("read %s failed", CONFIG_FILE_PATH);
        return -2;
    }

    cJSON *json = cJSON_Parse(buf);
    if (!json)
    {
        LOG_E("parse json failed");
        return -3;
    }

    json_to_ele_ds_cfg(cfg, json);
    cJSON_Delete(json);
    free(buf);
    return 0;
}


/**
 * @description: 初始化系统文件, 如果 SYSFILE_PATH 不存在, 则初始化基本文件
 * @param {ele_ds_cfg_t} *cfg
 * @return {*}
 */
static int32_t init_cfgfile(ele_ds_cfg_t *cfg)
{
    init_ele_ds_cfg(cfg);
    if (write_ele_ds_cfg(cfg) != 0)
    {
        LOG_E("write config file failed");
        return -1;
    }
    return 0;
}

/**
 * @description: 打印配置文件信息
 * @param {ele_ds_cfg_t} *cfg
 * @return {*}
 */
void ele_ds_cfg_print(ele_ds_cfg_t *cfg)
{
#if 1 
    rt_kprintf("config file info:\n");
    rt_kprintf("version: %08u\n", cfg->clientcfg.version);
    rt_kprintf("cityid: %d\n", cfg->clientcfg.cityid);
    rt_kprintf("wifi_ssid: %s\n", cfg->wifi_ssid);
    rt_kprintf("wifi_passwd: %s\n", cfg->wifi_passwd);
    rt_kprintf("server_addr: %s\n", cfg->server_addr);
    rt_kprintf("server_port: %d\n", cfg->server_port);
    rt_kprintf("weather info:\n");
    for (int32_t i = 0; i < 7; i++)
    {
        rt_kprintf("weather[%d]: %s\t", i, cfg->weather_info[i].textDay);
        rt_kprintf("tempMax: %d tempMin: %d\n", cfg->weather_info[i].tempMax, cfg->weather_info[i].tempMin);
    }
#else
    (void)cfg;
    cJSON *json = ele_ds_cfg_to_json(cfg);
    char *json_str = cJSON_Print(json);
    rt_kprintf("%s\n", json_str);
    cJSON_Delete(json);
#endif
}
void ele_ds_cfg_print_cmd(int argc, char **argv)
{
    ele_ds_cfg_print(&g_ele_ds->device_cfg);
}
MSH_CMD_EXPORT_ALIAS(ele_ds_cfg_print_cmd, showcfg, print config file info);

/**
 * @description: 初始化系统文件, 如果 SYSFILE_PATH 不存在, 则初始化基本文件
 * @return {int32_t} 返回0表示成功, 返回-1表示失败
 */
static int32_t sysfile_init(void)
{
    // 检查文件是否存在
    if (access(CONFIG_FILE_PATH, R_OK) == -1)
    {
        LOG_D("filesystem need init, mkdir %s", SYSFILE_PATH);
        mkdir(SYSFILE_PATH, 0);
        init_cfgfile(&g_ele_ds->device_cfg);
    }
    else
    {
#if 0 // 改为json
        int fd = open(CONFIG_FILE_PATH, O_RDONLY);
        if (fd < 0)
        {
            LOG_E("open %s failed", CONFIG_FILE_PATH);
            return -2;
        }
        read(fd, &g_ele_ds->device_cfg, sizeof(ele_ds_cfg_t));
        close(fd);
#if 0 // 使用json的话不需要校验了, 因为可以手动修改文件内容
        if (g_ele_ds->device_cfg.check != CFGFILE_CHECK)
        {
            LOG_E("config file check failed, need init");
            init_cfgfile(&g_ele_ds->device_cfg);
            return -3;
        }
#endif
        LOG_D("filesystem already init");
#else
        read_ele_ds_cfg(&g_ele_ds->device_cfg);
#endif
        // 输出配置信息
        ele_ds_cfg_print(&g_ele_ds->device_cfg);
    }
    return 0;
}

/**
 * @description: 挂载文件系统
 * @return {int} 挂载成功返回0, 失败返回-1
 */
int mnt_init(void)
{
    if (dfs_mount("norflash0", "/", "elm", 0, 0) == 0)  // "norflash0":挂载的设备名称, "/":挂载路径, 这里挂载到跟目录下
    {
        LOG_D("norflash0 mount successful!");
    }
    else
    {
        dfs_mkfs("elm", "norflash0");  // 如果是第一次挂载文件系统必须要先格式化
        if(dfs_mount("norflash0", "/", "elm", 0, 0) != 0)
        {
            LOG_D("norflash0 mount failed!");
        }
        else
        {
            LOG_D("norflash0 mkfs successful!");
            // 第一次挂载文件系统，需要初始化系统文件
        }
    }
    sysfile_init();

    return 0;
}


void cmd_test_write_speed(void)
{
    int32_t fd = open("/sysfile/testspeed", O_RDWR | O_CREAT);
    if (fd < 0)
    {
        LOG_E("open %s failed", "/sysfile/testspeed");
        return;
    }
    
    char *buf = rt_calloc(1, 1024);
    uint32_t pack_num = 1024;
    uint32_t start_time = rt_tick_get_millisecond();
    for (uint32_t i = 0; i < pack_num; i++)
    {
        write(fd, buf, 1024);
    }
    uint32_t end_time = rt_tick_get_millisecond();
    close(fd);
    free(buf);

    uint32_t total_bytes = 1024 * pack_num;  // 1MB
    uint32_t elapsed_ticks = end_time - start_time;
    uint32_t elapsed_ms = elapsed_ticks;
    
    if (elapsed_ms == 0) elapsed_ms = 1;  // 防止除零
    uint32_t speed_kb_s = (total_bytes / 1024) * 1000 / elapsed_ms;

    rt_kprintf("start_time = %d, end_time = %d, elapsed_ms = %d\nwrite speed: %d KB/s\n", start_time, end_time, elapsed_ms, speed_kb_s);
}
MSH_CMD_EXPORT_ALIAS(cmd_test_write_speed, testspeed, test write flash speed);


