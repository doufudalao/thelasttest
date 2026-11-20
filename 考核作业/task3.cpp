
#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <cstdlib>
#include <climits>

using namespace std;

// ==================== 全局数据 ====================

int R, C;
vector<string> grid;

// 上、下、左、右四个移动方向
const int dr[4] = {-1, 1, 0, 0};
const int dc[4] = {0, 0, -1, 1};


// ==================== 地图基础判断 ====================

// 判断一个位置是否在地图范围内且不是障碍物
bool isValid(int r, int c) {
    return r >= 0 && r < R &&
           c >= 0 && c < C &&
           grid[r][c] != '#';
}

// 获取进入某个格子的代价
int getCost(int r, int c) {
    char ch = grid[r][c];

    if (ch >= '2' && ch <= '9') {
        return ch - '0';
    }

    return 1;
}


// ==================== DFS 1：统计起点可达格子 ====================

void dfsReachable(int r, int c,
                  vector<vector<bool>>& visited,
                  int& count) {

    // 标记当前格子
    visited[r][c] = true;
    count++;

    // 搜索四个方向
    for (int i = 0; i < 4; i++) {

        int nr = r + dr[i];
        int nc = c + dc[i];

        if (isValid(nr, nc) && !visited[nr][nc]) {
            dfsReachable(nr, nc, visited, count);
        }
    }
}


// ==================== DFS 2：统计连通区域 ====================

void dfsRegion(int r, int c,
               vector<vector<bool>>& visited) {

    visited[r][c] = true;

    for (int i = 0; i < 4; i++) {

        int nr = r + dr[i];
        int nc = c + dc[i];

        if (isValid(nr, nc) && !visited[nr][nc]) {
            dfsRegion(nr, nc, visited);
        }
    }
}


// ==================== A*：节点定义 ====================

struct Node {
    int r, c;   // 当前坐标
    int g;      // 起点到当前格子的实际代价
    int f;      // f = g + h
};

// 自定义比较器：f 值越小，优先级越高
struct Compare {
    bool operator()(const Node& a, const Node& b) const {
        return a.f > b.f;
    }
};


// ==================== A*：曼哈顿距离 ====================

int heuristic(int r, int c, int gr, int gc) {
    return abs(r - gr) + abs(c - gc);
}


// ==================== A*：最小代价搜索 ====================

int aStar(int sr, int sc, int gr, int gc) {

    const int INF = INT_MAX;

    // 记录起点到每个格子的当前最小代价
    vector<vector<int>> dist(
        R, vector<int>(C, INF)
    );

    // 创建小顶堆
    priority_queue<Node, vector<Node>, Compare> pq;

    // 起点代价为 0
    dist[sr][sc] = 0;

    // 将起点加入优先队列
    pq.push({
        sr,
        sc,
        0,
        heuristic(sr, sc, gr, gc)
    });

    while (!pq.empty()) {

        // 取出 f 值最小的节点
        Node cur = pq.top();
        pq.pop();

        int r = cur.r;
        int c = cur.c;
        int g = cur.g;

        // 跳过已经过期的节点
        if (g != dist[r][c]) {
            continue;
        }

        // 到达终点，返回最小代价
        if (r == gr && c == gc) {
            return g;
        }

        // 遍历四个方向
        for (int i = 0; i < 4; i++) {

            int nr = r + dr[i];
            int nc = c + dc[i];

            // 越界或障碍物不能进入
            if (!isValid(nr, nc)) {
                continue;
            }

            // 计算进入新格子后的实际代价
            int newG = g + getCost(nr, nc);

            // 只有找到更优路线才更新
            if (newG < dist[nr][nc]) {

                dist[nr][nc] = newG;

                int h = heuristic(nr, nc, gr, gc);

                // 加入新的搜索节点
                pq.push({
                    nr,
                    nc,
                    newG,
                    newG + h
                });
            }
        }
    }

    // 队列为空仍未到达终点
    return -1;
}


// ==================== 主函数 ====================

int main() {

    cin >> R >> C;

    grid.resize(R);

    int sr = -1, sc = -1;
    int gr = -1, gc = -1;

    // 读取地图并寻找 S 和 G
    for (int i = 0; i < R; i++) {

        cin >> grid[i];

        for (int j = 0; j < C; j++) {

            if (grid[i][j] == 'S') {
                sr = i;
                sc = j;
            }

            if (grid[i][j] == 'G') {
                gr = i;
                gc = j;
            }
        }
    }

    // ==================== 第一项：reachable_cells ====================

    vector<vector<bool>> visited(
        R, vector<bool>(C, false)
    );

    int reachableCells = 0;

    dfsReachable(sr, sc, visited, reachableCells);


    // ==================== 第二项：regions ====================

    vector<vector<bool>> regionVisited(
        R, vector<bool>(C, false)
    );

    int regions = 0;

    for (int i = 0; i < R; i++) {

        for (int j = 0; j < C; j++) {

            if (isValid(i, j) && !regionVisited[i][j]) {

                regions++;

                dfsRegion(i, j, regionVisited);
            }
        }
    }


    // ==================== 第三项：cost ====================

    int cost = aStar(sr, sc, gr, gc);


    // ==================== 输出结果 ====================

    cout << "reachable_cells: " << reachableCells << '\n';
    cout << "regions: " << regions << '\n';
    cout << "cost: " << cost << '\n';

    return 0;
}
