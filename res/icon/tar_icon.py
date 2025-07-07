'''
Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
Date: 2025-04-20 13:02:46
LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
LastEditTime: 2025-04-21 15:15:13
FilePath: \ele_ds\res\icon\tar_icon.py
Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
'''
import os
import tarfile
from PIL import Image, ImageEnhance

from PIL import Image, ImageEnhance
import os

# 固定的 LVGL 图片头（CF_INDEXED_1BIT）
LVGL_HEADER = bytes.fromhex("07c0000603030302515151d3")

def convert_png_to_birn(png_path, bin_path, threshold=128):
    # 打开图片并转为灰度图
    img = Image.open(png_path).convert("L")

    # 增强对比度
    enhancer = ImageEnhance.Contrast(img)
    img = enhancer.enhance(2.0)

    width, height = img.size
    pixels = img.load()

    # 构建 1bit 图像数据
    bin_data = bytearray()
    for y in range(height):
        byte = 0
        bit_count = 0
        for x in range(width):
            gray = pixels[x, y]
            bit = 0 if gray < threshold else 1
            byte = (byte << 1) | bit
            bit_count += 1
            if bit_count == 8:
                bin_data.append(byte)
                byte = 0
                bit_count = 0
        if bit_count > 0:
            byte = byte << (8 - bit_count)
            bin_data.append(byte)

    # 合成完整文件内容：header + image data
    full_data = LVGL_HEADER + bin_data

    # 写入文件
    with open(bin_path, "wb") as f:
        f.write(full_data)

    # 输出调试信息
    flat_bits = [1 if pixels[x % width, x // width] < threshold else 0 for x in range(min(width * height, 32))]
    print(f"{os.path.basename(png_path)} First 32 pixels (bit): {flat_bits}")
    print(f"Converted {png_path} -> {bin_path}")


def pack_dir_to_tar(source_dir, output_tar):
    with tarfile.open(output_tar, "w") as tar:
        for root, dirs, files in os.walk(source_dir):
            for file in files:
                if file.endswith(".png"):
                    full_path = os.path.join(root, file)
                    bin_path = os.path.splitext(full_path)[0] + ".bin"
                    
                    # 转换 png -> bin
                    convert_png_to_bin(full_path, bin_path)

                    # 计算打包路径
                    arcname = os.path.relpath(bin_path, start=source_dir)
                    tar.add(bin_path, arcname=arcname)
                    print(f"Added {arcname} to tar")

                    # 删除 bin 文件
                    os.remove(bin_path)
                    print(f"Deleted temporary {bin_path}")

if __name__ == "__main__":
    # ⚠️ 修改下面两个路径
    source_directory = r"./tianqi_48"
    output_tar_file = r"./tianqi_48.tar"

    pack_dir_to_tar(source_directory, output_tar_file)
    print("Packing complete.")
