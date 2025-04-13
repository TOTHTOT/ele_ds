'''
Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
Date: 2025-04-08 14:50:00
LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
LastEditTime: 2025-04-13 15:14:18
FilePath: \ele_ds\res\output_epdfb.py
Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
'''
# -*- coding: utf-8 -*-

from PIL import Image

# 配置图像尺寸
width = 264
height = 176
bytes_per_row = (width + 7) // 8  # 每行字节数
frame_size = bytes_per_row * height  # 每帧图像数据所需字节数

# 从文件中读取所有十六进制数据
with open("test_image.txt", "r") as f:
    hex_data = f.read()

# 转换为字节数组
hex_values = hex_data.strip().split()
byte_data = bytes(int(b, 16) for b in hex_values)

# 计算总帧数
total_frames = len(byte_data) // frame_size

if len(byte_data) % frame_size != 0:
    raise ValueError("数据总长度不是单帧大小的整数倍，请检查数据完整性", len(byte_data), frame_size)

print(f"检测到 {total_frames} 帧图像...")

# 逐帧处理
for frame_idx in range(total_frames):
    frame_bytes = byte_data[frame_idx * frame_size : (frame_idx + 1) * frame_size]
    img = Image.new("1", (width, height))
    
    for y in range(height):
        for x in range(width):
            byte_index = y * bytes_per_row + x // 8
            bit_index = 7 - (x % 8)
            byte = frame_bytes[byte_index]
            bit = (byte >> bit_index) & 1
            img.putpixel((x, y), 255 if bit else 0)
    
    # 保存图像
    output_filename = f"frame_{frame_idx}.png"
    img.save(output_filename)
    print(f"保存图像: {output_filename}")

print("全部图像处理完成！")
