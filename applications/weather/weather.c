/*
 * @Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @Date: 2025-05-19 15:47:44
 * @LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
 * @LastEditTime: 2025-05-19 15:47:56
 * @FilePath: \ele_ds\applications\weather\weather.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "weather.h"

const weather_code_map_t weather_map[] = {
    {100, WEATHER_QING},
    {101, WEATHER_DUOYUN},
    {102, WEATHER_DUOYUN},
    {103, WEATHER_DUOYUN},
    {104, WEATHER_YIN},
    {150, WEATHER_QING},
    {153, WEATHER_DUOYUN},
    {154, WEATHER_YIN},

    {300, WEATHER_ZHENYU},
    {301, WEATHER_ZHENYU},
    {302, WEATHER_LEIYU},
    {303, WEATHER_LEIBAO},
    {304, WEATHER_BAOYU},

    {305, WEATHER_XIAOYU},
    {306, WEATHER_ZHONGYU},
    {307, WEATHER_DAYU},
    {308, WEATHER_BAOYU},
    {309, WEATHER_XIAOYU},
    {310, WEATHER_BAOYU},
    {311, WEATHER_BAOYU},
    {312, WEATHER_BAOYU},
    {313, WEATHER_BINGJING},
    {314, WEATHER_XIAOYU},
    {315, WEATHER_ZHONGYU},
    {316, WEATHER_DAYU},
    {317, WEATHER_BAOYU},
    {318, WEATHER_BAOYU},
    {399, WEATHER_XIAOYU},
    {350, WEATHER_ZHENYU},
    {351, WEATHER_ZHENYU},

    {400, WEATHER_XIAOXUE},
    {401, WEATHER_ZHONGXUE},
    {402, WEATHER_DAXUE},
    {403, WEATHER_BAOXUE},
    {404, WEATHER_YUJIAXUE},
    {405, WEATHER_YUJIAXUE},
    {406, WEATHER_YUJIAXUE},
    {407, WEATHER_XIAOXUE},
    {408, WEATHER_XIAOXUE},
    {409, WEATHER_ZHONGXUE},
    {410, WEATHER_DAXUE},
    {499, WEATHER_XIAOXUE},
    {456, WEATHER_YUJIAXUE},
    {457, WEATHER_XIAOXUE},

    {500, WEATHER_QINGWU},
    {501, WEATHER_ZHONGWU},
    {502, WEATHER_MAI},
    {503, WEATHER_YANGSHA},
    {504, WEATHER_FUCHEN},
    {507, WEATHER_SHACHENBAO},
    {508, WEATHER_SHACHENBAO},
    {509, WEATHER_DAWU},
    {510, WEATHER_DAWU},
    {511, WEATHER_MAI},
    {512, WEATHER_MAI},
    {513, WEATHER_MAI},
    {514, WEATHER_DAWU},
    {515, WEATHER_DAWU},

    {900, WEATHER_RE},
    {901, WEATHER_LENG},
    {999, WEATHER_UNKNOWN},
};

weather_type_t get_weather_type(uint32_t code)
{
    for (size_t i = 0; i < ARRAY_SIZE(weather_map); i++)
    {
        if (weather_map[i].code == code)
            return weather_map[i].type;
    }
    return WEATHER_UNKNOWN;
}

