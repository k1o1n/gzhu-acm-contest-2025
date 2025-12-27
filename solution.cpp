#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <iomanip>
#include <array>
#include <map>
#include <set>
#include <queue>
#include <random>
#include <numeric>
#include <functional>
#include <bitset>
#include <cassert>
#include <climits>
#include <cstdlib>
#include <tuple>
#include <utility>
#include <stack>
#include <deque>

using namespace std;

#pragma GCC optimize("O3")
#pragma GCC target("avx2")

typedef long long ll;
typedef unsigned long long ull;
typedef pair<int,int> pii;
typedef pair<ll,ll> pll;
typedef vector<int> vi;
typedef vector<ll> vll;
typedef vector<pii> vpii;
typedef vector<pll> vpll;

const int MAXN = 40;
const int MAXS = 40;
const int MAXL = 70;
const int MAXM = 300;
const int MAXK = 5;
const int MAXR = 3000;
const int MAXP = 20;
const int MAXQ = 10000;
const ll INF = 1e18;
const int IINF = 1e9;

int N, S, L, M, K, P, R;
int S_per_P;
int M_per_P;
const int MAX_QUERIES = 5;

struct FlowDemand {
    int id;
    int group_a;
    int leaf_a;
    int group_b;
    int leaf_b;
    int assigned_spine_a;
    int assigned_link_a;
    int assigned_oxc;
    int assigned_spine_b;
    int assigned_link_b;
    bool assigned;
};

int oxc_adjacency[MAXM][MAXR];
int load_leaf_spine[MAXN][MAXL][MAXS];
int load_spine_oxc[MAXN][MAXS][MAXM][MAXK];
int load_oxc_port[MAXM][MAXR];

struct PortMapping {
    int group;
    int local_spine;
    int link;
};

PortMapping port_to_info[MAXR];
int spine_to_plane[MAXS];
int oxc_to_plane[MAXM];

inline int get_plane_of_spine(int spine_idx) {
    return spine_idx / S_per_P;
}

inline int get_plane_of_oxc(int oxc_idx) {
    return oxc_idx / M_per_P;
}

inline int get_port_id(int group, int local_spine, int link) {
    return group * S_per_P * K + local_spine * K + link;
}

inline PortMapping get_port_mapping(int port_id) {
    PortMapping pm;
    pm.group = port_id / (S_per_P * K);
    int remainder = port_id % (S_per_P * K);
    pm.local_spine = remainder / K;
    pm.link = remainder % K;
    return pm;
}

inline int local_spine_to_global(int local_spine, int plane) {
    return plane * S_per_P + local_spine;
}

inline int global_spine_to_local(int global_spine) {
    return global_spine % S_per_P;
}

void initialize_mappings() {
    for (int s = 0; s < S; s++) {
        spine_to_plane[s] = s / S_per_P;
    }
    for (int m = 0; m < M; m++) {
        oxc_to_plane[m] = m / M_per_P;
    }
    for (int p = 0; p < R; p++) {
        port_to_info[p] = get_port_mapping(p);
    }
}

void reset_all_loads() {
    memset(load_leaf_spine, 0, sizeof(load_leaf_spine));
    memset(load_spine_oxc, 0, sizeof(load_spine_oxc));
    memset(load_oxc_port, 0, sizeof(load_oxc_port));
}

void initialize_oxc_state() {
    for (int m = 0; m < M; m++) {
        for (int r = 0; r < R; r++) {
            oxc_adjacency[m][r] = -1;
        }
    }
}

bool is_port_free(int oxc, int port) {
    if (port < 0 || port >= R) return false;
    return oxc_adjacency[oxc][port] == -1;
}

void establish_connection(int oxc, int port_a, int port_b) {
    if (port_a < 0 || port_a >= R || port_b < 0 || port_b >= R) return;
    int old_a = oxc_adjacency[oxc][port_a];
    int old_b = oxc_adjacency[oxc][port_b];
    if (old_a != -1 && old_a != port_b) {
        oxc_adjacency[oxc][old_a] = -1;
    }
    if (old_b != -1 && old_b != port_a) {
        oxc_adjacency[oxc][old_b] = -1;
    }
    oxc_adjacency[oxc][port_a] = port_b;
    oxc_adjacency[oxc][port_b] = port_a;
}

void remove_connection(int oxc, int port_a, int port_b) {
    if (port_a < 0 || port_a >= R || port_b < 0 || port_b >= R) return;
    oxc_adjacency[oxc][port_a] = -1;
    oxc_adjacency[oxc][port_b] = -1;
}

int find_existing_connection(int oxc, int group_a, int group_b, int& found_port_a, int& found_port_b) {
    for (int local_s_a = 0; local_s_a < S_per_P; local_s_a++) {
        for (int k_a = 0; k_a < K; k_a++) {
            int port_a = get_port_id(group_a, local_s_a, k_a);
            if (port_a >= R) continue;
            int connected_port = oxc_adjacency[oxc][port_a];
            if (connected_port == -1) continue;
            PortMapping pm = get_port_mapping(connected_port);
            if (pm.group == group_b) {
                found_port_a = port_a;
                found_port_b = connected_port;
                return 1;
            }
        }
    }
    return 0;
}

vector<pii> find_all_connections(int oxc, int group_a, int group_b) {
    vector<pii> result;
    for (int local_s_a = 0; local_s_a < S_per_P; local_s_a++) {
        for (int k_a = 0; k_a < K; k_a++) {
            int port_a = get_port_id(group_a, local_s_a, k_a);
            if (port_a >= R) continue;
            int connected_port = oxc_adjacency[oxc][port_a];
            if (connected_port == -1) continue;
            PortMapping pm = get_port_mapping(connected_port);
            if (pm.group == group_b) {
                result.push_back({port_a, connected_port});
            }
        }
    }
    return result;
}

vector<int> find_free_ports_for_group(int oxc, int group) {
    vector<int> result;
    for (int local_s = 0; local_s < S_per_P; local_s++) {
        for (int k = 0; k < K; k++) {
            int port = get_port_id(group, local_s, k);
            if (port >= R) continue;
            if (oxc_adjacency[oxc][port] == -1) {
                result.push_back(port);
            }
        }
    }
    return result;
}

ll calculate_route_cost(int group_a, int leaf_a, int spine_a, int link_a, int oxc, int group_b, int leaf_b, int spine_b, int link_b) {
    ll cost = 0;
    cost += load_leaf_spine[group_a][leaf_a][spine_a];
    cost += load_spine_oxc[group_a][spine_a][oxc][link_a];
    int port_a = get_port_id(group_a, global_spine_to_local(spine_a), link_a);
    cost += load_oxc_port[oxc][port_a];
    int port_b = get_port_id(group_b, global_spine_to_local(spine_b), link_b);
    cost += load_oxc_port[oxc][port_b];
    cost += load_spine_oxc[group_b][spine_b][oxc][link_b];
    cost += load_leaf_spine[group_b][leaf_b][spine_b];
    return cost;
}

