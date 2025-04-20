'''
Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
Date: 2025-04-20 13:02:46
LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
LastEditTime: 2025-04-20 21:49:00
FilePath: \ele_ds\res\icon\tar_icon.py
Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
'''
import os
import tarfile
from PIL import Image, ImageEnhance

def convert_png_to_bin(png_path, bin_path, threshold=128):
    # 打开并转换为灰度图
    img = Image.open(png_path).convert("L")

    # 增强对比度（放大2倍）
    enhancer = ImageEnhance.Contrast(img)
    img = enhancer.enhance(2.0)

    # 灰度图转黑白（8-bit 黑白）
    bw_img = img.point(lambda p: 255 if p > threshold else 0)

    # 保存为 bin 文件：每像素一个字节
    with open(bin_path, "wb") as f:
        f.write(bw_img.tobytes())

    # 输出调试信息
    sample = list(bw_img.getdata())[:32]
    print(f"{os.path.basename(png_path)} First 32 pixels: {sample}")
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
