#ifndef CORE_H
#define CORE_H

#include <cstdio>
#include <utility>
#include <iostream>
#include <QString>
#define REP(i,n) for (int i=1;i<=n;++i)
#define FOR(i,n) for (__typeof(n.begin())i=n.begin();i!=n.end();++i)
using namespace std;

enum Rename{
    Add1, Add2, Add3,
    Mult1, Mult2,
    Load1, Load2, Load3,
    Store1, Store2, Store3
};

enum Opname {
    ADDD, SUBD, MULTD, DIVD, LD, ST
};
int op_cost[] = {
    2, 2, 10, 40, 2, 2
};

struct Instruction {
    QString symbol;
    Opname op;
    int i, j, k;
} ins[100010];
int run_state[100010][3];

int time_cnt;
int fin_num, next_start_num, ins_num;
float mem[100010];
float reg[100010];

struct ExpItem{
    int order;
    enum Opname op; // ADDD or SUBD
    int i, j_, k_;
    float j, k;
};

struct LdItem{
    int order;
    enum Opname op;
    int i, j_;
    float j;
};

struct StItem{
    int order;
    enum Opname op;
    int j, i_;
    int i;
};

pair<bool, ExpItem> add_stack[3], mult_stack[2];
pair<bool, LdItem> load_stack[3];
pair<bool, StItem> store_stack[3];
int add_stack_size = 3;
int mult_stack_size = 2;
int load_stack_size = 3;
int store_stack_size = 3;

ExpItem generate_expitem(int o) {
    ExpItem item;
    item.order = o;
    item.op = ins[o].op;
    item.i = ins[o].i;
    item.j = item.k = 0;
    item.j_ = item.k_ = 0;
    for (int i = 0; i < add_stack_size; ++i)
        if (add_stack[i].first) {
            ExpItem* p = &add_stack[i].second;
            if (p->i == ins[o].j) item.j_ = max(item.j_, p->order);
            if (p->i == ins[o].k) item.k_ = max(item.k_, p->order);
        }
    for (int i = 0; i < mult_stack_size; ++i)
        if (mult_stack[i].first) {
            ExpItem* p = &mult_stack[i].second;
            if (p->i == ins[o].j) item.j_ = max(item.j_, p->order);
            if (p->i == ins[o].k) item.k_ = max(item.k_, p->order);
        }
    for (int i = 0; i < load_stack_size; ++i)
        if (load_stack[i].first) {
            LdItem* p = &load_stack[i].second;
            if (p->i == ins[o].j) item.j_ = max(item.j_, p->order);
            if (p->i == ins[o].k) item.k_ = max(item.k_, p->order);
        }

    run_state[o][0] = time_cnt;
    if (!item.j_ && !item.k_) {
        run_state[o][1] = run_state[o][0] + op_cost[item.op];
        run_state[o][2] = run_state[o][1] + 1;
    }
    if (!item.j_) item.j = reg[ins[o].j];
    if (!item.k_) item.k = reg[ins[o].k];
    return item;
}

LdItem generate_lditem(int o) {
    LdItem item;
    item.order = o;
    item.op = ins[o].op;
    item.i = ins[o].i;
    item.j = 0;
    item.j_ = 0;
    for (int i = 0; i < store_stack_size; ++i)
        if (store_stack[i].first) {
            StItem* p = &store_stack[i].second;
            if (p->j == ins[o].j) item.j_ = max(item.j_, p->order);
        }
    run_state[o][0] = time_cnt;
    if (!item.j_) {
        run_state[o][1] = run_state[o][0] + op_cost[item.op];
        run_state[o][2] = run_state[o][1] + 1;
    }
    if (!item.j_) item.j = mem[ins[o].j];
    return item;
}

StItem generate_stitem(int o) {
    StItem item;
    item.order = o;
    item.op = ins[o].op;
    item.j = ins[o].j;
    item.i = 0;
    item.i_ = 0;
    for (int i = 0; i < add_stack_size; ++i)
        if (add_stack[i].first) {
            ExpItem* p = &add_stack[i].second;
            if (p->i == ins[o].i) item.i_ = max(item.i_, p->order);
        }
    for (int i = 0; i < mult_stack_size; ++i)
        if (mult_stack[i].first) {
            ExpItem* p = &mult_stack[i].second;
            if (p->i == ins[o].i) item.i_ = max(item.i_, p->order);
        }
    for (int i = 0; i < load_stack_size; ++i)
        if (load_stack[i].first) {
            LdItem* p = &load_stack[i].second;
            if (p->i == ins[o].i) item.i_ = max(item.i_, p->order);
        }

    run_state[o][0] = time_cnt;
    if (!item.i_) {
        run_state[o][1] = run_state[o][0] + op_cost[item.op];
        run_state[o][2] = run_state[o][1] + 1;
    }
    if (!item.i_) item.i = reg[ins[o].i];
    return item;
}