ll calculate_max_load_after(int group_a, int leaf_a, int spine_a, int link_a, int oxc, int group_b, int leaf_b, int spine_b, int link_b) {
    ll max_load = 0;
    max_load = max(max_load, (ll)(load_leaf_spine[group_a][leaf_a][spine_a] + 1));
    max_load = max(max_load, (ll)(load_spine_oxc[group_a][spine_a][oxc][link_a] + 1));
    int port_a = get_port_id(group_a, global_spine_to_local(spine_a), link_a);
    max_load = max(max_load, (ll)(load_oxc_port[oxc][port_a] + 1));
    int port_b = get_port_id(group_b, global_spine_to_local(spine_b), link_b);
    max_load = max(max_load, (ll)(load_oxc_port[oxc][port_b] + 1));
    max_load = max(max_load, (ll)(load_spine_oxc[group_b][spine_b][oxc][link_b] + 1));
    max_load = max(max_load, (ll)(load_leaf_spine[group_b][leaf_b][spine_b] + 1));
    return max_load;
}

void apply_route(int group_a, int leaf_a, int spine_a, int link_a, int oxc, int group_b, int leaf_b, int spine_b, int link_b) {
    load_leaf_spine[group_a][leaf_a][spine_a]++;
    load_spine_oxc[group_a][spine_a][oxc][link_a]++;
    int port_a = get_port_id(group_a, global_spine_to_local(spine_a), link_a);
    load_oxc_port[oxc][port_a]++;
    int port_b = get_port_id(group_b, global_spine_to_local(spine_b), link_b);
    load_oxc_port[oxc][port_b]++;
    load_spine_oxc[group_b][spine_b][oxc][link_b]++;
    load_leaf_spine[group_b][leaf_b][spine_b]++;
}

struct RouteCandidate {
    int spine_a;
    int link_a;
    int oxc;
    int spine_b;
    int link_b;
    ll max_load;
    ll total_cost;
    bool new_connection;
    int port_a;
    int port_b;
};

bool compare_candidates(const RouteCandidate& a, const RouteCandidate& b) {
    if (a.max_load != b.max_load) return a.max_load < b.max_load;
    if (a.new_connection != b.new_connection) return !a.new_connection;
    return a.total_cost < b.total_cost;
}

RouteCandidate find_best_route_for_flow(int group_a, int leaf_a, int group_b, int leaf_b) {
    vector<RouteCandidate> candidates;
    
    for (int plane = 0; plane < P; plane++) {
        int oxc_start = plane * M_per_P;
        int oxc_end = oxc_start + M_per_P;
        int spine_start = plane * S_per_P;
        int spine_end = spine_start + S_per_P;
        
        for (int oxc = oxc_start; oxc < oxc_end; oxc++) {
            vector<pii> existing_conns = find_all_connections(oxc, group_a, group_b);
            
            for (const auto& conn : existing_conns) {
                int port_a = conn.first;
                int port_b = conn.second;
                PortMapping pm_a = get_port_mapping(port_a);
                PortMapping pm_b = get_port_mapping(port_b);
                int spine_a = local_spine_to_global(pm_a.local_spine, plane);
                int spine_b = local_spine_to_global(pm_b.local_spine, plane);
                int link_a = pm_a.link;
                int link_b = pm_b.link;
                
                RouteCandidate cand;
                cand.spine_a = spine_a;
                cand.link_a = link_a;
                cand.oxc = oxc;
                cand.spine_b = spine_b;
                cand.link_b = link_b;
                cand.max_load = calculate_max_load_after(group_a, leaf_a, spine_a, link_a, oxc, group_b, leaf_b, spine_b, link_b);
                cand.total_cost = calculate_route_cost(group_a, leaf_a, spine_a, link_a, oxc, group_b, leaf_b, spine_b, link_b);
                cand.new_connection = false;
                cand.port_a = port_a;
                cand.port_b = port_b;
                candidates.push_back(cand);
            }
            
            vector<int> free_ports_a = find_free_ports_for_group(oxc, group_a);
            vector<int> free_ports_b = find_free_ports_for_group(oxc, group_b);
            
            if (!free_ports_a.empty() && !free_ports_b.empty()) {
                int best_port_a = -1, best_port_b = -1;
                ll best_max_load = INF;
                
                for (int port_a : free_ports_a) {
                    PortMapping pm_a = get_port_mapping(port_a);
                    int spine_a = local_spine_to_global(pm_a.local_spine, plane);
                    int link_a = pm_a.link;
                    
                    for (int port_b : free_ports_b) {
                        PortMapping pm_b = get_port_mapping(port_b);
                        int spine_b = local_spine_to_global(pm_b.local_spine, plane);
                        int link_b = pm_b.link;
                        
                        ll max_load = calculate_max_load_after(group_a, leaf_a, spine_a, link_a, oxc, group_b, leaf_b, spine_b, link_b);
                        if (max_load < best_max_load) {
                            best_max_load = max_load;
                            best_port_a = port_a;
                            best_port_b = port_b;
                        }
                    }
                }
                
                if (best_port_a != -1 && best_port_b != -1) {
                    PortMapping pm_a = get_port_mapping(best_port_a);
                    PortMapping pm_b = get_port_mapping(best_port_b);
                    int spine_a = local_spine_to_global(pm_a.local_spine, plane);
                    int spine_b = local_spine_to_global(pm_b.local_spine, plane);
                    int link_a = pm_a.link;
                    int link_b = pm_b.link;
                    
                    RouteCandidate cand;
                    cand.spine_a = spine_a;
                    cand.link_a = link_a;
                    cand.oxc = oxc;
                    cand.spine_b = spine_b;
                    cand.link_b = link_b;
                    cand.max_load = best_max_load;
                    cand.total_cost = calculate_route_cost(group_a, leaf_a, spine_a, link_a, oxc, group_b, leaf_b, spine_b, link_b);
                    cand.new_connection = true;
                    cand.port_a = best_port_a;
                    cand.port_b = best_port_b;
                    candidates.push_back(cand);
                }
            }
        }
    }
    
    if (candidates.empty()) {
        RouteCandidate fallback;
        fallback.spine_a = 0;
        fallback.link_a = 0;
        fallback.oxc = 0;
        fallback.spine_b = 0;
        fallback.link_b = 0;
        fallback.max_load = INF;
        fallback.total_cost = INF;
        fallback.new_connection = true;
        fallback.port_a = get_port_id(group_a, 0, 0);
        fallback.port_b = get_port_id(group_b, 0, 0);
        return fallback;
    }
    
    sort(candidates.begin(), candidates.end(), compare_candidates);
    return candidates[0];
}

