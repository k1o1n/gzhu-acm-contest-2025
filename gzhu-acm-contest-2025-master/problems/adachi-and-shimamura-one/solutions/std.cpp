#include <cmath>
#include <cstdint>
#include <iostream>
#include <set>

using namespace std;
using i64 = int64_t;
using u64 = uint64_t;

const double eps = 1e-8;

int sgn(double x) {
    if (fabs(x) < eps) return 0;
    if (x < 0) return -1;
    else return 1;
}

int dcmp(double x, double y) {
    if (fabs(x - y) < eps) return 0;
    if (x > y) return 1;
    return -1;
}

inline double sqr(double x) { return x * x; }
struct Point {
    double x{}, y{};
    Point() = default;
    Point(double _x, double _y) {
        x = _x;
        y = _y;
    }
    void   input() { cin >> x >> y; }
    bool   operator==(Point b) const { return sgn(x - b.x) == 0 && sgn(y - b.y) == 0; }
    Point  operator-(const Point& b) const { return Point(x - b.x, y - b.y); }
    double operator^(const Point& b) const { return x * b.y - y * b.x; }
    double distance(Point p) { return hypot(x - p.x, y - p.y); }
    Point  operator+(const Point& b) const { return Point(x + b.x, y + b.y); }
};

struct Line {
    Point s, e;

    Line() = default;
    Line(Point _s, Point _e) {
        s = _s;
        e = _e;
    }

    void input() {
        s.input();
        e.input();
    }

    double length() { return s.distance(e); }
    int    relation(Point p) {
        int c = sgn((p - s) ^ (e - s));
        if (c < 0) return 1;
        else if (c > 0) return 2;
        else return 3;
    }

    Point crosspoint(Line v) {
        double a1 = (v.e - v.s) ^ (s - v.s);
        double a2 = (v.e - v.s) ^ (e - v.s);
        return Point((s.x * a2 - e.x * a1) / (a2 - a1), (s.y * a2 - e.y * a1) / (a2 - a1));
    }
    int linecrossseg(Line v) {
        int d1 = sgn((e - s) ^ (v.s - s));
        int d2 = sgn((e - s) ^ (v.e - s));
        if ((d1 ^ d2) == -2) return 2;
        return (d1 == 0 || d2 == 0);
    }
};

struct cmp {
    bool operator()(const pair<Point, int>& a, const pair<Point, int>& b) const {
        if (sgn(a.first.x - b.first.x) != 0) return a.first.x < b.first.x;
        else if (sgn(a.first.y - b.first.y) != 0) return a.first.y < b.first.y;
        return a.second > b.second;
    }
};

void solve() {
    set<pair<Point, int>, cmp> p;
    double                     H, W;
    Point                      ball, v;
    int                        n;
    cin >> H >> W;
    ball.input();
    v.input();
    cin >> n;
    for (int i = 0; i < n; i++) {
        Point o;
        o.input();
        p.insert({o, 0});
    }
    p.insert({ball, 0});

    Line l;
    Line table(Point(-H / 2, 0.0), Point(H / 2, 0.0));
    Line web(Point(0.0, 0.0), Point(0.0, W));
    auto getpoint = [&]() {
        l = Line(ball, ball + v);
        if (l.linecrossseg(table) != 0) {
            Point tablep = l.crosspoint(table);
            if (sgn(tablep.x) != 0 || sgn(tablep.y) != 0) {
                p.emplace(tablep, 1);
            }
        }
        if (l.linecrossseg(web) != 0) {
            p.emplace(l.crosspoint(web), 2);
        }
    };


    bool ok  = false;
    int  ans = 0;
    for (auto& k : p) {
        if (k.first == ball) {
            getpoint();
            ok = true;
        } else {
            if (!ok) continue;
            if (l.relation(k.first) == 3) {
                if (k.second == 0) v.y = -fabs(v.y);
                if (k.second == 1) {
                    v.y = fabs(v.y);
                    ans++;
                }
                if (k.second == 2) break;
                ball = k.first;
                getpoint();
            }
        }
    }
    cout << ans << '\n';
}

int main() {
    int t = 1;
    while (t--) solve();
    return 0;
}
