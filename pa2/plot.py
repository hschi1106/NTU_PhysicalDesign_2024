import matplotlib.pyplot as plt
from collections import defaultdict

# 讀取檔案，解析每個block的位置資訊
blocks = {}
with open("./output/ami33.output", "r") as file:
    for idx, line in enumerate(file):
        if idx < 5:  # 跳過前面的幾行
            continue
        parts = line.strip().split()
        block_name = parts[0]
        position = tuple(map(int, parts[1:]))
        blocks[block_name] = position

# 建立一个字典来跟踪每个位置的块
position_to_blocks = defaultdict(list)
for block, (x1, y1, x2, y2) in blocks.items():
    for x in range(x1, x2+1):
        for y in range(y1, y2+1):
            position_to_blocks[(x, y)].append(block)

# 繪製每個block的位置
for block, (x1, y1, x2, y2) in blocks.items():
    # 检查当前 block 区域是否与其他区域重叠
    overlapping_blocks = set()
    for x in range(x1, x2+1):
        for y in range(y1, y2+1):
            overlapping_blocks.update(position_to_blocks[(x, y)])

    # 移除当前 block 自身
    overlapping_blocks.remove(block)

    # 绘制外框，并填充当前 block 区域
    plt.plot([x1, x2, x2, x1, x1], [y1, y1, y2, y2, y1], color='black')  # 外框填充为黑色
    if overlapping_blocks:
        plt.fill([x1, x2, x2, x1, x1], [y1, y1, y2, y2, y1], alpha=0.3, color='red')  # 如果有重叠，以红色填充
    else:
        plt.fill([x1, x2, x2, x1, x1], [y1, y1, y2, y2, y1], alpha=0.3, color='blue')  # 如果没有重叠，以蓝色填充

    # 显示 block 名称
    plt.text((x1 + x2) / 2, (y1 + y2) / 2, block, ha='center', va='center', color='white')

plt.xlabel('X')
plt.ylabel('Y')
plt.title('Block Placement')
plt.gca().set_aspect('equal', adjustable='box')  # 固定比例
plt.show()
