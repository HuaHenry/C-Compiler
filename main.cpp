#include <iostream>
#include <iomanip>
#include <sstream>
#include "1_Lexical_PreProcess.h"
#include "2_Grammatical_Analysis.h"
#include "2_Grammatical_LR1Grammer.h"
#include "4_Assembly_Generator.h"

using namespace std;

extern void lexAnalysis(const string path);           // 词法分析接口函数
extern vector<TOKEN>lexAnswer;						  // 1、词法分析处理结果
map<pair<int, string>, pair<string, int>> LR1Table;   // 2、LR1分析表
std::vector<productItem>productionItems;              // 3、产生式
SemanticAnalysis semantic;							  // 4、语义分析
string START_SIGN = "Program";// 原文法的起始符


// 打印系统头部信息
void printInfo_start() {
	system("cls");
	cout << endl;
	cout << "  ██████╗     ██████╗ ██████╗ ███╗   ███╗██████╗ ██╗██╗     ███████╗██████╗ " << endl;
	cout << " ██╔════╝    ██╔════╝██╔═══██╗████╗ ████║██╔══██╗██║██║     ██╔════╝██╔══██╗" << endl;
	cout << " ██║         ██║     ██║   ██║██╔████╔██║██████╔╝██║██║     █████╗  ██████╔╝" << endl;
	cout << " ██║         ██║     ██║   ██║██║╚██╔╝██║██╔═══╝ ██║██║     ██╔══╝  ██╔══██╗" << endl;
	cout << " ╚██████╗    ╚██████╗╚██████╔╝██║ ╚═╝ ██║██║     ██║███████╗███████╗██║  ██║" << endl;
	cout << "  ╚═════╝     ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚═╝     ╚═╝╚══════╝╚══════╝╚═╝  ╚═╝" << endl;
	
	cout << endl << "                      类C语言编译器 -- 2151127 华洲琦                      " << endl << endl;
	cout << "------------------------------- 输 入 文 件 -------------------------------" << endl;
	cout << "                                                               " << endl;
	cout << "高级语言程序：sourceProgram.c                       " << endl;
	cout << "语法文件：    grammer.txt                           " << endl;
	cout << "                                                               " << endl;
	cout << "------------------------------- 输 出 文 件 -------------------------------" << endl;
	cout << "                                                               " << endl;
	cout << "词法分析结果：1_Lexical_AnalysisAnswer.txt         " << endl;
	cout << "语法分析文法：2_Grammatical_Grammer.txt            " << endl;
	cout << "LR1分析表：   2_Grammatical_LR1Table.txt           " << endl;
	cout << "语法树脚本：  syntaxTree.dot						" << endl;
	cout << "语法树图片：  syntaxTree.png						" << endl;
	cout << "中间四元式：  3_Semantic_Quaternary.txt			" << endl;
	cout << "                                                               " << endl;
	cout << "---------------------------------------------------------------------------" << endl;
	cout << "                                                               " << endl;
	system("pause");
}

// 词法分析打印二元组函数
void lexAnalysis_printRes(string ch) {
	if (ch == "y" || ch == "Y") {
		for (TOKEN tempPair : lexAnswer) {                          // 打印词法分析得到的文法符号序列
			cout << setw(15) << tempPair.token_key << " " << setw(15) << tempPair.value <<
				setw(10) << " ROW:" << setw(3) << tempPair.row << setw(10) << " COL:" << setw(3) << tempPair.col << endl;
		}
	}
}

// 词法分析控制函数
void lexAnalysis_controller() {
	system("cls");
	cout << "------------------- STEP1 词法分析 --------------------" << endl;
	cout << "正在进行词法分析……" << endl;
	lexAnalysis("../infiles/sourceProgram.c");               // 词法分析
	string ch;
	cout << "词法分析成功！" << endl;
	cout << "是否需要打印文法符号序列到屏幕？ y/Y - 是 ";
	cin >> ch;
	lexAnalysis_printRes(ch);
	system("pause");
}

