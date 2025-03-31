#include <cstdint>
#include <iostream>

using namespace std;
using i64 = int64_t;
using u64 = uint64_t;

constexpr int N = 100010, M = N * 4 * 30, K = 30;
struct Node {
    int l, r, sum;
};

int  a[N], idx, root[K];
Node tr[M];

void pushup(int u) { tr[u].sum = tr[tr[u].l].sum + tr[tr[u].r].sum; }

int build(int l, int r, int k) {
    if (l == r) {
        tr[idx] = {-1, -1, 0};
        if (a[l] == k) tr[idx].sum = 1;
        return idx++;
    }
    int u   = idx++;
    tr[u]   = {-1, -1, 0};
    int mid = (l + r) >> 1;
    tr[u].l = build(l, mid, k);
    tr[u].r = build(mid + 1, r, k);
    pushup(u);
    return u;
}

void modify(int l, int r, int u, int v, int L, int R) {
    int mid = (L + R) >> 1;
    if (l <= mid) {
        if (L >= l && mid <= r) swap(tr[u].l, tr[v].l);
        else modify(l, r, tr[u].l, tr[v].l, L, mid);
    }
    if (r > mid) {
        if (mid + 1 >= l && R <= r) swap(tr[u].r, tr[v].r);
        else modify(l, r, tr[u].r, tr[v].r, mid + 1, R);
    }
    pushup(u);
    pushup(v);
}

int query(int u, int l, int r, int L, int R) {
    if (L >= l && R <= r) return tr[u].sum;
    int mid = (L + R) >> 1;
    if (r <= mid) return query(tr[u].l, l, r, L, mid);
    if (l > mid) return query(tr[u].r, l, r, mid + 1, R);
    return query(tr[u].l, l, r, L, mid) + query(tr[u].r, l, r, mid + 1, R);
}

void solve() {
    int n, m;
    cin >> n >> m;
    for (int i = 1; i <= n; i++) cin >> a[i];
    for (int i = 1; i <= m; i++) root[i] = build(1, n, i);
    int q;
    cin >> q;
    while (q--) {
        int op;
        cin >> op;
        if (op == 1) {
            int l, r, x, y;
            cin >> l >> r >> x >> y;
            if (l == 1 && r == n) swap(root[x], root[y]);
            else modify(l, r, root[x], root[y], 1, n);
        } else {
            int l, r, k;
            cin >> l >> r >> k;
            cout << query(root[k], l, r, 1, n) << endl;
        }
    }
}

int main() {
    int t = 1;
    while (t--) solve();
}