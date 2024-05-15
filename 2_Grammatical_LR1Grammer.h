#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include "1_Lexical_PreProcess.h"

using namespace std;

// LR1的一个项目
class LR1_item {
public:
	string left_symbol;				// 产生式左边的symbol
	vector<string> right_symbol;	// 产生式右边的symbol
	int index;						// 对应产生式的编号
	int dotPos;					    // 点的位置
	string seek_symbol;				// 前向搜索的一个符号

	// set中find和count均支持，const关键字必不可少
	bool operator==(const LR1_item& item);
	bool operator<(const LR1_item& item) const;
};

// LR1文法
class LR1_grammer {
public:
	vector<LR1_item> allItems;							// 所有项目
	vector<set<LR1_item>>closureFamily;					// 项目集族
	map<string, set<string>>FIRST;						// FIRST集
	map<pair<int, string>, pair<string, int>> LR1_table;// LR1分析表
	CGrammer grammer;									// 文法

	void printItems(set<LR1_item> inItems);//打印项目

	void generateFirst();							// 求所有文法符号的FIRST集
	void printFirst();								// 打印first集
	set<string> getStrFirstSet(vector<string> str);	// 求一个文法符号串的FIRST集
	
	void getClosure(set<LR1_item> inItems, set<LR1_item>& outItems);
	bool isSameClosure(set<LR1_item>closure1, set<LR1_item>closure2);

	
	void generateClosureFamily();	// 生成项目集族
	// 生成下一个项目集（状态）
	void generateNextClosure(set<LR1_item> fromItems, set<LR1_item>& toItems, 
		string presentStr);

	void fillGuiYueTable();			// 求出项目集后，调用该函数填规约项目
	void printLR1Table(string filename);
};


