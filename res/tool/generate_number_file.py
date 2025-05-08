'''
Author: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
Date: 2025-05-08 14:27:57
LastEditors: TOTHTOT 37585883+TOTHTOT@users.noreply.github.com
LastEditTime: 2025-05-08 14:29:40
FilePath: \ele_ds\res\tool\creat_test_updatefile.py
Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
'''
def generate_number_file(filepath, target_size_kb=500):
    line_template = "{:06d}\n"  # 每行6位数字 + 换行符
    line_size = len(line_template.format(0).encode("utf-8"))  # 每行字节数
    total_lines = (target_size_kb * 1024) // line_size

    with open(filepath, "w") as f:
        for i in range(1, total_lines + 1):
            f.write(line_template.format(i))

    print(f"Generated file: {filepath}, total lines: {total_lines}, approx size: {target_size_kb} KB")

if __name__ == "__main__":
    generate_number_file("data_seq.txt")