// 语法分析控制函数
void GrammaticalAnalysis_controller() {
	system("cls");
	cout << "----------------- STEP2 语法分析 ------------------" << endl;
	LR1_grammer g;
	cout << "正在读取类C语言文法……" << endl;
	g.grammer.readGrammer("../infiles/grammer.txt");        // 读类C语言文法文法
	g.grammer.printGrammer("../outfiles/2_Grammatical_Grammer.txt");   // 打印文法（输出到文件）
	cout << "正在生成FIRST集……" << endl;
	g.generateFirst();                                       // 生成所有符号的first集 
	cout << "正在生成项目集族……" << endl;
	g.generateClosureFamily();                               // 生成项目集族（过程中会生成LR1分析表的移进部分和GoTo表部分）
	g.fillGuiYueTable();								     // 填写LR1分析表的归约部分
	
	LR1Table = g.LR1_table;
	productionItems = g.grammer.productItems;

	cout << "正在生成LR1分析表……" << endl;
	g.printLR1Table("../outfiles/2_Grammatical_LR1Table.txt");                // 打印LR1分析表

	// 语法分析（执行规约）+ 语法树绘制
	ThreadedList* Root = grammaticalAnalysis(lexAnswer, LR1Table, productionItems, semantic);
	if (!Root) {
		cout << "ERROR: 规约失败!" << endl;
		exit(EXIT_FAILURE);
	}
	else {
		cout << "规约成功!" << endl;
	}
	cout << "正在绘制语法树……" << endl;
	generateTreeDot(Root);            // 绘制语法树
	cout << "语法分析成功！" << endl;
	system("pause");
}

// 语义分析控制函数
void SemanticAnalysis_controller() {
	system("cls");
	cout << "------------------- STEP3 语义分析 -------------------" << endl;
	//在语法分析中已经执行完毕了语义分析，此处负责显示结果
	cout << "语义分析成功！" << endl;
	cout << "中间代码生成成功！" << endl;
	semantic.PrintQuaternary("../outfiles/3_Semantic_Quaternary.txt");
	cout << "是否打印四元式结果到屏幕？ y/Y - 是 ";
	string ch;
	cin >> ch;
	if (ch == "y" || ch == "Y") {
		for (int i = 0; i < semantic.quaternary.size(); i++) {
			cout << semantic.quaternary[i].idx << " (" << semantic.quaternary[i].operatorType << "," <<
				semantic.quaternary[i].op1 << "," << semantic.quaternary[i].op2 << "," << semantic.quaternary[i].result << ")\n";
		}
	}
	system("pause");
}

// 汇编代码生成
void AssemblyGeneration_controller() {
	system("cls");
	cout << "------------------ STEP4 Assembly Generation ------------------" << endl;
	cout << "正在进行汇编代码生成" << endl;
	ifstream quat_stream;
	quat_stream.open("../outfiles/3_Semantic_Quaternary.txt", ios::in);

	Assembly_Generator generator("../outfiles/4_Assembly.asm", semantic.quaternary);
	// Assembly_Generator generator("../outfiles/4_Assembly.asm", debug);
	generator.generate();
	cout << "汇编代码生成成功！" << endl;
	cout << "---------------------------------------------------------------" << endl;
	generator.Print_Assembly("../outfiles/4_Assembly.asm");
	cout << "---------------------------------------------------------------" << endl;
	system("pause");
}

// 编译成功后返回主页打印信息
void printInfo_end() {
	system("cls");
	cout << endl;
	cout << "      ___           ___           ___     " << endl;
	cout << "     /\\  \\         |\\__\\         /\\  \\    " << endl;
	cout << "    /::\\  \\        |:|  |       /::\\  \\   " << endl;
	cout << "   /:/\\:\\  \\       |:|  |      /:/\\:\\  \\  " << endl;
	cout << "  /::\\~\\:\\__\\      |:|__|__   /::\\~\\:\\  \\" << endl;
	cout << " /:/\\:\\ \\:|__|     /::::\\__\\ /:/\\:\\ \\:\\__\\ " << endl;
	cout << " \\:\\~\\:\\/:/  /    /:/~~/~    \\:\\~\\:\\ \\/__/" << endl;
	cout << "  \\:\\ \\::/  /    /:/  /       \\:\\ \\:\\__\\  " << endl;
	cout << "   \\:\\/:/  /     \\/__/         \\:\\ \\/__/  " << endl;
	cout << "    \\::/__/                     \\:\\__\\    " << endl;
	cout << "     ~~                          \\/__/    " << endl;
	cout << endl << endl << "编译成功" << endl;
	system("pause");
}

int main()
{
	// 打印系统信息
	printInfo_start();
	
	// 词法分析
	lexAnalysis_controller();

	// 语法分析
	GrammaticalAnalysis_controller();

	// 语义分析
	SemanticAnalysis_controller();

	// 汇编代码生成
	AssemblyGeneration_controller();
	
	// 返回主页
	printInfo_end();

	return 0;
}



