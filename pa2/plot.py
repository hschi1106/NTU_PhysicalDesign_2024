import argparse
import matplotlib.pyplot as plt

def calculate_hpwl(net, blocks, terminals):
    min_x = float('inf')
    min_y = float('inf')
    max_x = float('-inf')
    max_y = float('-inf')
    
    for instance in net:
        if instance in blocks:
            x1, y1, x2, y2 = blocks[instance]
            midx = (x1+x2)/2
            midy = (y1+y2)/2
            min_x = min(min_x, midx)
            min_y = min(min_y, midy)
            max_x = max(max_x, midx)
            max_y = max(max_y, midy)
        elif instance in terminals:
            x, y = terminals[instance]
            min_x = min(min_x, x)
            min_y = min(min_y, y)
            max_x = max(max_x, x)
            max_y = max(max_y, y)
    
    return min_x, min_y, max_x, max_y

def main(block_file, nets_file, output_file):
    # 讀取block檔案
    with open(block_file, 'r') as file:
        block_lines = file.readlines()
    
    # 讀取nets檔案
    with open(nets_file, 'r') as file:
        nets_lines = file.readlines()

    # 讀取output檔案
    with open(output_file, 'r') as file:
        output_lines = file.readlines()

    # 讀取chipWidth和chipHeight
    chipWidth = int(output_lines[3].strip().split()[0])
    chipHeight = int(output_lines[3].strip().split()[1])
    
    # 讀取outline
    outlineWidth = int(block_lines[0].strip().split()[1])
    outlineHeight = int(block_lines[0].strip().split()[2])

    # 跳過前面幾行
    output_lines = output_lines[5:]

    # 儲存block的位置
    blocks = {}

    # 解析每一行，並將block的位置存入字典
    for line in output_lines:
        parts = line.strip().split()
        if len(parts) == 5:  # 假設只有block的行有5個部分
            name, x1, y1, x2, y2 = parts
            blocks[name] = (int(x1), int(y1), int(x2), int(y2))
            
    # 儲存terminal的位置
    terminals = {}

    # 解析terminal的位置
    for line in block_lines[4:]:
        parts = line.strip().split()
        if len(parts) == 4 and parts[1]=="terminal":
            name, _, x, y = parts
            terminals[name] = (int(x), int(y))
            chipWidth = max(int(x), chipWidth)
            chipHeight = max(int(y), chipHeight)
            
    nets_lines = nets_lines[1:]
    
    # 儲存nets，直接存儲連結的blocks和terminals
    nets = []
    current_net = []
    for line in nets_lines:
        line = line.strip()
        if line.startswith("NetDegree"):
            if current_net != []:
                nets.append(current_net)
                current_net = []
        else:
            current_net.append(line)
    nets.append(current_net)

    # 繪圖
    fig, ax = plt.subplots()

    # 繪製outline的外框
    outline_rect = plt.Rectangle((0, 0), outlineWidth, outlineHeight, linewidth=2, edgecolor='black', facecolor='none')
    ax.add_patch(outline_rect)
    
    # 繪製blocks
    for name, (x1, y1, x2, y2) in blocks.items():
        width = x2 - x1
        height = y2 - y1
        rect = plt.Rectangle((x1, y1), width, height, linewidth=1.5, edgecolor='r', facecolor='none')
        ax.add_patch(rect)
        ax.text(x1, y1, name, fontsize=8, va='bottom', ha='left', color='r')
        
    # 繪製terminals
    for name, (x, y) in terminals.items():
        ax.scatter(x, y, color='green')
        ax.text(x, y, name, fontsize=8, va='bottom', ha='left', color='green')
        
    # 繪製nets
    for net in nets:
        min_x, min_y, max_x, max_y = calculate_hpwl(net, blocks, terminals)
        
        # 繪製矩形的右邊和下邊
        plt.plot([min_x, max_x], [min_y, min_y], color='b', linestyle='--', linewidth=1)
        plt.plot([max_x, max_x], [min_y, max_y], color='b', linestyle='--', linewidth=1)

        
    ax.set_xlim(0, max(outlineWidth, chipWidth))
    ax.set_ylim(0, max(outlineHeight, chipHeight))
    ax.set_aspect('equal')
    plt.xlabel('X-coordinate')
    plt.ylabel('Y-coordinate')
    plt.title(f'Block and Terminal Positions for {block_file.split("/")[-1]}', y=1.05)
    plt.grid(False)
    plt.show()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Plot block positions from input and output files.')
    parser.add_argument('block_file', type=str, help='Path to the block input file')
    parser.add_argument('nets_file', type=str, help='Path to the nets input file')
    parser.add_argument('output_file', type=str, help='Path to the output file')
    
    args = parser.parse_args()
    main(args.block_file, args.nets_file, args.output_file)