void push_next_ins() {
    if (next_start_num > ins_num) return;
    int pos;
    switch (ins[next_start_num].op) {
        case ADDD:
        case SUBD:
            pos = 0;
            while (add_stack[pos].first && pos < 3) ++pos;
            if (pos == add_stack_size) return;
            add_stack[pos] = make_pair(true, generate_expitem(next_start_num));
            ins[next_start_num].symbol = QString("Add") + QString::number(pos + 1);
            break;
        case MULTD:
        case DIVD:
            pos = 0;
            while (mult_stack[pos].first && pos < 2) ++pos;
            if (pos == mult_stack_size) return;
            mult_stack[pos] = make_pair(true, generate_expitem(next_start_num));
            ins[next_start_num].symbol = QString("Mult") + QString::number(pos + 1);
            break;
        case LD:
            pos = 0;
            while (load_stack[pos].first && pos < 3) ++pos;
            if (pos == load_stack_size) return;
            load_stack[pos] = make_pair(true, generate_lditem(next_start_num));
            ins[next_start_num].symbol = QString("Load") + QString::number(pos + 1);
            break;
        case ST:
            pos = 0;
            while (store_stack[pos].first && pos < 3) ++pos;
            if (pos == store_stack_size) return;
            store_stack[pos] = make_pair(true, generate_stitem(next_start_num));
            ins[next_start_num].symbol = QString("Store") + QString::number(pos + 1);
            break;
    }
    ++next_start_num;
}

void update_global(int order, float result) {
    for (int i = 0; i < add_stack_size; ++i)
        if (add_stack[i].first) {
            ExpItem* p = &add_stack[i].second;
            if (p->j_ == order) {p->j = result; p->j_ = 0;}
            if (p->k_ == order) {p->k = result; p->k_ = 0;}
            if (!p->j_ && !p->k_ && !run_state[p->order][1]) {
                run_state[p->order][1] = time_cnt + op_cost[p->op];
                run_state[p->order][2] = run_state[p->order][1] + 1;
            }
        }
    for (int i = 0; i < mult_stack_size; ++i)
        if (mult_stack[i].first) {
            ExpItem* p = &mult_stack[i].second;
            if (p->j_ == order) {p->j = result; p->j_ = 0;}
            if (p->k_ == order) {p->k = result; p->k_ = 0;}
            if (!p->j_ && !p->k_ && !run_state[p->order][1]) {
                run_state[p->order][1] = time_cnt + op_cost[p->op];
                run_state[p->order][2] = run_state[p->order][1] + 1;
            }
        }
    for (int i = 0; i < load_stack_size; ++i)
        if (load_stack[i].first) {
            LdItem* p = &load_stack[i].second;
            if (p->j_ == order) {p->j = result; p->j_ = 0;}
            if (!p->j_ && !run_state[p->order][1]) {
                run_state[p->order][1] = time_cnt + op_cost[p->op];
                run_state[p->order][2] = run_state[p->order][1] + 1;
            }
        }
    for (int i = 0; i < store_stack_size; ++i)
        if (store_stack[i].first) {
            StItem* p = &store_stack[i].second;
            if (p->i_ == order) {p->i = result; p->i_ = 0;}
            if (!p->i_ && !run_state[p->order][1]) {
                run_state[p->order][1] = time_cnt + op_cost[p->op];
                run_state[p->order][2] = run_state[p->order][1] + 1;
            }
        }
}

void time_step() {
    for (int i = 0; i < add_stack_size; ++i)
        if (add_stack[i].first) {
            ExpItem* p = &add_stack[i].second;
            if (run_state[p->order][2] == time_cnt) {
                if (p->op == ADDD) reg[p->i] = p->j + p->k;
                if (p->op == SUBD) reg[p->i] = p->j - p->k;
                update_global(p->order, reg[p->i]);
                add_stack[i].first=false;
                ++fin_num;
            }
        }
    for (int i = 0; i < mult_stack_size; ++i)
        if (mult_stack[i].first) {
            ExpItem* p = &mult_stack[i].second;
            if (run_state[p->order][2] == time_cnt) {
                if (p->op == MULTD) reg[p->i] = p->j * p->k;
                if (p->op == DIVD) reg[p->i] = p->j * p->k;
                update_global(p->order, reg[p->i]);
                mult_stack[i].first=false;
                ++fin_num;
            }
        }
    for (int i = 0; i < load_stack_size; ++i)
        if (load_stack[i].first) {
            LdItem* p = &load_stack[i].second;
            if (run_state[p->order][2] == time_cnt) {
                reg[p->i] = p->j;
                update_global(p->order, reg[p->i]);
                load_stack[i].first=false;
                ++fin_num;
            }
        }
    for (int i = 0; i < store_stack_size; ++i)
        if (store_stack[i].first) {
            StItem* p = &store_stack[i].second;
            if (run_state[p->order][2] == time_cnt) {
                mem[p->j] = p->i;
                update_global(p->order, reg[p->j]);
                store_stack[i].first=false;
                ++fin_num;
            }
        }
}

#endif // CORE_H
