#include <cstdint>
#include <iostream>
#include <queue>
#include <vector>

using namespace std;
using i64 = int64_t;
using u64 = uint64_t;

constexpr int maxn = 1e9 + 7;
int           mx[10];

struct node {
    int  x, y;
    int  block, step;
    bool operator<(const node& a) const { return block > a.block || (block == a.block && step > a.step); }
};

struct dis {
    int         b = maxn, s = maxn;
    friend bool le(dis a, dis b) {
        if (a.b != b.b) {
            return a.b < b.b;
        }
        return a.s < b.s;
    }
};
int main() {
    i64 n, m, k;
    int tx, ty;
    cin >> n >> m >> k;
    vector<string> vt(n);
    for (auto& x : vt) cin >> x;

    mx[0] = k;
    for (int i = 0; i <= k; i++) {
        for (int j = 0; j <= k; j++) {
            if (i * i + j * j > k * k) {
                mx[i] = j - 1;
                break;
            }
        }
    }
    vector<vector<bool>> vis(n, vector(m, false));
    vector<vector<dis>>  dist(n, vector(m, dis()));

    priority_queue<node, vector<node>> q;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (vt[i][j] == 's') {
                q.push({i, j, 0, 0});
                dist[i][j] = {0, 0};
                vt[i][j]   = '1';
            } else if (vt[i][j] == 't') {
                tx = i, ty = j;
                vt[i][j] = '1';
            }
        }
    }
    while (!q.empty()) {
        i64 nx = q.top().x, ny = q.top().y;
        q.pop();
        if (vis[nx][ny]) {
            continue;
        }
        vis[nx][ny] = true;
        for (auto i = max(0ll, nx - k); i <= min(n - 1, nx + k); i++) {
            int d = mx[abs(i - nx)];
            for (auto j = max(0ll, ny - d); j <= min(m - 1, ny + d); j++) {
                if (i == nx && j == ny || vis[i][j]) {
                    continue;
                }
                int add = 1 - (vt[i][j] - '0');
                dis u   = {dist[nx][ny].b + add, dist[nx][ny].s + 1};
                if (le(u, dist[i][j])) {
                    dist[i][j] = u;
                    q.emplace(i, j, dist[nx][ny].b + add, dist[nx][ny].s + 1);
                }
            }
        }
    }
    cout << dist[tx][ty].b << " " << dist[tx][ty].s << "\n";
}
