#include <cmath>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <queue>
#include <vector>


using namespace std;
using i64 = int64_t;
using u64 = uint64_t;

const int N = 500 + 5, M = 1e9 + 7;

template <typename T>
struct Flow_ {

    const int n;
    const T   inf = numeric_limits<T>::max();

    struct Edge {
        int to;
        T   w;
        Edge(int to, T w) : to(to), w(w) {}
    };
    
    vector<Edge>        ver;
    vector<vector<int>> h;
    vector<int>         cur, d;

    explicit Flow_(int n) : n(n + 1), h(n + 1) {}
    
    void add(int u, int v, T c) {
        h[u].push_back(ver.size());
        ver.emplace_back(v, c);
        h[v].push_back(ver.size());
        ver.emplace_back(u, 0);
    }

    bool bfs(int s, int t) {
        d.assign(n, -1);
        d[s] = 0;
        queue<int> q;
        q.push(s);

        while (!q.empty()) {
            auto x = q.front();
            q.pop();

            for (auto it : h[x]) {
                auto [y, w] = ver[it];

                if (w && d[y] == -1) {
                    d[y] = d[x] + 1;

                    if (y == t) return true;

                    q.push(y);
                }
            }
        }
        return false;
    }

    T dfs(int u, int t, T f) {
        if (u == t) return f;

        auto r = f;

        for (int& i = cur[u]; i < h[u].size(); i++) {
            auto j        = h[u][i];
            auto& [v, c]  = ver[j];
            auto& [u, rc] = ver[j ^ 1];

            if (c && d[v] == d[u] + 1) {
                auto a  = dfs(v, t, std::min(r, c));
                c      -= a;
                rc     += a;
                r      -= a;

                if (!r) return f;
            }
        }

        return f - r;
    }

    T work(int s, int t) {
        T ans = 0;
        while (bfs(s, t)) {
            cur.assign(n, 0);
            ans += dfs(s, t, inf);
        }
        return ans;
    }
};
using Flow = Flow_<int>;

int  a[N];
void solve() {
    int n, m;
    cin >> n >> m;
    int sum = 0;

    for (int i = 1; i <= n; i++) {
        cin >> a[i];
        sum += a[i];
    }

    Flow flow(2 * n + 5);
    int  S = 2 * n + 1, T = 2 * n + 2;

    for (int i = 1; i <= n; i++) {
        if (!a[i]) continue;

        flow.add(S, i, a[i]);
        flow.add(n + i, T, a[i]);
    }

    for (int i = 1; i <= n; i++) {
        for (int j = 1; j < i; j++) {
            if (gcd(i, j) != 1 && a[j]) {
                flow.add(i, n + j, sum);
            }
        }
    }

    sum -= flow.work(S, T);
    cout << (sum <= m ? "Adachi" : "Shimamura");
}

int main() {
    int t = 1;
    while (t--) solve();
}