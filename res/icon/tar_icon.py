'''
Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
Date: 2025-04-20 13:02:46
LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
LastEditTime: 2025-04-20 13:30:54
FilePath: \ele_ds\res\icon\tar_icon.py
Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
'''
import os
import tarfile

def pack_dir_to_tar(source_dir, output_tar):
    with tarfile.open(output_tar, "w") as tar:
        for root, dirs, files in os.walk(source_dir):
            for file in files:
                full_path = os.path.join(root, file)
                arcname = os.path.relpath(full_path, start=source_dir)
                tar.add(full_path, arcname=arcname)
                print(f"Added {arcname}")

if __name__ == "__main__":
    # ⚠️ 修改下面两个路径
    source_directory = r"./tianqi_48"
    output_tar_file = r"./tianqi_48.tar"

    pack_dir_to_tar(source_directory, output_tar_file)
    print("Packing complete.")
