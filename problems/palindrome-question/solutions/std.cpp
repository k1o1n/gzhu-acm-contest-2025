#include <cstdint>
#include <iostream>
#include <vector>

using namespace std;
using i64 = int64_t;
using u64 = uint64_t;

struct PAM {
    std::vector<int>         Last, len, fail;
    std::vector<vector<int>> fa;
    std::vector<vector<int>> son;
    std::vector<char>        s;
    int                      tot{}, last{}, cnt{};
    int                      length{};

    int node(int l) {
        tot++;
        len[tot] = l;
        return tot;
    }
    void clear() {
        last = cnt = 0;
        tot        = -1;
        s.resize(length + 7);
        Last.resize(length + 7);
        len.resize(length + 7);
        fail.resize(length + 7);
        son  = vector(length + 7, vector(26, 0));
        fa   = vector(length + 7, vector(23, -1));
        s[0] = '#';
        node(0);
        node(-1);
        fail[0] = 1;
    }
    int getfail(int x) {
        while (s[cnt - len[x] - 1] != s[cnt]) x = fail[x];
        return x;
    }
    void insert(char c) {
        s[++cnt] = c;
        int now  = getfail(last);
        if (!son[now][c - 'a']) {
            int x = node(len[now] + 2), y = son[getfail(fail[now])][c - 'a'];
            fail[x]           = y;
            fa[x][0]          = y;
            son[now][c - 'a'] = x;
            for (int i = 1; i <= 20; i++) {
                int half = fa[x][i - 1];
                if (half != -1) fa[x][i] = fa[half][i - 1];
            }
        }
        last = son[now][c - 'a'];
    }
    void build(string t) {
        length = t.size();
        clear();
        t = "#" + t;
        for (int i = 1; i <= length; i++) {
            insert(t[i]);
            Last[i] = last;
        }
    }
    int query(int l, int r) {
        int u = Last[r];
        if (len[u] <= r - l + 1) return len[u];
        for (int i = 20; i >= 0; i--) {
            int v = fa[u][i];
            if (v != -1 && len[v] > r - l + 1) u = v;
        }
        u = fa[u][0];
        return len[u];
    }
};

constexpr auto N = 1000010;
int            l[N], r[N];

void solve() {
    PAM    pam;
    int    n, q;
    string s;
    cin >> n >> q >> s;
    for (int i = 1; i <= q; i++) cin >> l[i] >> r[i];
    pam.build(s);
    for (int i = 1; i <= q; i++) {
        int res = pam.query(l[i], r[i]);
        cout << res << "\n";
    }
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(0);
    int T;
    T = 1;
    while (T--) solve();
}