void process_flow(FlowDemand& flow) {
    RouteCandidate best = find_best_route_for_flow(flow.group_a, flow.leaf_a, flow.group_b, flow.leaf_b);
    
    flow.assigned_spine_a = best.spine_a;
    flow.assigned_link_a = best.link_a;
    flow.assigned_oxc = best.oxc;
    flow.assigned_spine_b = best.spine_b;
    flow.assigned_link_b = best.link_b;
    flow.assigned = true;
    
    if (best.new_connection) {
        establish_connection(best.oxc, best.port_a, best.port_b);
    }
    
    apply_route(flow.group_a, flow.leaf_a, flow.assigned_spine_a, flow.assigned_link_a,
                flow.assigned_oxc, flow.group_b, flow.leaf_b, flow.assigned_spine_b, flow.assigned_link_b);
}

bool validate_oxc_topology() {
    for (int m = 0; m < M; m++) {
        for (int r = 0; r < R; r++) {
            int connected = oxc_adjacency[m][r];
            if (connected == -1) continue;
            if (connected < 0 || connected >= R) return false;
            if (oxc_adjacency[m][connected] != r) return false;
        }
    }
    return true;
}

void fix_oxc_topology() {
    for (int m = 0; m < M; m++) {
        vector<int> to_clear;
        for (int r = 0; r < R; r++) {
            int connected = oxc_adjacency[m][r];
            if (connected == -1) continue;
            if (connected < 0 || connected >= R) {
                to_clear.push_back(r);
                continue;
            }
            if (oxc_adjacency[m][connected] != r) {
                to_clear.push_back(r);
            }
        }
        for (int r : to_clear) {
            int connected = oxc_adjacency[m][r];
            oxc_adjacency[m][r] = -1;
            if (connected >= 0 && connected < R && oxc_adjacency[m][connected] == r) {
                oxc_adjacency[m][connected] = -1;
            }
        }
    }
}

void output_oxc_topology() {
    fix_oxc_topology();
    for (int m = 0; m < M; m++) {
        for (int r = 0; r < R; r++) {
            if (r > 0) cout << " ";
            cout << oxc_adjacency[m][r];
        }
        cout << "\n";
    }
}

void output_flow_routes(const vector<FlowDemand>& flows) {
    for (const auto& flow : flows) {
        cout << flow.assigned_spine_a << " " << flow.assigned_link_a << " "
             << flow.assigned_oxc << " " << flow.assigned_spine_b << " "
             << flow.assigned_link_b << "\n";
    }
}

void process_query() {
    int Q;
    cin >> Q;
    
    vector<FlowDemand> flows(Q);
    for (int i = 0; i < Q; i++) {
        flows[i].id = i;
        cin >> flows[i].group_a >> flows[i].leaf_a >> flows[i].group_b >> flows[i].leaf_b;
        flows[i].assigned = false;
    }
    
    reset_all_loads();
    
    for (int i = 0; i < Q; i++) {
        process_flow(flows[i]);
    }
    
    output_oxc_topology();
    output_flow_routes(flows);
}

class AdvancedFlowRouter {
public:
    int group_pair_demand[MAXN][MAXN];
    vector<int> flow_indices_by_pair[MAXN][MAXN];
    
    void analyze_demands(const vector<FlowDemand>& flows) {
        memset(group_pair_demand, 0, sizeof(group_pair_demand));
        for (int i = 0; i < MAXN; i++) {
            for (int j = 0; j < MAXN; j++) {
                flow_indices_by_pair[i][j].clear();
            }
        }
        
        for (int i = 0; i < (int)flows.size(); i++) {
            int ga = flows[i].group_a;
            int gb = flows[i].group_b;
            if (ga > gb) swap(ga, gb);
            group_pair_demand[ga][gb]++;
            flow_indices_by_pair[ga][gb].push_back(i);
        }
    }
    
    vector<tuple<int,int,int>> get_sorted_pairs() {
        vector<tuple<int,int,int>> pairs;
        for (int i = 0; i < N; i++) {
            for (int j = i + 1; j < N; j++) {
                if (group_pair_demand[i][j] > 0) {
                    pairs.push_back({group_pair_demand[i][j], i, j});
                }
            }
        }
        sort(pairs.rbegin(), pairs.rend());
        return pairs;
    }
};

class LoadBalancer {
public:
    int spine_load[MAXN][MAXS];
    int oxc_load[MAXM];
    
    void reset() {
        memset(spine_load, 0, sizeof(spine_load));
        memset(oxc_load, 0, sizeof(oxc_load));
    }
    
    void add_load(int group, int spine, int oxc) {
        spine_load[group][spine]++;
        oxc_load[oxc]++;
    }
    
    int get_spine_load(int group, int spine) {
        return spine_load[group][spine];
    }
    
    int get_oxc_load(int oxc) {
        return oxc_load[oxc];
    }
};

class ConnectionManager {
public:
    set<pii> active_connections[MAXM];
    
    void reset_all() {
        for (int m = 0; m < MAXM; m++) {
            active_connections[m].clear();
        }
    }
    
    void add_connection(int oxc, int port_a, int port_b) {
        if (port_a > port_b) swap(port_a, port_b);
        active_connections[oxc].insert({port_a, port_b});
    }
    
    void remove_connection(int oxc, int port_a, int port_b) {
        if (port_a > port_b) swap(port_a, port_b);
        active_connections[oxc].erase({port_a, port_b});
    }
    
    bool has_connection(int oxc, int port_a, int port_b) {
        if (port_a > port_b) swap(port_a, port_b);
        return active_connections[oxc].count({port_a, port_b}) > 0;
    }
    
    int count_connections(int oxc) {
        return active_connections[oxc].size();
    }
};

class TopologyOptimizer {
public:
    int current_topology[MAXM][MAXR];
    int target_topology[MAXM][MAXR];
    
    void copy_current_from_global() {
        for (int m = 0; m < M; m++) {
            for (int r = 0; r < R; r++) {
                current_topology[m][r] = oxc_adjacency[m][r];
            }
        }
    }
    
    void set_target(int m, int port_a, int port_b) {
        target_topology[m][port_a] = port_b;
        target_topology[m][port_b] = port_a;
    }
    
    void reset_target() {
        for (int m = 0; m < M; m++) {
            for (int r = 0; r < R; r++) {
                target_topology[m][r] = -1;
            }
        }
    }
    
    int calculate_edit_distance(int oxc) {
        int cost = 0;
        set<pii> current_set, target_set;
        
        for (int r = 0; r < R; r++) {
            if (current_topology[oxc][r] != -1 && r < current_topology[oxc][r]) {
                current_set.insert({r, current_topology[oxc][r]});
            }
            if (target_topology[oxc][r] != -1 && r < target_topology[oxc][r]) {
                target_set.insert({r, target_topology[oxc][r]});
            }
        }
        
        for (const auto& c : current_set) {
            if (target_set.find(c) == target_set.end()) {
                cost++;
            }
        }
        for (const auto& t : target_set) {
            if (current_set.find(t) == current_set.end()) {
                cost++;
            }
        }
        
        return cost;
    }
    
    int calculate_total_edit_distance() {
        int total = 0;
        for (int m = 0; m < M; m++) {
            total += calculate_edit_distance(m);
        }
        return total;
    }
};

