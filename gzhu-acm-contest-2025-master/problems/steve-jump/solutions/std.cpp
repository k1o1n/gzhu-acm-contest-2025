#include <iostream>
#include <vector>
using namespace std;
typedef long long ll;
ll sub(ll n, ll a, ll b)
{
    return n - n * a / b;
}
int main()
{
    int t;
    cin >> t;
    while (t--)
    {
        ll n, a, b;
        cin >> n >> a >> b;
        ll cnt = 0;
        while (n)
        {
            ll l = 0, r = n;
            ll s = sub(n, a, b);
            while (l + 1 < r)
            {
                ll mid = (l + r) / 2;
                ll s1 = sub(mid, a, b);
                if (s1 == s)
                {
                    r = mid;
                }
                else
                {
                    l = mid;
                }
            }
            ll d = (n - r) / s + 1;
            n -= d * s;
            cnt += d;
        }
        cout << cnt - 1 << "\n";
    }

}
