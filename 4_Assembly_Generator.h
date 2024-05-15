#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include "3_Semantic_Analysis.h"
#include "4_Register_Info.h"

#define REGS_NUMBER 32				// �Ĵ�������
#define DATA_SEG_ADDR 0x10010000	// ���ݶε���ʼ��ַ
#define STACK_SEG_ADDR 0x10040000	// ��ջ�ε���ʼ��ַ
#define LOCAL_VAR_OFFSET -8			// �ֲ�������ջ�������fp��ƫ��������ջ֡������йأ�

// ��Ҫע����ǣ����ɻ�����Ĺ����Ǵ�ͷ��ʼ�ģ����ڻ�������ԣ����޷��������ò�����ϵ��
// �����м�������ɲ����кܴ�Ĳ�ͬ
using namespace std;
class Assembly_Generator {
private:
	ofstream Asm_stream;				// ����ļ������
	vector<Quatenary> quatenary;		// �洢�м������Ԫʽ
	set<string>J_DestinationAddress;	// �洢��Ԫʽ����Ҫ��ת��Ŀ���ַ�����ں����ӱ�ǩ

	Reg RegisterInfo[REGS_NUMBER];		// �Ĵ�����Ϣ
	string RVALUE[REGS_NUMBER];			// RVALUE: �Ĵ����洢�ı�����
	map<string, int> AVALUE;		    // AVALUE: �������ڵ�һϵ�мĴ���(-1:�������ڴ�)

	map<string, int>localVariantAddressTable;	// �ֲ�������ַ��
	map<string, int>globalVariantAddressTable;	// ȫ�ֱ�����ַ��

	int localVarOffset = -4;			// �ֲ�������ַƫ��ָ��
	int globalVarAddr = DATA_SEG_ADDR;	// ȫ�ֱ�����ַָ��
	int paramsNumber = 2;				// ���ò�������������callʱ����ջ֡����λ��

	string procedureName;				// ���̿���

	void callTypeHandler(Quatenary quat);	// ��������
	void returnTypeHandler(Quatenary quat);	// ����
	void jTypeHandler(Quatenary quat);		// ��ת������j j< j> j= j<= j>=��
	void computeTypeHandler(Quatenary quat);// ���㣨����+ - * /��
	void assignTypeHandler(Quatenary quat);	// ��ֵ
	void paramTypeHandler(Quatenary quat);	// �β�����
	void defparTypeHandler(Quatenary quat);	// ʵ������
	void defprocTypeHandler(Quatenary quat);// ���̿�����
	
	// ����һ���Ĵ���������A��A := B op C��
	int allocateReg(string A, string B = "-", string C = "-");
	void saveVar(string var);			// ������var�����ڴ�
	// ѡһ���Ĵ�����������var�ŵ��Ĵ����У�ָ��������except1��except2
	int loadVar(string var, int except1 = -1, int except2 = -1);
	// ͨ��LRU�㷨�ҵ���Զδ��ʹ�õļĴ�����ָ��������except1��except2
	int LRU_GetRegister(int except1, int except2);				
public:
	Assembly_Generator(string, const vector<Quatenary>);
	~Assembly_Generator();
	void Print_Assembly(string path);
	void generate();
};
