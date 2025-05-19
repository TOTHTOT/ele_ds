/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-04-21 17:56:41
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-19 15:22:18
 * @FilePath: \ele_ds\applications\weather\weather.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __WEATHER_H__
#define __WEATHER_H__

#include <stdio.h>
#include <stdint.h>

typedef enum
{
    WEATHER_QING,   // 晴
    WEATHER_DUOYUN, // 多云、少云、晴间多云
    WEATHER_YIN,    // 阴

    WEATHER_ZHENYU,   // 阵雨、强阵雨
    WEATHER_XIAOYU,   // 小雨、毛毛雨
    WEATHER_ZHONGYU,  // 中雨
    WEATHER_DAYU,     // 大雨
    WEATHER_BAOYU,    // 暴雨、大暴雨、特大暴雨、极端降雨
    WEATHER_LEIYU,    // 雷阵雨、强雷阵雨
    WEATHER_LEIBAO,   // 雷阵雨伴有冰雹
    WEATHER_YUJIAXUE, // 雨夹雪、雨雪、阵雨夹雪
    WEATHER_BINGJING, // 冻雨

    WEATHER_XIAOXUE,  // 小雪
    WEATHER_ZHONGXUE, // 中雪
    WEATHER_DAXUE,    // 大雪
    WEATHER_BAOXUE,   // 暴雪、大到暴雪
    WEATHER_ZHENXUE,  // 阵雪
    WEATHER_XUEMI,    // 雪（模糊/通用）

    WEATHER_QINGWU,  // 薄雾
    WEATHER_ZHONGWU, // 雾
    WEATHER_DAWU,    // 浓雾、大雾、强浓雾、特强浓雾
    WEATHER_MAI,     // 霾（中度、重度、严重）

    WEATHER_YANGSHA,    // 扬沙
    WEATHER_FUCHEN,     // 浮尘
    WEATHER_SHACHENBAO, // 沙尘暴、强沙尘暴

    WEATHER_RE,   // 热
    WEATHER_LENG, // 冷

    WEATHER_UNKNOWN, // 未知

    WEATHER_TYPE_COUNT // 类型总数
} weather_type_t;

typedef struct
{
    int code;
    weather_type_t type;
} weather_code_map_t;
extern const weather_code_map_t weather_map[];
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))


// 天气信息
typedef struct weather_info
{
    char fxDate[11];        // 日期 (如 "2025-03-20")
    char sunrise[6];        // 日出时间 (如 "06:19")
    char sunset[6];         // 日落时间 (如 "18:26")
    char moonrise[6];       // 月出时间 (允许为空)
    char moonset[6];        // 月落时间 (如 "08:53")
    char moonPhase[16];     // 月相描述 (如 "亏凸月")
    char moonPhaseIcon[4];  // 月相图标 ID (如 "805")
    int8_t tempMax;         // 最高温度 (如 "24")
    int8_t tempMin;         // 最低温度 (如 "10")
    char iconDay[4];        // 白天图标 ID (如 "100")
    char textDay[16];       // 白天天气描述 (如 "晴")
    char iconNight[4];      // 夜间图标 ID (如 "150")
    char textNight[16];     // 夜间天气描述 (如 "晴")
    uint16_t wind360Day;    // 白天风向角度 (如 "315")
    char windDirDay[16];    // 白天风向 (如 "西北风")
    char windScaleDay[8];   // 白天风力等级 (如 "1-3")
    uint8_t windSpeedDay;   // 白天风速 (如 "3")
    uint16_t wind360Night;  // 夜间风向角度 (如 "270")
    char windDirNight[16];  // 夜间风向 (如 "西风")
    char windScaleNight[8]; // 夜间风力等级 (如 "1-3")
    uint8_t windSpeedNight; // 夜间风速 (如 "3")
    uint8_t humidity;       // 湿度 (如 "21")
    float precip;           // 降水量 (如 "0.0")
    uint16_t pressure;      // 气压 (如 "1009")
    uint8_t vis;            // 能见度 (如 "25")
    uint8_t cloud;          // 云量 (如 "0")
    uint8_t uvIndex;        // 紫外线指数 (如 "5")
} weather_info_t;           // 天气信息

extern weather_type_t get_weather_type(uint32_t code);


#endif /* __WEATHER_H__ */
