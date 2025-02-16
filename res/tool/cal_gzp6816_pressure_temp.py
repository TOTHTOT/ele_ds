'''
Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
Date: 2025-02-16 15:38:59
LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
LastEditTime: 2025-02-16 16:07:16
FilePath: \ele_ds\res\tool\cal_gzp6816_pressure_temp.py
Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
'''
def pressure_calculation(ad_value, ad_min, ad_max, pressure_min, pressure_max):
    """
    计算压力值

    :param ad_value:  电桥校准值（AD值）
    :param ad_min:  AD值的最小值
    :param ad_max:  AD值的最大值
    :param pressure_min:  压力的最小值
    :param pressure_max:  压力的最大值
    :return: 计算得到的实际压力值（单位：Kpa）
    """
    return (pressure_max - pressure_min) / (ad_max - ad_min) * (ad_value - ad_min) + 20

def temperature_calculation(ad_value, temperature_min, temperature_max, max_value=65535):
    """
    计算温度校准值

    :param ad_value:  温度校准值（AD值）
    :param temperature_min:  温度最小值（℃）
    :param temperature_max:  温度最大值（℃）
    :param max_value:  16bit无符号数的最大值
    :return: 计算得到的温度值（单位：℃）
    """
    # 计算百分比
    percentage = ad_value / max_value
    # 根据百分比换算得到温度
    return (temperature_max - temperature_min) * percentage + temperature_min

# 校准数据（模拟从串口读取的数据）
ad_values_bridge = [0x9B, 0xB0, 0xC5]  # 这是一个包含多个原始AD值的数组
ad_values_temperature = [0x56, 0xAA]  # 假设这是读取到的温度校准值

# 电桥的最小值和最大值
ad_min_bridge = 1677722
ad_max_bridge = 15099494

# 压力的最小值和最大值（单位：Kpa）
pressure_min = 30
pressure_max = 110

# 温度的最小值和最大值（单位：℃）
temperature_min = -40
temperature_max = 150

# 将原始的 AD 值数组转为十进制值
ad_value_bridge_decimal = int(''.join(f'{x:02x}' for x in ad_values_bridge), 16)
ad_value_temperature_decimal = int(''.join(f'{x:02x}' for x in ad_values_temperature), 16)

# 计算压力值
pressure = pressure_calculation(ad_value_bridge_decimal, ad_min_bridge, ad_max_bridge, pressure_min, pressure_max)

# 计算温度值
temperature = temperature_calculation(ad_value_temperature_decimal, temperature_min, temperature_max)

# 输出结果
print(f"实际压力值: {pressure:.2f} Kpa")
print(f"温度校准值: {temperature:.2f} ℃")
