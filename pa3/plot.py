import argparse
import matplotlib.pyplot as plt

def main(node_file, pl_file):
    # 讀取.node檔案
    with open(node_file, 'r') as file:
        node_lines = file.readlines()

    # 讀取.pl檔案
    with open(pl_file, 'r') as file:
        pl_lines = file.readlines()

    # 跳過前面幾行
    node_lines = node_lines[8:]
    pl_lines = pl_lines[2:]

    # 儲存node的大小
    nodes = {}

    # 解析每一行，並將node的大小存入字典
    for line in node_lines:
        parts = line.strip().split()
        if len(parts) == 3:  # 假設只有node的行有3個部分
            name, width, height = parts
            nodes[name] = (float(width), float(height))
            
    # 儲存node的位置
    location = {}
    
    # 解析每一行，並將node的位置存入字典        
    for line in pl_lines:
        parts = line.strip().split()
        if len(parts) == 5:
            name, x, y, _, _ = parts
            location[name] = (float(x), float(y))
    
    # 繪圖
    fig, ax = plt.subplots()
    
    # 設定範圍
    leftest_node_name = min(location.keys(), key=lambda x: location[x][0])
    rightest_node_name = max(location.keys(), key=lambda x: location[x][0])
    top_node_name = max(location.keys(), key=lambda x: location[x][1])
    bottom_node_name = min(location.keys(), key=lambda x: location[x][1])
    min_x = location[leftest_node_name][0]
    max_x = location[rightest_node_name][0] + nodes[rightest_node_name][0]
    min_y = location[bottom_node_name][1]
    max_y = location[top_node_name][1] + nodes[top_node_name][1]
    
    # 繪製blocks
    for name, (width, height) in nodes.items():
      x, y = location[name]
      rect = plt.Rectangle((x, y), width, height, linewidth=1.5, edgecolor='r', facecolor='none')
      ax.add_patch(rect)

    ax.set_aspect('equal')
    plt.xlabel('X-coordinate')
    plt.ylabel('Y-coordinate')
    plt.title(f'Block and Terminal Positions for {node_file.split("/")[-1]}', y=1.05)
    plt.grid(False)
    
    plt.xlim(-33330, 33396)
    plt.ylim(-33208, 33320)
    
    plt.show()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Plot block positions from input and output files.')
    parser.add_argument('node_file', type=str, help='Path to the block input file')
    parser.add_argument('pl_file', type=str, help='Path to the output file')
    
    args = parser.parse_args()
    main(args.node_file, args.pl_file)