class FlowScheduler {
public:
    struct ScheduledRoute {
        int flow_id;
        int spine_a, link_a, oxc, spine_b, link_b;
        int port_a, port_b;
    };
    
    vector<ScheduledRoute> schedule;
    
    void clear() {
        schedule.clear();
    }
    
    void add_route(int flow_id, int spine_a, int link_a, int oxc, int spine_b, int link_b, int port_a, int port_b) {
        ScheduledRoute sr;
        sr.flow_id = flow_id;
        sr.spine_a = spine_a;
        sr.link_a = link_a;
        sr.oxc = oxc;
        sr.spine_b = spine_b;
        sr.link_b = link_b;
        sr.port_a = port_a;
        sr.port_b = port_b;
        schedule.push_back(sr);
    }
    
    int size() {
        return schedule.size();
    }
};

class GroupPairRouter {
public:
    struct ConnectionSlot {
        int oxc;
        int port_a;
        int port_b;
        int current_load;
    };
    
    vector<ConnectionSlot> available_slots[MAXN][MAXN];
    
    void reset() {
        for (int i = 0; i < MAXN; i++) {
            for (int j = 0; j < MAXN; j++) {
                available_slots[i][j].clear();
            }
        }
    }
    
    void discover_slots(int group_a, int group_b) {
        int ga = min(group_a, group_b);
        int gb = max(group_a, group_b);
        available_slots[ga][gb].clear();
        
        for (int plane = 0; plane < P; plane++) {
            int oxc_start = plane * M_per_P;
            int oxc_end = oxc_start + M_per_P;
            
            for (int oxc = oxc_start; oxc < oxc_end; oxc++) {
                vector<pii> existing = find_all_connections(oxc, ga, gb);
                for (const auto& conn : existing) {
                    ConnectionSlot slot;
                    slot.oxc = oxc;
                    slot.port_a = conn.first;
                    slot.port_b = conn.second;
                    slot.current_load = load_oxc_port[oxc][conn.first] + load_oxc_port[oxc][conn.second];
                    available_slots[ga][gb].push_back(slot);
                }
                
                vector<int> free_a = find_free_ports_for_group(oxc, ga);
                vector<int> free_b = find_free_ports_for_group(oxc, gb);
                
                for (int pa : free_a) {
                    for (int pb : free_b) {
                        ConnectionSlot slot;
                        slot.oxc = oxc;
                        slot.port_a = pa;
                        slot.port_b = pb;
                        slot.current_load = 0;
                        available_slots[ga][gb].push_back(slot);
                        break;
                    }
                    if (!free_b.empty()) break;
                }
            }
        }
    }
    
    int get_slot_count(int group_a, int group_b) {
        int ga = min(group_a, group_b);
        int gb = max(group_a, group_b);
        return available_slots[ga][gb].size();
    }
};

class MetricCalculator {
public:
    int max_flow_conflict;
    int oxc_adjustment_cost;
    double convergence_ratio;
    
    void reset() {
        max_flow_conflict = 0;
        oxc_adjustment_cost = 0;
        convergence_ratio = 1.0;
    }
    
    void calculate_convergence_ratio() {
        int up = M_per_P * K;
        int down = L;
        if (down == up) convergence_ratio = 1.0;
        else if (down == 3 * up) convergence_ratio = 3.0;
        else if (down == 7 * up) convergence_ratio = 7.0;
        else convergence_ratio = (double)down / up;
    }
    
    void update_max_conflict(int load) {
        max_flow_conflict = max(max_flow_conflict, load);
    }
    
    double calculate_score() {
        double alpha = 1000.0;
        double beta = 300.0;
        double score = alpha / (max_flow_conflict * convergence_ratio);
        score += beta * (1.0 - (double)oxc_adjustment_cost / (M * R));
        return score;
    }
};

class SmartRouter {
public:
    AdvancedFlowRouter afr;
    LoadBalancer lb;
    ConnectionManager cm;
    TopologyOptimizer to;
    FlowScheduler fs;
    GroupPairRouter gpr;
    MetricCalculator mc;
    
    void initialize() {
        lb.reset();
        cm.reset_all();
        to.reset_target();
        fs.clear();
        gpr.reset();
        mc.reset();
        mc.calculate_convergence_ratio();
    }
    
    void precompute_for_query(const vector<FlowDemand>& flows) {
        afr.analyze_demands(flows);
    }
};

class GreedyOptimizer {
public:
    int best_assignment[MAXQ][5];
    int current_assignment[MAXQ][5];
    ll best_max_load;
    ll current_max_load;
    
    void reset() {
        best_max_load = INF;
        current_max_load = 0;
    }
    
    void save_best() {
        for (int i = 0; i < MAXQ; i++) {
            for (int j = 0; j < 5; j++) {
                best_assignment[i][j] = current_assignment[i][j];
            }
        }
        best_max_load = current_max_load;
    }
    
    void restore_best() {
        current_max_load = best_max_load;
    }
};

class LocalSearchOptimizer {
public:
    int iteration_count;
    int improvement_count;
    double temperature;
    
    void reset() {
        iteration_count = 0;
        improvement_count = 0;
        temperature = 100.0;
    }
    
    void cool_down() {
        temperature *= 0.95;
    }
    
    bool should_accept(ll old_cost, ll new_cost) {
        if (new_cost < old_cost) return true;
        if (temperature <= 0.01) return false;
        double prob = exp((double)(old_cost - new_cost) / temperature);
        return (rand() / (double)RAND_MAX) < prob;
    }
};

class BatchProcessor {
public:
    int batch_size;
    int current_batch;
    
    void reset(int total_flows) {
        batch_size = max(1, total_flows / 10);
        current_batch = 0;
    }
    
    int get_batch_start(int batch_id) {
        return batch_id * batch_size;
    }
    
    int get_batch_end(int batch_id, int total_flows) {
        return min((batch_id + 1) * batch_size, total_flows);
    }
};

class StatisticsCollector {
public:
    int total_flows_processed;
    int total_connections_created;
    int total_connections_reused;
    ll total_load_generated;
    
    void reset() {
        total_flows_processed = 0;
        total_connections_created = 0;
        total_connections_reused = 0;
        total_load_generated = 0;
    }
    
    void record_flow(bool new_connection, ll load) {
        total_flows_processed++;
        if (new_connection) total_connections_created++;
        else total_connections_reused++;
        total_load_generated += load;
    }
};

class PortSelector {
public:
    int select_best_port_for_group_leaf(int oxc, int group, int leaf, const vector<int>& free_ports) {
        if (free_ports.empty()) return -1;
        
        int best_port = free_ports[0];
        int best_load = IINF;
        
        for (int port : free_ports) {
            PortMapping pm = get_port_mapping(port);
            int plane = get_plane_of_oxc(oxc);
            int spine = local_spine_to_global(pm.local_spine, plane);
            int load = load_leaf_spine[group][leaf][spine];
            if (load < best_load) {
                best_load = load;
                best_port = port;
            }
        }
        
        return best_port;
    }
};

