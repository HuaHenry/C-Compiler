#pragma once
#include <iostream>
#include <vector>
#include <set>

using namespace std;


// 类C语言文法的产生式
struct productItem {
    string left_symbol;         // 产生式左边的symbol
    vector<string> right_symbol;// 产生式右边的symbol
};

// 类C语言文法类
class CGrammer {
public:
    vector<productItem> productItems;   // 所有产生式
    int startItemPos;                   // 起始产生式位置
    set<string> Terminals;              // 所有终结符
    set<string> NTterminals;            // 所有非终结符

    // 特殊符号定义
    const char DERI_SIGN = ':'; // 推导符
    const char SEPE_SIGN = '\\';// 分隔符
    const char AND_SIGN = ' ';  // 候选式内部连接符
    const char EXEG_SIGN = '$'; // 注释符
    const char INIT_SIGN = 'S'; // 扩展文法起始符
    const char NULL_SIGN = '@'; // 空串符

    CGrammer();
    void readGrammer(string filename);
    void printGrammer(string filename);
};