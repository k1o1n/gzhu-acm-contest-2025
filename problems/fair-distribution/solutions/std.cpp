#include <cstdint>
#include <iostream>
#include <queue>
#include <vector>

using namespace std;
using i64 = int64_t;
using u64 = uint64_t;

void solve() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    
    int n, m;
    cin >> n >> m;
    vector<vector<int>> v(n, vector(m, 0));

    vector<int> state(m);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++) cin >> v[i][j];

    vector<priority_queue<pair<int, int>>> q(n);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++) q[i].emplace(v[i][j], j);

    vector<vector<int>> ans(n);
    int                 T = m, idx = 0;
    while (T--) {
        while (state[q[idx].top().second]) q[idx].pop();
        auto t = q[idx].top();
        q[idx].pop();
        state[t.second] = 1;
        ans[idx].push_back(t.second);
        idx = (idx + 1) % n;
    }

    for (int i = 0; i < n; i++) {
        cout << ans[i].size() << ' ';
        for (auto t : ans[i]) cout << t + 1 << ' ';
        cout << "\n";
    }
}

int main() {
    int t;
    t = 1;
    while (t--) solve();
}