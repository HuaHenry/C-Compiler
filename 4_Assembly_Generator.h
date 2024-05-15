#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include "3_Semantic_Analysis.h"
#include "4_Register_Info.h"

#define REGS_NUMBER 32				// 寄存器总数
#define DATA_SEG_ADDR 0x10010000	// 数据段的起始地址
#define STACK_SEG_ADDR 0x10040000	// 堆栈段的起始地址
#define LOCAL_VAR_OFFSET -8			// 局部变量在栈中相对于fp的偏移量（与栈帧的设计有关）

// 需要注意的是：生成汇编代码的过程是从头开始的，对于汇编代码而言，是无法看到调用层次与关系的
// 这与中间代码生成部分有很大的不同
using namespace std;
class Assembly_Generator {
private:
	ofstream Asm_stream;				// 汇编文件输出流
	vector<Quatenary> quatenary;		// 存储中间代码四元式
	set<string>J_DestinationAddress;	// 存储四元式中需要跳转的目标地址，用于后续加标签

	Reg RegisterInfo[REGS_NUMBER];		// 寄存器信息
	string RVALUE[REGS_NUMBER];			// RVALUE: 寄存器存储的变量名
	map<string, int> AVALUE;		    // AVALUE: 变量所在的一系列寄存器(-1:被换至内存)

	map<string, int>localVariantAddressTable;	// 局部变量地址表
	map<string, int>globalVariantAddressTable;	// 全局变量地址表

	int localVarOffset = -4;			// 局部变量地址偏移指针
	int globalVarAddr = DATA_SEG_ADDR;	// 全局变量地址指针
	int paramsNumber = 2;				// 调用参数个数，用于call时设置栈帧返回位移

	string procedureName;				// 过程块名

	void callTypeHandler(Quatenary quat);	// 函数调用
	void returnTypeHandler(Quatenary quat);	// 返回
	void jTypeHandler(Quatenary quat);		// 跳转（包括j j< j> j= j<= j>=）
	void computeTypeHandler(Quatenary quat);// 计算（包括+ - * /）
	void assignTypeHandler(Quatenary quat);	// 赋值
	void paramTypeHandler(Quatenary quat);	// 形参声明
	void defparTypeHandler(Quatenary quat);	// 实参声明
	void defprocTypeHandler(Quatenary quat);// 过程块声明
	
	// 分配一个寄存器给变量A（A := B op C）
	int allocateReg(string A, string B = "-", string C = "-");
	void saveVar(string var);			// 将变量var存入内存
	// 选一个寄存器，将变量var放到寄存器中，指定不考虑except1和except2
	int loadVar(string var, int except1 = -1, int except2 = -1);
	// 通过LRU算法找到最远未被使用的寄存器，指定不考虑except1和except2
	int LRU_GetRegister(int except1, int except2);				
public:
	Assembly_Generator(string, const vector<Quatenary>);
	~Assembly_Generator();
	void Print_Assembly(string path);
	void generate();
};
