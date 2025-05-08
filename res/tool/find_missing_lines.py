def find_missing_lines(std_file, target_file):
    # 读取标准文件中的所有序号
    with open(std_file, "r") as f:
        std_numbers = set(int(line.strip()) for line in f if line.strip().isdigit())

    # 读取目标文件中的所有序号
    with open(target_file, "r") as f:
        target_numbers = set(int(line.strip()) for line in f if line.strip().isdigit())

    # 找出缺失的行号
    missing = sorted(std_numbers - target_numbers)
    extra = sorted(target_numbers - std_numbers)

    print(f"标准总行数: {len(std_numbers)}")
    print(f"目标总行数: {len(target_numbers)}")
    print(f"缺失数量: {len(missing)}")
    print(f"多余数量: {len(extra)}")

    if missing:
        print("\n丢失行号（前20行）:")
        print(missing[:20])
    if extra:
        print("\n额外行号（前20行）:")
        print(extra[:20])

if __name__ == "__main__":
    # 替换成你的文件路径
    find_missing_lines("data_seq.txt", "received_data.txt")