class ConnectionReuser {
public:
    map<tuple<int,int,int>, vector<pii>> connection_cache;
    
    void reset() {
        connection_cache.clear();
    }
    
    void cache_connection(int oxc, int group_a, int group_b, int port_a, int port_b) {
        int ga = min(group_a, group_b);
        int gb = max(group_a, group_b);
        connection_cache[{oxc, ga, gb}].push_back({port_a, port_b});
    }
    
    vector<pii> get_cached_connections(int oxc, int group_a, int group_b) {
        int ga = min(group_a, group_b);
        int gb = max(group_a, group_b);
        auto it = connection_cache.find({oxc, ga, gb});
        if (it != connection_cache.end()) {
            return it->second;
        }
        return {};
    }
};

class FlowPrioritizer {
public:
    vector<int> get_priority_order(const vector<FlowDemand>& flows) {
        vector<pair<int,int>> flow_weights;
        for (int i = 0; i < (int)flows.size(); i++) {
            int weight = abs(flows[i].group_a - flows[i].group_b);
            flow_weights.push_back({weight, i});
        }
        sort(flow_weights.rbegin(), flow_weights.rend());
        
        vector<int> order;
        for (const auto& fw : flow_weights) {
            order.push_back(fw.second);
        }
        return order;
    }
};

class ResourceTracker {
public:
    int port_usage[MAXM][MAXR];
    int connection_count[MAXM];
    
    void reset() {
        memset(port_usage, 0, sizeof(port_usage));
        memset(connection_count, 0, sizeof(connection_count));
    }
    
    void mark_used(int oxc, int port) {
        port_usage[oxc][port]++;
    }
    
    void mark_connection(int oxc) {
        connection_count[oxc]++;
    }
    
    int get_port_usage(int oxc, int port) {
        return port_usage[oxc][port];
    }
    
    int get_connection_count(int oxc) {
        return connection_count[oxc];
    }
};

class PlaneBalancer {
public:
    int plane_load[MAXP];
    
    void reset() {
        memset(plane_load, 0, sizeof(plane_load));
    }
    
    void add_load(int plane, int amount) {
        plane_load[plane] += amount;
    }
    
    int get_plane_load(int plane) {
        return plane_load[plane];
    }
    
    int get_least_loaded_plane() {
        int best = 0;
        for (int p = 1; p < P; p++) {
            if (plane_load[p] < plane_load[best]) {
                best = p;
            }
        }
        return best;
    }
};

class OxcBalancer {
public:
    int oxc_load_counter[MAXM];
    
    void reset() {
        memset(oxc_load_counter, 0, sizeof(oxc_load_counter));
    }
    
    void add_load(int oxc) {
        oxc_load_counter[oxc]++;
    }
    
    int get_load(int oxc) {
        return oxc_load_counter[oxc];
    }
    
    int get_least_loaded_oxc_in_plane(int plane) {
        int start = plane * M_per_P;
        int end = start + M_per_P;
        int best = start;
        for (int m = start; m < end; m++) {
            if (oxc_load_counter[m] < oxc_load_counter[best]) {
                best = m;
            }
        }
        return best;
    }
};

class SpineBalancer {
public:
    int spine_load_counter[MAXN][MAXS];
    
    void reset() {
        memset(spine_load_counter, 0, sizeof(spine_load_counter));
    }
    
    void add_load(int group, int spine) {
        spine_load_counter[group][spine]++;
    }
    
    int get_load(int group, int spine) {
        return spine_load_counter[group][spine];
    }
    
    int get_least_loaded_spine_in_plane(int group, int plane) {
        int start = plane * S_per_P;
        int end = start + S_per_P;
        int best = start;
        for (int s = start; s < end; s++) {
            if (spine_load_counter[group][s] < spine_load_counter[group][best]) {
                best = s;
            }
        }
        return best;
    }
};

void read_input_parameters() {
    if (!(cin >> N >> S >> L)) {
        exit(0);
    }
    cin >> M >> K >> P;
    
    S_per_P = S / P;
    M_per_P = M / P;
    R = N * S_per_P * K;
    
    initialize_mappings();
    initialize_oxc_state();
}

