#include <cstdint>
#include <iostream>

using namespace std;
using i64 = int64_t;
using u64 = uint64_t;

int main() {
    i64 n;
    cin >> n;
    i64 ans = 0;
    for (auto i = 1; i < n; ++i) {
        i64 d, e;
        cin >> d >> e;
        ans += d * e;
    }
    cout << ans << endl;
    return 0;
}