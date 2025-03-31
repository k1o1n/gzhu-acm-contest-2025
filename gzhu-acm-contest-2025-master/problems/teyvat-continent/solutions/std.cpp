#include <cstddef>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <vector>

using namespace std;
using i64 = int64_t;
using u64 = uint64_t;

template <class T>
constexpr bool chmax(T& x, T y) {
    if (y > x) {
        x = y;
        return true;
    }
    return false;
}

template <class T>
constexpr bool chmin(T& x, T y) {
    if (y < x) {
        x = y;
        return true;
    }
    return false;
}

struct RevocableDSU {
    vector<int>            par, siz;
    vector<pair<int, int>> stk;

    explicit RevocableDSU(int n) {
        par.resize(n + 1);
        siz.assign(n + 1, 1);
        iota(par.begin(), par.end(), 0);
        stk.clear();
    }

    int find(int x) {
        while (x != par[x]) {
            x = par[x];
        }
        return x;
    }

    bool same(int x, int y) { return find(x) == find(y); }

    bool merge(int x, int y, i64& sum) {
        x = find(x), y = find(y);
        if (x == y) {
            stk.emplace_back(-1, -1);
            return false;
        }
        if (siz[x] < siz[y]) {
            std::swap(x, y);
        }
        sum    += siz[x] - siz[y];
        siz[x] += siz[y];
        par[y]  = x;
        stk.emplace_back(x, y);
        return true;
    }

    int size(int x) { return siz[find(x)]; }

    size_t version() const { return stk.size(); }

    void rollback(i64& sum) {
        auto [x, y] = stk.back();
        stk.pop_back();
        if (x != -1 && y != -1) {
            siz[x] -= siz[y];
            sum    -= siz[x] - siz[y];
            par[y]  = y;
        }
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    
    int n, m, q, r;
    std::cin >> n >> m >> q >> r;

    std::vector<std::pair<int, int>> edges(m + 1);
    for (int i = 1; i <= m; i += 1) {
        auto& [u, v] = edges[i];
        std::cin >> u >> v;
    }

    i64              sum = 0;
    RevocableDSU     dsu(n);
    std::vector<i64> ans(q + 1);
    for (int i = 1; i <= q; i += 1) {
        int o, p;
        std::cin >> o >> p;
        if (o == 1) {
            auto [u, v] = edges[p];
            dsu.merge(u, v, sum);
        } else {
            auto [u, v] = edges[p];
            dsu.rollback(sum);
        }
        ans[i] = sum;
    }

    for (int i = 1; i <= r; i += 1) {
        int t;
        std::cin >> t;
        std::cout << ans[t] << '\n';
    }
    return 0;
}