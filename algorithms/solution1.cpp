#include "game_map.hpp"
#include "read_map.hpp"
#include <set>
#include <unordered_map>
#include <iostream>
#include <cassert>

using namespace std;
struct step {
    int o1, o2;
};
struct state {
    game_map map;
    std::vector<step> steps;
};

struct comp_state {
    static float get_val(int current_score, int steps) {
        return (float) current_score + (float) steps * 30;
    }

    static float get_val(const state &s) {
        return get_val(s.map.current_score, s.map.get_steps());
    }

    bool operator()(const state &lhs, const state &rhs) {
        return get_val(lhs) > get_val(rhs);
    }

};

state pop_best_state(set<state, comp_state> &qu) {
    auto it = qu.begin();
    auto x = *it;
    qu.erase(it);
    return x;
}

state pop_worst_state(set<state, comp_state> &qu) {
    auto it = --qu.end();
    auto x = *it;
    qu.erase(it);
    return x;
}

state get_worst_state(set<state, comp_state> &qu) {
    auto it = --qu.end();
    return *it;
}

void show_ans(ostream &os, const state &s) {
    char buf[256];
    for (auto step : s.steps) {
        int r1 = step.o1 / 8, c1 = step.o1 % 8;
        int r2 = step.o2 / 8, c2 = step.o2 % 8;
        sprintf(buf, "(%d,%d)-(%d,%d)", r1, c1, r2, c2);
        os << buf << " ";
    }
    os << std::endl;
}

const int QUEUE_LIMIT = 50000;
static unordered_map<bitset<64>, int> cache;

state solve(const game_map &init_state) {
    set<state, comp_state> qu;
    qu.insert(state{init_state});
    auto best_solution = state{init_state};
    clock_t begin = clock();
    while (!qu.empty() && double(clock() - begin) / CLOCKS_PER_SEC < 40) {
        state x = pop_best_state(qu);
//        show_ans(cout, x);
        if (x.map.get_steps() == 32) {
            if (best_solution.map.current_score < x.map.current_score) {
                best_solution = x;
                show_ans(cerr, x);
            }
            continue;
        }
        for (int i = 0; i < 64; ++i) {
            if (x.map.empty(i / 8, i % 8)) continue;
            for (int j = 0; j < 64; ++j) {
                if (x.map.empty(j / 8, j % 8)) continue;
                int result = x.map.search(i / 8, i % 8, j / 8, j % 8);
                if (x.map.get_score_by_result(result) < 0) continue;
                int score = x.map.current_score + x.map.get_score_by_result(result);
                if (qu.size() < QUEUE_LIMIT ||
                    comp_state::get_val(score, x.map.get_steps() + 1) > comp_state::get_val(get_worst_state(qu))) {
                    auto new_s = x;
//                    to avoid redundant computation
//                    new_s.map.link(i / 8, i % 8, j / 8, j % 8);
                    new_s.map.current_score = score;
                    new_s.steps.push_back(step{i, j});
                    new_s.map.remove(i / 8, i % 8, j / 8, j % 8);
                    if (!cache.count(new_s.map.removed) || score > cache[new_s.map.removed]) {
                        qu.insert(new_s);
                        cache[new_s.map.removed] = score;
                        if (qu.size() > QUEUE_LIMIT)
                            pop_worst_state(qu);
                    }
                }

            }
        }
    }
    return best_solution;
}

int main() {
    ans_list = read_list(cin, 8 * 8);
    game_map map;
    auto answer = solve(map);
    assert(answer.steps.size() == 32);
    for (auto step : answer.steps) {
        cout << step.o1 << " " << step.o2 << " ";
    }
    cout << std::endl;

}
