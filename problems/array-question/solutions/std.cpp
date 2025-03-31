#include <array>
#include <cstdint>
#include <iostream>


using namespace std;
using i64 = int64_t;
using u64 = uint64_t;

constexpr i64 p = 998244353;

struct mtx {
    std::array<std::array<int, 5>, 5> a{};

    mtx() = default;
    void eye() {
        for (int i = 0; i < 5; ++i) {
            a[i][i] = 1;
        }
    }
    void set(int x) {
        eye();
        a[0][1] = a[3][4] = 1;
        a[2][3] = a[2][4] = x % p;
        a[0][2] = a[1][2] = 2 * x % p;
        a[0][3] = a[0][4] = a[1][3] = a[1][4] = 1ll * x * x % p;
    }
};
mtx operator*(mtx& x, mtx& y) {
    mtx res;
    for (int i = 0; i < 5; ++i) {
        for (int j = i; j < 5; ++j) {
            for (int k = 0; k < 5; ++k) {
                res.a[i][j] += 1ll * x.a[i][k] * y.a[k][j] % p;
                res.a[i][j] %= p;
            }
        }
    }
    return res;
}

int n, q, x, l, r;
mtx pre[500005], suf[500005];
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    cin >> n;
    for (int i = 1; i <= n; ++i) {
        cin >> x;
        pre[i].set(x);
        suf[i].set(x);
    }

    pre[0].eye();
    for (int i = 1; i <= n; ++i) {
        pre[i] = pre[i] * pre[i - 1];
    }

    suf[n + 1].eye();
    for (int i = n; i >= 1; --i) {
        suf[i] = suf[i + 1] * suf[i];
    }

    cin >> q;
    while (q--) {
        cin >> l >> r;
        cout << (suf[r + 1] * pre[l - 1]).a[0][4] << "\n";
    }
}