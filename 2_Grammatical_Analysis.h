#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "1_Lexical_Analysis.h"
#include "3_Semantic_Analysis.h"
using namespace std;

struct productItem;
//struct Item {
//	string left_symbol;			  //产生式左边的symbol
//	vector<string> right_symbol;  //产生式右边的symbol
//};

// 穿线表，存储当前节点字符和指针
struct ThreadedList {
	string symbol;
	string IdIfExists;		// 如果是identifier，此处指示之
	// 若为非终结符，则当前非终结符为规约中第几步生成的（终结符则默认为0）
	// 注意不是穿线表的“孩子结点个数”，其可以通过vector.size()得到
	int regulationStep;
	vector<ThreadedList*>threads;

	ThreadedList(int step = 0) { regulationStep = step; }
};

// 太长了所以分出来写
#define ANSWER_TYPE      TOKEN
#define TABLE_KEY_TYPE   pair<int, string>
#define TABLE_VALUE_TYPE pair<string, int>
#define ITEM_TYPE		 productItem

void generateTreeDot(ThreadedList* Root);
void deleteThreadedList(ThreadedList* Root);
ThreadedList* grammaticalAnalysis(vector<ANSWER_TYPE>& Answer, map<TABLE_KEY_TYPE, TABLE_VALUE_TYPE>& Table, vector<ITEM_TYPE>& Items, SemanticAnalysis& Semantic);