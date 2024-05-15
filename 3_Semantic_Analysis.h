#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <stack>

using namespace std;

//错误码
#define FILE_OPEN_ERROE 1
#define LEXICAL_ERROR_UNDEFINED_WORD 2
#define GRAMMAR_ERROR 3
#define GRAMMATICAL_ERROR_UNDEFINED_WORD 4
#define GRAMMATICAL_ERROR_CANNOT_ANALYSIS 5
#define SEMANTIC_ERROR_NO_MAIN 6
#define SEMANTIC_ERROR_REDEFINED 7
#define SEMANTIC_ERROR_UNDEFINED 8
#define SEMANTIC_ERROR_PARAMETER_NUM 9
//#define SEMANTIC_ERROR_UNDEFINED 10
#define SEMANTIC_ERROR_NO_RETURN 11

// 语义分析符号
struct SemanticSymbol
{
	string token;	// 符号类型
	string value;	// 值
	int row;		// 源程序中的位置
	int col;
	int tableIdx;	// 符号所在表的索引
	int idx;		// 符号在表中的索引
};

// 标识符相关信息，包括函数、变量、临时变量、常量的具体信息
struct IdentifierInfo
{
	// 标识符类型:函数、变量、临时变量、常量、返回值
	enum idType { FUN, VAR, TMP, CST, RET };

	idType idType;			// 标识符的类型
	string specifierType;	// 变(常)量类型/函数返回类型
	string identifierName;	// 标识符名称/常量值
	int funcParameterNum;	// 函数参数个数
	int funcEntry;			// 函数入口地址(四元式的标号)
	int funcTableIdx;		// 函数的函数符号表在整个程序的符号表列表中的索引
};

// 符号表
struct SemanticSymbolTable
{
	// 几种表的类型，分别为全局表、函数表、块级表、临时表
	enum TableType { GlobalTable, FunctionTable, BlockTable, TempTable };

	TableType table_type;			// 表的类型
	vector<IdentifierInfo> table;	// 符号表
	string table_name;				// 表名

	// 构造函数
	SemanticSymbolTable(const TableType type, const string name);
	// 根据变量名字寻找一个变量 没找到则返回-1，否则返回变量在表中的标号
	int FindSymbol(const string name);
	// 加入一个变量，返回加入的位置，若之前已经加入则返回-1。
	int AddSymbol(const IdentifierInfo id);
};

// 四元式
struct Quatenary
{
	int idx;				// 四元式标号
	string operatorType;	// 操作类型
	string op1;				// 操作数1
	string op2;				// 操作数2
	string result;			// 结果
};

// 语义分析
class SemanticAnalysis
{
public:
	vector<Quatenary> quaternary;		// 四元式
	int mainIdx;						// main函数对应的四元式标号
	int backpatchLevel;					// 回填层次
	vector<int> backpatchList;			// 回填列表
	int nextQuaternaryIdx;				// 下一个四元式标号
	int tmpVarCount;					// 临时变量计数
	vector<SemanticSymbol> symbolList;	// 语义分析过程的符号流
	vector<SemanticSymbolTable> tables;	// 程序所有符号表
	vector<int> currentTableStack;		// 当前作用域对应的符号表索引栈

	SemanticAnalysis();
	void AddSymbol2List(const SemanticSymbol symbol);
	void Analysis(const string production_left, const vector<string> production_right);
	void PrintQuaternary(const string file_path);

private:
	//语义分析函数
	void Semantic_Analysis_Program(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_ExtDef(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_VarSpecifier(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_FunSpecifier(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_FunDec(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_CreateFunTable_m(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_ParamDec(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_Block(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_Def(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_AssignStmt(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_Exp(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_AddSubExp(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_Item(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_Factor(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_CallStmt(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_CallFunCheck(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_Args(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_ReturnStmt(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_Relop(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_IfStmt(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_IfStmt_m1(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_IfStmt_m2(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_IfNext(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_IfStmt_next(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_WhileStmt(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_WhileStmt_m1(const string production_left, const vector<string> production_right);
	void Semantic_Analysis_WhileStmt_m2(const string production_left, const vector<string> production_right);

};