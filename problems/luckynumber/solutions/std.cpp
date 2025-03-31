#include <cstdint>
#include <iostream>
#include <vector>

using namespace std;
using i64 = int64_t;
using u64 = uint64_t;

int main() {
    
    int         A = 1000000;
    vector<int> pre(A + 5, 0);
    vector<int> cnt(10, 0), temp(10, 0);

    // 枚举因子
    for (int i = 1; i <= A; i++) {
        if (pre[i]) continue;

        // 计算 i 数位上 0~9 出现的次数
        for (int o = 0; o < 10; o++) cnt[o] = 0;
        for (int o = i; o; o /= 10) cnt[o % 10]++;

        // 枚举 i 的倍数 j
        for (int j = 2 * i; j <= A; j += i) {
            if (pre[j]) continue;

            // 计算 j 数位上 0~9 出现的次数
            for (int o = 0; o < 10; o++) temp[o] = 0;
            for (int o = j; o; o /= 10) temp[o % 10]++;

            bool ok = true;
            for (int l = 1; l < 10; l++) {
                if (temp[l] != cnt[l]) ok = false;
            }

            if (ok && temp[0] >= cnt[0]) pre[j] = 1;
        }
    }

    // 前缀和
    for (int i = 1; i <= A; i++) pre[i] += pre[i - 1];

    int T, l, r;
    cin >> T;

    while (T--) {
        cin >> l >> r;
        cout << pre[r] - pre[l - 1] << '\n';
    }

    return 0;
}