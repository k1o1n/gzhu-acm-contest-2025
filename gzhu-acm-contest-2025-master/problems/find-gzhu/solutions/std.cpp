#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>

using namespace std;
using i64 = int64_t;
using u64 = uint64_t;

int main() {
    int n, m;
    cin >> n >> m;

    string s;
    s.reserve(n * m);
    for (int i = 0; i < n; ++i) {
        std::string row;
        cin >> row;
        if (i % 2 == 1) std::reverse(row.begin(), row.end());
        s += row;
    }

    string target = "GZHU";
    int64_t ans = 0;
    int ind = 0;
    for (char i : s) {
        if (i == target[ind]) {
            ind++;
            if (ind == 4) {
                ans++;
                ind = 0;
            }
        } else {
            ind = 0;
        }
    }
    cout << ans << endl;

}