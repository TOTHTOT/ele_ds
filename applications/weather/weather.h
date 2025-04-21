#ifndef __WEATHER_H__
#define __WEATHER_H__


#include <stdio.h>
#include <stdint.h>

typedef enum
{
    WEATHER_BAOXUE,
    WEATHER_BAOYU,
    WEATHER_BINGBAO,
    WEATHER_BINGJING,
    WEATHER_BINGLI,
    WEATHER_DAXUE,
    WEATHER_DAYU,
    WEATHER_DAWU,
    WEATHER_DUOYUN,
    WEATHER_DUOYUN_YE,
    WEATHER_FUCHEN,
    WEATHER_LEIBAO,
    WEATHER_LEIYU,
    WEATHER_MAI,
    WEATHER_QING,
    WEATHER_QING_YE,
    WEATHER_QINGWU,
    WEATHER_SHACHENBAO,
    WEATHER_XIAOXUE,
    WEATHER_XIAOYU,
    WEATHER_XUEMI,
    WEATHER_YANGSHA,
    WEATHER_YIN,
    WEATHER_YUJIAXUE,
    WEATHER_ZHENYU,
    WEATHER_ZHONGWU,
    WEATHER_ZHONGXUE,
    WEATHER_ZHONGYU,

    WEATHER_TYPE_COUNT
} weather_type_t; // 天气类型

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
}weather_info_t; // 天气信息

#endif /* __WEATHER_H__ */