void solve_problem() {
    read_input_parameters();
    
    SmartRouter sr;
    GreedyOptimizer go;
    LocalSearchOptimizer lso;
    BatchProcessor bp;
    StatisticsCollector sc;
    PortSelector ps;
    ConnectionReuser cr;
    FlowPrioritizer fp;
    ResourceTracker rt;
    PlaneBalancer plb;
    OxcBalancer ob;
    SpineBalancer sb;
    
    for (int query = 0; query < MAX_QUERIES; query++) {
        int Q;
        cin >> Q;
        
        vector<FlowDemand> flows(Q);
        for (int i = 0; i < Q; i++) {
            flows[i].id = i;
            cin >> flows[i].group_a >> flows[i].leaf_a >> flows[i].group_b >> flows[i].leaf_b;
            flows[i].assigned = false;
        }
        
        reset_all_loads();
        sr.initialize();
        go.reset();
        lso.reset();
        bp.reset(Q);
        sc.reset();
        cr.reset();
        rt.reset();
        plb.reset();
        ob.reset();
        sb.reset();
        
        sr.precompute_for_query(flows);
        
        vector<int> priority_order = fp.get_priority_order(flows);
        
        for (int idx : priority_order) {
            FlowDemand& flow = flows[idx];
            RouteCandidate best = find_best_route_for_flow(flow.group_a, flow.leaf_a, flow.group_b, flow.leaf_b);
            
            flow.assigned_spine_a = best.spine_a;
            flow.assigned_link_a = best.link_a;
            flow.assigned_oxc = best.oxc;
            flow.assigned_spine_b = best.spine_b;
            flow.assigned_link_b = best.link_b;
            flow.assigned = true;
            
            if (best.new_connection) {
                establish_connection(best.oxc, best.port_a, best.port_b);
                sc.record_flow(true, best.max_load);
            } else {
                sc.record_flow(false, best.max_load);
            }
            
            apply_route(flow.group_a, flow.leaf_a, flow.assigned_spine_a, flow.assigned_link_a,
                        flow.assigned_oxc, flow.group_b, flow.leaf_b, flow.assigned_spine_b, flow.assigned_link_b);
            
            rt.mark_used(best.oxc, best.port_a);
            rt.mark_used(best.oxc, best.port_b);
            plb.add_load(get_plane_of_oxc(best.oxc), 1);
            ob.add_load(best.oxc);
            sb.add_load(flow.group_a, flow.assigned_spine_a);
            sb.add_load(flow.group_b, flow.assigned_spine_b);
        }
        
        output_oxc_topology();
        output_flow_routes(flows);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(NULL);
    
    solve_problem();
    
    return 0;
}

class DemandAnalyzer {
public:
    int pair_count[MAXN][MAXN];
    int leaf_activity[MAXN][MAXL];
    int group_total_demand[MAXN];
    
    void reset() {
        memset(pair_count, 0, sizeof(pair_count));
        memset(leaf_activity, 0, sizeof(leaf_activity));
        memset(group_total_demand, 0, sizeof(group_total_demand));
    }
    
    void analyze(const vector<FlowDemand>& flows) {
        reset();
        for (const auto& f : flows) {
            int ga = min(f.group_a, f.group_b);
            int gb = max(f.group_a, f.group_b);
            pair_count[ga][gb]++;
            leaf_activity[f.group_a][f.leaf_a]++;
            leaf_activity[f.group_b][f.leaf_b]++;
            group_total_demand[f.group_a]++;
            group_total_demand[f.group_b]++;
        }
    }
    
    int get_pair_count(int ga, int gb) {
        if (ga > gb) swap(ga, gb);
        return pair_count[ga][gb];
    }
    
    int get_leaf_activity(int group, int leaf) {
        return leaf_activity[group][leaf];
    }
    
    int get_group_demand(int group) {
        return group_total_demand[group];
    }
};

class PathValidator {
public:
    bool validate_path(int group_a, int spine_a, int link_a, int oxc, int group_b, int spine_b, int link_b) {
        int plane = get_plane_of_oxc(oxc);
        if (get_plane_of_spine(spine_a) != plane) return false;
        if (get_plane_of_spine(spine_b) != plane) return false;
        int port_a = get_port_id(group_a, global_spine_to_local(spine_a), link_a);
        int port_b = get_port_id(group_b, global_spine_to_local(spine_b), link_b);
        if (port_a >= R || port_b >= R) return false;
        int connected = oxc_adjacency[oxc][port_a];
        return connected == port_b;
    }
    
    bool check_port_bounds(int port, int max_r) {
        return port >= 0 && port < max_r;
    }
    
    bool check_group_bounds(int group, int max_n) {
        return group >= 0 && group < max_n;
    }
    
    bool check_spine_bounds(int spine, int max_s) {
        return spine >= 0 && spine < max_s;
    }
    
    bool check_oxc_bounds(int oxc, int max_m) {
        return oxc >= 0 && oxc < max_m;
    }
};

class ConnectionOptimizer {
public:
    int connection_usage[MAXM][MAXR];
    int connection_priority[MAXM][MAXR];
    
    void reset() {
        memset(connection_usage, 0, sizeof(connection_usage));
        memset(connection_priority, 0, sizeof(connection_priority));
    }
    
    void mark_usage(int oxc, int port, int count) {
        connection_usage[oxc][port] += count;
    }
    
    void set_priority(int oxc, int port, int priority) {
        connection_priority[oxc][port] = priority;
    }
    
    int get_usage(int oxc, int port) {
        return connection_usage[oxc][port];
    }
    
    int get_priority(int oxc, int port) {
        return connection_priority[oxc][port];
    }
    
    double calculate_connection_score(int oxc, int port_a, int port_b, int demand) {
        double score = 0.0;
        score -= connection_usage[oxc][port_a] * 10.0;
        score -= connection_usage[oxc][port_b] * 10.0;
        score += connection_priority[oxc][port_a] * 5.0;
        score += connection_priority[oxc][port_b] * 5.0;
        score += demand * 2.0;
        return score;
    }
};

class FlowDistributor {
public:
    vector<int> flow_assignment_order;
    map<pair<int,int>, vector<int>> flows_by_group_pair;
    
    void reset() {
        flow_assignment_order.clear();
        flows_by_group_pair.clear();
    }
    
    void prepare(const vector<FlowDemand>& flows) {
        reset();
        for (int i = 0; i < (int)flows.size(); i++) {
            int ga = min(flows[i].group_a, flows[i].group_b);
            int gb = max(flows[i].group_a, flows[i].group_b);
            flows_by_group_pair[{ga, gb}].push_back(i);
        }
    }
    
    vector<int> get_flows_for_pair(int ga, int gb) {
        if (ga > gb) swap(ga, gb);
        auto it = flows_by_group_pair.find({ga, gb});
        if (it != flows_by_group_pair.end()) {
            return it->second;
        }
        return {};
    }
    
    vector<pair<int,int>> get_all_pairs() {
        vector<pair<int,int>> pairs;
        for (const auto& kv : flows_by_group_pair) {
            pairs.push_back(kv.first);
        }
        return pairs;
    }
};

class RoundRobinScheduler {
public:
    int current_plane;
    int current_oxc[MAXP];
    int current_spine[MAXP][MAXN];
    
    void reset() {
        current_plane = 0;
        memset(current_oxc, 0, sizeof(current_oxc));
        memset(current_spine, 0, sizeof(current_spine));
    }
    
    int next_plane() {
        int result = current_plane;
        current_plane = (current_plane + 1) % P;
        return result;
    }
    
    int next_oxc(int plane) {
        int start = plane * M_per_P;
        int result = start + current_oxc[plane];
        current_oxc[plane] = (current_oxc[plane] + 1) % M_per_P;
        return result;
    }
    
    int next_spine(int plane, int group) {
        int start = plane * S_per_P;
        int result = start + current_spine[plane][group];
        current_spine[plane][group] = (current_spine[plane][group] + 1) % S_per_P;
        return result;
    }
};

class CostCalculator {
public:
    int edit_costs[MAXM];
    int total_edit_cost;
    int max_load_observed;
    
    void reset() {
        memset(edit_costs, 0, sizeof(edit_costs));
        total_edit_cost = 0;
        max_load_observed = 0;
    }
    
    void calculate_edit_cost_for_oxc(int oxc, int target_topology[MAXR], int current_topology[MAXR]) {
        int cost = 0;
        set<pii> current_set, target_set;
        
        for (int r = 0; r < R; r++) {
            if (current_topology[r] != -1 && r < current_topology[r]) {
                current_set.insert({r, current_topology[r]});
            }
            if (target_topology[r] != -1 && r < target_topology[r]) {
                target_set.insert({r, target_topology[r]});
            }
        }
        
        for (const auto& c : current_set) {
            if (target_set.find(c) == target_set.end()) cost++;
        }
        for (const auto& t : target_set) {
            if (current_set.find(t) == current_set.end()) cost++;
        }
        
        edit_costs[oxc] = cost;
    }
    
    void sum_total_cost() {
        total_edit_cost = 0;
        for (int m = 0; m < M; m++) {
            total_edit_cost += edit_costs[m];
        }
    }
    
    double calculate_final_score(double convergence_ratio) {
        double alpha = 1000.0;
        double beta = 300.0;
        double score = 0.0;
        if (max_load_observed > 0) {
            score += alpha / (max_load_observed * convergence_ratio);
        }
        score += beta * (1.0 - (double)total_edit_cost / (M * R));
        return score;
    }
};

class MultiPathRouter {
public:
    struct PathOption {
        int spine_a, link_a, oxc, spine_b, link_b;
        int port_a, port_b;
        ll max_load;
        ll total_load;
        bool exists;
        double score;
    };
    
    vector<PathOption> find_all_options(int ga, int la, int gb, int lb) {
        vector<PathOption> options;
        
        for (int plane = 0; plane < P; plane++) {
            int oxc_start = plane * M_per_P;
            int oxc_end = oxc_start + M_per_P;
            int spine_start = plane * S_per_P;
            
            for (int oxc = oxc_start; oxc < oxc_end; oxc++) {
                for (int ls_a = 0; ls_a < S_per_P; ls_a++) {
                    for (int k_a = 0; k_a < K; k_a++) {
                        int port_a = get_port_id(ga, ls_a, k_a);
                        if (port_a >= R) continue;
                        int connected = oxc_adjacency[oxc][port_a];
                        if (connected == -1) continue;
                        
                        PortMapping pm_b = get_port_mapping(connected);
                        if (pm_b.group != gb) continue;
                        
                        int spine_a = spine_start + ls_a;
                        int spine_b = spine_start + pm_b.local_spine;
                        int link_a = k_a;
                        int link_b = pm_b.link;
                        
                        PathOption opt;
                        opt.spine_a = spine_a;
                        opt.link_a = link_a;
                        opt.oxc = oxc;
                        opt.spine_b = spine_b;
                        opt.link_b = link_b;
                        opt.port_a = port_a;
                        opt.port_b = connected;
                        opt.max_load = calculate_max_load_after(ga, la, spine_a, link_a, oxc, gb, lb, spine_b, link_b);
                        opt.total_load = calculate_route_cost(ga, la, spine_a, link_a, oxc, gb, lb, spine_b, link_b);
                        opt.exists = true;
                        opt.score = -opt.max_load * 1000.0 - opt.total_load;
                        options.push_back(opt);
                    }
                }
                
                vector<int> free_a = find_free_ports_for_group(oxc, ga);
                vector<int> free_b = find_free_ports_for_group(oxc, gb);
                
                if (!free_a.empty() && !free_b.empty()) {
                    int best_pa = -1, best_pb = -1;
                    ll best_max_load = INF;
                    
                    for (int pa : free_a) {
                        for (int pb : free_b) {
                            PortMapping pm_a = get_port_mapping(pa);
                            PortMapping pm_b = get_port_mapping(pb);
                            int spine_a = spine_start + pm_a.local_spine;
                            int spine_b = spine_start + pm_b.local_spine;
                            
                            ll max_load = calculate_max_load_after(ga, la, spine_a, pm_a.link, oxc, gb, lb, spine_b, pm_b.link);
                            if (max_load < best_max_load) {
                                best_max_load = max_load;
                                best_pa = pa;
                                best_pb = pb;
                            }
                        }
                    }
                    
                    if (best_pa != -1 && best_pb != -1) {
                        PortMapping pm_a = get_port_mapping(best_pa);
                        PortMapping pm_b = get_port_mapping(best_pb);
                        int spine_a = spine_start + pm_a.local_spine;
                        int spine_b = spine_start + pm_b.local_spine;
                        
                        PathOption opt;
                        opt.spine_a = spine_a;
                        opt.link_a = pm_a.link;
                        opt.oxc = oxc;
                        opt.spine_b = spine_b;
                        opt.link_b = pm_b.link;
                        opt.port_a = best_pa;
                        opt.port_b = best_pb;
                        opt.max_load = best_max_load;
                        opt.total_load = calculate_route_cost(ga, la, spine_a, pm_a.link, oxc, gb, lb, spine_b, pm_b.link);
                        opt.exists = false;
                        opt.score = -opt.max_load * 1000.0 - opt.total_load - 100;
                        options.push_back(opt);
                    }
                }
            }
        }
        
        return options;
    }
    
    PathOption select_best_option(vector<PathOption>& options) {
        if (options.empty()) {
            PathOption fallback;
            fallback.spine_a = 0;
            fallback.link_a = 0;
            fallback.oxc = 0;
            fallback.spine_b = 0;
            fallback.link_b = 0;
            fallback.port_a = 0;
            fallback.port_b = 0;
            fallback.max_load = INF;
            fallback.total_load = INF;
            fallback.exists = false;
            fallback.score = -INF;
            return fallback;
        }
        
        sort(options.begin(), options.end(), [](const PathOption& a, const PathOption& b) {
            if (a.max_load != b.max_load) return a.max_load < b.max_load;
            if (a.exists != b.exists) return a.exists > b.exists;
            return a.total_load < b.total_load;
        });
        
        return options[0];
    }
};

class TopologyTracker {
public:
    int prev_topology[MAXM][MAXR];
    bool initialized;
    
    void reset() {
        initialized = false;
        for (int m = 0; m < MAXM; m++) {
            for (int r = 0; r < MAXR; r++) {
                prev_topology[m][r] = -1;
            }
        }
    }
    
    void save_current() {
        for (int m = 0; m < M; m++) {
            for (int r = 0; r < R; r++) {
                prev_topology[m][r] = oxc_adjacency[m][r];
            }
        }
        initialized = true;
    }
    
    int count_changes() {
        if (!initialized) return 0;
        int changes = 0;
        for (int m = 0; m < M; m++) {
            set<pii> prev_set, curr_set;
            for (int r = 0; r < R; r++) {
                if (prev_topology[m][r] != -1 && r < prev_topology[m][r]) {
                    prev_set.insert({r, prev_topology[m][r]});
                }
                if (oxc_adjacency[m][r] != -1 && r < oxc_adjacency[m][r]) {
                    curr_set.insert({r, oxc_adjacency[m][r]});
                }
            }
            for (const auto& p : prev_set) {
                if (curr_set.find(p) == curr_set.end()) changes++;
            }
            for (const auto& c : curr_set) {
                if (prev_set.find(c) == prev_set.end()) changes++;
            }
        }
        return changes;
    }
};

class LoadTracker {
public:
    int max_leaf_spine_load;
    int max_spine_oxc_load;
    int max_oxc_port_load;
    int overall_max_load;
    
    void reset() {
        max_leaf_spine_load = 0;
        max_spine_oxc_load = 0;
        max_oxc_port_load = 0;
        overall_max_load = 0;
    }
    
    void update(int ga, int la, int sa, int ka, int oxc, int gb, int lb, int sb, int kb) {
        max_leaf_spine_load = max(max_leaf_spine_load, load_leaf_spine[ga][la][sa]);
        max_leaf_spine_load = max(max_leaf_spine_load, load_leaf_spine[gb][lb][sb]);
        max_spine_oxc_load = max(max_spine_oxc_load, load_spine_oxc[ga][sa][oxc][ka]);
        max_spine_oxc_load = max(max_spine_oxc_load, load_spine_oxc[gb][sb][oxc][kb]);
        int port_a = get_port_id(ga, global_spine_to_local(sa), ka);
        int port_b = get_port_id(gb, global_spine_to_local(sb), kb);
        max_oxc_port_load = max(max_oxc_port_load, load_oxc_port[oxc][port_a]);
        max_oxc_port_load = max(max_oxc_port_load, load_oxc_port[oxc][port_b]);
        overall_max_load = max({max_leaf_spine_load, max_spine_oxc_load, max_oxc_port_load});
    }
    
    void compute_all() {
        max_leaf_spine_load = 0;
        for (int g = 0; g < N; g++) {
            for (int l = 0; l < L; l++) {
                for (int s = 0; s < S; s++) {
                    max_leaf_spine_load = max(max_leaf_spine_load, load_leaf_spine[g][l][s]);
                }
            }
        }
        max_spine_oxc_load = 0;
        for (int g = 0; g < N; g++) {
            for (int s = 0; s < S; s++) {
                for (int m = 0; m < M; m++) {
                    for (int k = 0; k < K; k++) {
                        max_spine_oxc_load = max(max_spine_oxc_load, load_spine_oxc[g][s][m][k]);
                    }
                }
            }
        }
        max_oxc_port_load = 0;
        for (int m = 0; m < M; m++) {
            for (int r = 0; r < R; r++) {
                max_oxc_port_load = max(max_oxc_port_load, load_oxc_port[m][r]);
            }
        }
        overall_max_load = max({max_leaf_spine_load, max_spine_oxc_load, max_oxc_port_load});
    }
};

class HeuristicOptimizer {
public:
    int iterations;
    int improvements;
    bool converged;
    
    void reset() {
        iterations = 0;
        improvements = 0;
        converged = false;
    }
    
    void step() {
        iterations++;
    }
    
    void record_improvement() {
        improvements++;
    }
    
    bool should_continue(int max_iterations) {
        return iterations < max_iterations && !converged;
    }
    
    void mark_converged() {
        converged = true;
    }
};

class BalancedRouter {
public:
    int spine_assignment_count[MAXN][MAXS];
    int oxc_assignment_count[MAXM];
    
    void reset() {
        memset(spine_assignment_count, 0, sizeof(spine_assignment_count));
        memset(oxc_assignment_count, 0, sizeof(oxc_assignment_count));
    }
    
    void record_assignment(int ga, int sa, int oxc, int gb, int sb) {
        spine_assignment_count[ga][sa]++;
        spine_assignment_count[gb][sb]++;
        oxc_assignment_count[oxc]++;
    }
    
    int get_spine_assignments(int group, int spine) {
        return spine_assignment_count[group][spine];
    }
    
    int get_oxc_assignments(int oxc) {
        return oxc_assignment_count[oxc];
    }
    
    double calculate_balance_score() {
        double variance_spine = 0.0;
        double variance_oxc = 0.0;
        int total_spine = 0, count_spine = 0;
        int total_oxc = 0, count_oxc = 0;
        
        for (int g = 0; g < N; g++) {
            for (int s = 0; s < S; s++) {
                total_spine += spine_assignment_count[g][s];
                count_spine++;
            }
        }
        for (int m = 0; m < M; m++) {
            total_oxc += oxc_assignment_count[m];
            count_oxc++;
        }
        
        double mean_spine = count_spine > 0 ? (double)total_spine / count_spine : 0.0;
        double mean_oxc = count_oxc > 0 ? (double)total_oxc / count_oxc : 0.0;
        
        for (int g = 0; g < N; g++) {
            for (int s = 0; s < S; s++) {
                double diff = spine_assignment_count[g][s] - mean_spine;
                variance_spine += diff * diff;
            }
        }
        for (int m = 0; m < M; m++) {
            double diff = oxc_assignment_count[m] - mean_oxc;
            variance_oxc += diff * diff;
        }
        
        return -(variance_spine + variance_oxc);
    }
};

class ResultCollector {
public:
    vector<array<int, 5>> flow_results;
    int total_new_connections;
    int total_reused_connections;
    int final_max_load;
    int final_edit_cost;
    
    void reset(int num_flows) {
        flow_results.clear();
        flow_results.resize(num_flows);
        total_new_connections = 0;
        total_reused_connections = 0;
        final_max_load = 0;
        final_edit_cost = 0;
    }
    
    void record_flow_result(int flow_id, int sa, int ka, int oxc, int sb, int kb, bool new_conn) {
        if (flow_id >= 0 && flow_id < (int)flow_results.size()) {
            flow_results[flow_id] = {sa, ka, oxc, sb, kb};
            if (new_conn) total_new_connections++;
            else total_reused_connections++;
        }
    }
    
    array<int, 5> get_result(int flow_id) {
        if (flow_id >= 0 && flow_id < (int)flow_results.size()) {
            return flow_results[flow_id];
        }
        return {0, 0, 0, 0, 0};
    }
};

class GroupPairAllocator {
public:
    int connections_per_pair[MAXN][MAXN][MAXM];
    
    void reset() {
        memset(connections_per_pair, 0, sizeof(connections_per_pair));
    }
    
    void add_connection(int ga, int gb, int oxc) {
        if (ga > gb) swap(ga, gb);
        connections_per_pair[ga][gb][oxc]++;
    }
    
    int get_connections(int ga, int gb, int oxc) {
        if (ga > gb) swap(ga, gb);
        return connections_per_pair[ga][gb][oxc];
    }
    
    int get_total_connections(int ga, int gb) {
        if (ga > gb) swap(ga, gb);
        int total = 0;
        for (int m = 0; m < M; m++) {
            total += connections_per_pair[ga][gb][m];
        }
        return total;
    }
};

class LeafSpineAssigner {
public:
    int leaf_spine_preference[MAXN][MAXL][MAXS];
    
    void reset() {
        memset(leaf_spine_preference, 0, sizeof(leaf_spine_preference));
    }
    
    void update_preference(int group, int leaf, int spine, int delta) {
        leaf_spine_preference[group][leaf][spine] += delta;
    }
    
    int get_preference(int group, int leaf, int spine) {
        return leaf_spine_preference[group][leaf][spine];
    }
    
    int find_best_spine_for_leaf(int group, int leaf, int plane) {
        int start = plane * S_per_P;
        int end = start + S_per_P;
        int best = start;
        int best_score = IINF;
        
        for (int s = start; s < end; s++) {
            int score = leaf_spine_preference[group][leaf][s] + load_leaf_spine[group][leaf][s];
            if (score < best_score) {
                best_score = score;
                best = s;
            }
        }
        
        return best;
    }
};

class IterativeRefiner {
public:
    int refinement_rounds;
    int total_improvements;
    double best_score;
    
    void reset() {
        refinement_rounds = 0;
        total_improvements = 0;
        best_score = -1e18;
    }
    
    void complete_round() {
        refinement_rounds++;
    }
    
    bool update_best(double score) {
        if (score > best_score) {
            best_score = score;
            total_improvements++;
            return true;
        }
        return false;
    }
    
    double get_best_score() {
        return best_score;
    }
};
