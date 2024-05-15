#pragma once
#include <iostream>
#include <string>

using namespace std;

// 用来描述词法分析结果(tokens)的数据结构
struct TOKEN
{
	string token_key;	// 类型：保留字、标识符、number...
	string value;		// 值
	int row, col;		// 源文件中的位置，用于出错处理的定位
};