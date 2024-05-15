#include "3_Semantic_Analysis.h"
#include <fstream>

/*******************  SemanticSymbolTable  *******************/
// 构造函数
SemanticSymbolTable::SemanticSymbolTable(const TableType type, const string name)
{
	this->table_type = type;
	this->table_name = name;
}
// 根据名字寻找一个变量 没找到则返回-1，否则返回变量在表中的标号
int SemanticSymbolTable::FindSymbol(const string name)
{
	int size = this->table.size();
	for (int i = 0; i < size; i++) {
		if (table[i].identifierName == name)
			return i;
	}
	return -1;
}
// 加入一个变量，返回加入的位置
int SemanticSymbolTable::AddSymbol(const IdentifierInfo id)
{
	// 表中没有则加入 并返回其在表中的位置。
	if (FindSymbol(id.identifierName) == -1) {
		table.push_back(id);
		return table.size() - 1;
	}
	// 已存在则返回-1
	return -1;
}

/*******************  SemanticAnalysis  *******************/
// 构造函数
SemanticAnalysis::SemanticAnalysis()
{
	// 为语义分析构造全局符号表
	this->tables.push_back(SemanticSymbolTable(SemanticSymbolTable::GlobalTable, "theGlobalTable"));
	// 设置当前作用域为 全局作用域
	this->currentTableStack.push_back(0);
	// 构造临时变量符号表
	this->tables.push_back(SemanticSymbolTable(SemanticSymbolTable::TempTable, "theTempTable"));

	// 约定第0条四元式：(j,-,-,main_function)
	this->nextQuaternaryIdx = 1;
	this->mainIdx = -1;			// main函数标号初始值设置为-1
	this->tmpVarCount = 0;		// 临时变量计数为0
	this->backpatchLevel = 0;	// 起初不需要回填
}

void SemanticAnalysis::AddSymbol2List(const SemanticSymbol symbol)
{
	// 将所有符号信息放入symbol_list
	this->symbolList.push_back(symbol);
}

// 分析过程调用
// 根据不同类型，分不同情况进行语义分析
void SemanticAnalysis::Analysis(const string production_left, const vector<string> production_right){
	if (production_left == "Program")
		Semantic_Analysis_Program(production_left, production_right);
	else if (production_left == "ExtDef")
		Semantic_Analysis_ExtDef(production_left, production_right);
	else if (production_left == "VarSpecifier")
		Semantic_Analysis_VarSpecifier(production_left, production_right);
	else if (production_left == "FunSpecifier")
		Semantic_Analysis_FunSpecifier(production_left, production_right);
	else if (production_left == "FunDec")
		Semantic_Analysis_FunDec(production_left, production_right);
	else if (production_left == "CreateFunTable_m")
		Semantic_Analysis_CreateFunTable_m(production_left, production_right);
	else if (production_left == "ParamDec")
		Semantic_Analysis_ParamDec(production_left, production_right);
	else if (production_left == "Block")
		Semantic_Analysis_Block(production_left, production_right);
	else if (production_left == "Def")
		Semantic_Analysis_Def(production_left, production_right);
	else if (production_left == "AssignStmt")
		Semantic_Analysis_AssignStmt(production_left, production_right);
	else if (production_left == "Exp")
		Semantic_Analysis_Exp(production_left, production_right);
	else if (production_left == "AddSubExp")
		Semantic_Analysis_AddSubExp(production_left, production_right);
	else if (production_left == "Item")
		Semantic_Analysis_Item(production_left, production_right);
	else if (production_left == "Factor")
		Semantic_Analysis_Factor(production_left, production_right);
	else if (production_left == "CallStmt")
		Semantic_Analysis_CallStmt(production_left, production_right);
	else if (production_left == "CallFunCheck")
		Semantic_Analysis_CallFunCheck(production_left, production_right);
	else if (production_left == "Args")
		Semantic_Analysis_Args(production_left, production_right);
	else if (production_left == "ReturnStmt")
		Semantic_Analysis_ReturnStmt(production_left, production_right);
	else if (production_left == "Relop")
		Semantic_Analysis_Relop(production_left, production_right);
	else if (production_left == "IfStmt")
		Semantic_Analysis_IfStmt(production_left, production_right);
	else if (production_left == "IfStmt_m1")
		Semantic_Analysis_IfStmt_m1(production_left, production_right);
	else if (production_left == "IfStmt_m2")
		Semantic_Analysis_IfStmt_m2(production_left, production_right);
	else if (production_left == "IfNext")
		Semantic_Analysis_IfNext(production_left, production_right);
	else if (production_left == "IfStmt_next")
		Semantic_Analysis_IfStmt_next(production_left, production_right);
	else if (production_left == "WhileStmt")
		Semantic_Analysis_WhileStmt(production_left, production_right);
	else if (production_left == "WhileStmt_m1")
		Semantic_Analysis_WhileStmt_m1(production_left, production_right);
	else if (production_left == "WhileStmt_m2")
		Semantic_Analysis_WhileStmt_m2(production_left, production_right);
	else {
		if (production_right[0] != "@") {
			int count = production_right.size();
			while (count--)
				this->symbolList.pop_back();
		}
		this->symbolList.push_back({ production_left,"",-1,-1,-1,-1 });
	}
}

// 打印四元式表
void SemanticAnalysis::PrintQuaternary(const string file_path)
{
	ofstream fout;
	fout.open(file_path, ios::out);
	if (!fout) {
		cout << file_path << "文件打开失败!" << endl;
		return;
	}

	for (int i = 0; i < this->quaternary.size(); i++) {
		fout << this->quaternary[i].idx << " (" << this->quaternary[i].operatorType << "," <<
			this->quaternary[i].op1 << "," << this->quaternary[i].op2 << "," << this->quaternary[i].result << ")\n";
	}
}

// 语义分析函数

// Program : ExtDefList  
void SemanticAnalysis::Semantic_Analysis_Program(const string production_left, const vector<string> production_right)
{
	// 如果没有定义main函数，则报错
	if (mainIdx == -1) {
		cout << "语义分析中发生错误：未定义main函数" << endl;
		exit(SEMANTIC_ERROR_NO_MAIN);
	}
	int count = production_right.size();
	while (count--) {
		symbolList.pop_back();
	}
	// 在最前面加入四元式
	quaternary.insert(quaternary.begin(), { 0, "j","-" , "-", to_string(mainIdx) });
	
	symbolList.push_back({ production_left, "", -1, -1, -1,-1 });
}

// ExtDef : VarSpecifier identifier ; \ VarSpecifier FunDec Block
void SemanticAnalysis::Semantic_Analysis_ExtDef(const string production_left, const vector<string> production_right)
{
	// 如果是定义变量
	if (production_right.size() == 3) {
		// 目前symbolList的末尾是Specifier identifier ;，由此找到Specifier和identifier
		SemanticSymbol specifier = symbolList[symbolList.size() - 3];  // 变量类型
		SemanticSymbol identifier = symbolList[symbolList.size() - 2]; // 变量名

		// 用于判断该变量是否在当前层已经定义
		bool existed = false;
		SemanticSymbolTable* current_table = &tables[currentTableStack.back()];
		if (current_table->FindSymbol(identifier.value) != -1) {
			cout << "语义分析中发生错误：（" << identifier.row << "行，" << identifier.col << "列）变量" << identifier.value << "重定义" << endl;
			exit(SEMANTIC_ERROR_REDEFINED);
		}

		// 将这一变量加入table
		IdentifierInfo variable;
		variable.identifierName = identifier.value; // 变量名
		variable.idType = IdentifierInfo::VAR;
		variable.specifierType = specifier.value;   // 变量类型

		// 加入table
		current_table->AddSymbol(variable);

		// symbolList更新
		int count = production_right.size();
		while (count--)
			symbolList.pop_back();
		symbolList.push_back({ production_left,identifier.value,identifier.row,identifier.col,currentTableStack.back(), int(current_table->table.size() - 1) });
	}
	// 如果是定义函数
	else {
		SemanticSymbol identifier = symbolList[symbolList.size() - 2];

		// 需要退出作用域
		currentTableStack.pop_back();
		// 更新symbolList
		int count = production_right.size();
		while (count--)
			symbolList.pop_back();
		symbolList.push_back({ production_left,identifier.value,identifier.row,identifier.col,identifier.tableIdx,identifier.idx });
	}
}

// VarSpecifier : int \ void \ float \ double
void SemanticAnalysis::Semantic_Analysis_VarSpecifier(const string production_left, const vector<string> production_right)
{
	// symbolList的最后一个是int
	SemanticSymbol specifier = symbolList.back();
	specifier.value = production_right[0];			// 将变量类型设置为value加入symbolList
	int count = production_right.size();
	while (count--)
		symbolList.pop_back();
	symbolList.push_back({ production_left,specifier.value,specifier.row,specifier.col,-1,-1 });
}

// $ VarSpecifier : void \ int 
void SemanticAnalysis::Semantic_Analysis_FunSpecifier(const string production_left, const vector<string> production_right)
{
	// symbolList的最后一个是int或void
	SemanticSymbol specifier = symbolList.back();
	int count = production_right.size();
	while (count--)
		symbolList.pop_back();
	symbolList.push_back({ production_left,specifier.value,specifier.row,specifier.col,-1,-1 });
}

// FunDec : identifier CreateFunTable_m ( VarList )
void SemanticAnalysis::Semantic_Analysis_FunDec(const string production_left, const vector<string> production_right)
{
	// symbolList的CreateFunTable_m记录了table信息
	SemanticSymbol specifier = symbolList[symbolList.size() - 4];
	int count = production_right.size();
	while (count--)
		symbolList.pop_back();
	symbolList.push_back({ production_left,specifier.value,specifier.row,specifier.col,specifier.tableIdx,specifier.idx });
}
// CreateFunTable_m : @
void SemanticAnalysis::Semantic_Analysis_CreateFunTable_m(const string production_left, const vector<string> production_right)
{
	// 创建函数表
	// 此时symbolList最后一个符号为函数名，倒数第二个为函数返回值
	SemanticSymbol identifier = symbolList.back();
	SemanticSymbol specifier = symbolList[symbolList.size() - 2];

	// 首先在全局的table判断函数名是否重定义
	if (tables[0].FindSymbol(identifier.value) != -1) {
		cout << "语义分析中发生错误：（" << identifier.row << "行，" << identifier.col << "列）函数" << identifier.value << "重定义" << endl;
		exit(SEMANTIC_ERROR_REDEFINED);
	}

	// 创建函数表
	tables.push_back(SemanticSymbolTable(SemanticSymbolTable::FunctionTable, identifier.value));
	// 在全局符号表创建当前函数的符号项（这里参数个数和入口地址会进行回填）
	tables[0].AddSymbol({ IdentifierInfo::FUN,specifier.value,identifier.value,0,0,int(tables.size() - 1) });

	// 函数表压入栈
	currentTableStack.push_back(tables.size() - 1);
	// 返回值
	IdentifierInfo return_value;
	return_value.idType = IdentifierInfo::RET;
	return_value.identifierName = tables.back().table_name + "_return_value";
	return_value.specifierType = specifier.value;
	// 如果为main函数，则进行记录
	if (identifier.value == "main")
		mainIdx = nextQuaternaryIdx;
	// 加入四元式
	quaternary.push_back({ nextQuaternaryIdx++ , identifier.value,"-","-" ,"-" });
	// 向函数表中加入返回变量
	tables[currentTableStack.back()].AddSymbol(return_value);
	// 空串不需要pop
	// 进行pushback
	symbolList.push_back({ production_left,identifier.value,identifier.row,identifier.col,0,int(tables[0].table.size() - 1) });
}
// ParamDec : VarSpecifier identifier
void SemanticAnalysis::Semantic_Analysis_ParamDec(const string production_left, const vector<string> production_right)
{
	// symbolList最后一个为变量名，倒数第二个为类型
	SemanticSymbol identifier = symbolList.back();
	SemanticSymbol specifier = symbolList[symbolList.size() - 2];
	// 当前函数表
	SemanticSymbolTable& function_table = tables[currentTableStack.back()];


	// 如果已经进行过定义
	if (function_table.FindSymbol(identifier.value) != -1) {
		cout << "语义分析中发生错误：（" << identifier.row << "行，" << identifier.col << "列）函数参数" << identifier.value << "重定义" << endl;
		exit(SEMANTIC_ERROR_REDEFINED);
	}
	// 函数表加入形参变量
	int new_position = function_table.AddSymbol({ IdentifierInfo::VAR,specifier.value,identifier.value,-1,-1,-1 });
	// 当前函数在全局符号中的索引
	int table_position = tables[0].FindSymbol(function_table.table_name);
	// 形参个数++
	tables[0].table[table_position].funcParameterNum++;

	// 加入四元式
	quaternary.push_back({ nextQuaternaryIdx++, "defpar","-" , "-", identifier.value });

	// symbolList更新
	int count = production_right.size();
	while (count--)
		symbolList.pop_back();
	symbolList.push_back({ production_left,identifier.value,identifier.row,identifier.col,currentTableStack.back(),new_position });
}
// Block : { DefList StmtList }
void SemanticAnalysis::Semantic_Analysis_Block(const string production_left, const vector<string> production_right)
{
	// 更新symbolList
	int count = production_right.size();
	while (count--)
		symbolList.pop_back();
	symbolList.push_back({ production_left,to_string(nextQuaternaryIdx),-1,-1,-1,-1 });
}
// Def : VarSpecifier identifier ;
void SemanticAnalysis::Semantic_Analysis_Def(const string production_left, const vector<string> production_right)
{
	// symbolList的倒数第二个、倒数第三个是变量名和类型
	SemanticSymbol identifier = symbolList[symbolList.size() - 2];
	SemanticSymbol specifier = symbolList[symbolList.size() - 3];
	SemanticSymbolTable& current_table = tables[currentTableStack.back()];

	// 重定义则报错
	if (current_table.FindSymbol(identifier.value) != -1)
	{
		cout << "语义分析中发生错误：（" << identifier.row << "行，" << identifier.col << "列）变量" << identifier.value << "重定义" << endl;
		exit(SEMANTIC_ERROR_REDEFINED);
	}

	current_table.AddSymbol({ IdentifierInfo::VAR,specifier.value,identifier.value ,-1,-1,-1 });

	int count = production_right.size();
	while (count--)
		symbolList.pop_back();
	symbolList.push_back({ production_left, identifier.value ,identifier.row,identifier.col,currentTableStack.back(),int(tables[currentTableStack.back()].table.size() - 1) });
}
// AssignStmt : identifier = Exp
void SemanticAnalysis::Semantic_Analysis_AssignStmt(const string production_left, const vector<string> production_right)
{
	// symbolList的倒数第一个、倒数第三个是Exp和变量名
	SemanticSymbol identifier = symbolList[symbolList.size() - 3];
	SemanticSymbol exp = symbolList.back();

	// 检查id是否存在，不存在则报错
	bool existed = false;
	int tableIdx = -1, idx = -1;
	// 从当前层开始向上遍历
	for (int scope_layer = currentTableStack.size() - 1; scope_layer >= 0; scope_layer--) {
		auto current_table = tables[currentTableStack[scope_layer]];
		if ((idx = current_table.FindSymbol(identifier.value)) != -1) {
			existed = true;
			tableIdx = currentTableStack[scope_layer];
			break;
		}
	}
	if (existed == false) {
		cout << "语义分析中发生错误：（" << identifier.row << "行，" << identifier.col << "列）变量" << identifier.value << "未定义" << endl;
		exit(SEMANTIC_ERROR_UNDEFINED);
	}

	quaternary.push_back({ nextQuaternaryIdx++, "=", exp.value, "-", identifier.value });

	// 更新symbolList
	int count = production_right.size();
	while (count--)
		symbolList.pop_back();
	symbolList.push_back({ production_left, identifier.value ,identifier.row,identifier.col,tableIdx,idx });
}
// Exp : AddSubExp \ Exp Relop AddSubExp
void SemanticAnalysis::Semantic_Analysis_Exp(const string production_left, const vector<string> production_right)
{
	if (production_right.size() == 1) {
		SemanticSymbol exp = symbolList.back();
		// 更新symbolList
		int count = production_right.size();
		while (count--)
			symbolList.pop_back();
		symbolList.push_back({ production_left, exp.value ,exp.row,exp.col,exp.tableIdx,exp.idx });
	}
	else {
		SemanticSymbol sub_exp1 = symbolList[symbolList.size() - 3];
		SemanticSymbol op = symbolList[symbolList.size() - 2];
		SemanticSymbol sub_exp2 = symbolList[symbolList.size() - 1];
		int next_label_num = nextQuaternaryIdx++;
		string new_tmp_var = "T" + to_string(tmpVarCount++);
		quaternary.push_back({ next_label_num, "j" + op.value, sub_exp1.value, sub_exp2.value, to_string(next_label_num + 3) });
		quaternary.push_back({ nextQuaternaryIdx++, "=", "0", "-", new_tmp_var });
		quaternary.push_back({ nextQuaternaryIdx++, "j", "-", "-", to_string(next_label_num + 4) });
		quaternary.push_back({ nextQuaternaryIdx++, "=", "1", "-", new_tmp_var });

		int count = production_right.size();
		while (count--)
			symbolList.pop_back();
		symbolList.push_back({ production_left, new_tmp_var ,-1,-1,-1,-1 });
	}
}
// AddSubExp : Item \ Item + Item \ Item - Item
void SemanticAnalysis::Semantic_Analysis_AddSubExp(const string production_left, const vector<string> production_right)
{
	if (production_right.size() == 1) {
		SemanticSymbol exp = symbolList.back();
		// 更新symbolList
		int count = production_right.size();
		while (count--)
			symbolList.pop_back();
		symbolList.push_back({ production_left, exp.value ,exp.row,exp.col,exp.tableIdx,exp.idx });
	}
	else {
		SemanticSymbol sub_exp1 = symbolList[symbolList.size() - 3];
		SemanticSymbol op = symbolList[symbolList.size() - 2];
		SemanticSymbol sub_exp2 = symbolList[symbolList.size() - 1];
		string new_tmp_var = "T" + to_string(tmpVarCount++);
		quaternary.push_back({ nextQuaternaryIdx++, op.token, sub_exp1.value, sub_exp2.value, new_tmp_var });

		int count = production_right.size();
		while (count--)
			symbolList.pop_back();
		symbolList.push_back({ production_left, new_tmp_var ,-1,-1,-1,-1 });
	}
}
// Item : Factor \ Factor * Factor \ Factor / Factor
void SemanticAnalysis::Semantic_Analysis_Item(const string production_left, const vector<string> production_right)
{
	if (production_right.size() == 1) {
		SemanticSymbol exp = symbolList.back();
		// 更新symbolList
		int count = production_right.size();
		while (count--)
			symbolList.pop_back();
		symbolList.push_back({ production_left, exp.value ,exp.row,exp.col,exp.tableIdx,exp.idx });
	}
	else {
		SemanticSymbol sub_exp1 = symbolList[symbolList.size() - 3];
		SemanticSymbol op = symbolList[symbolList.size() - 2];
		SemanticSymbol sub_exp2 = symbolList[symbolList.size() - 1];
		std::string new_tmp_var = "T" + to_string(tmpVarCount++);
		quaternary.push_back({ nextQuaternaryIdx++, op.token, sub_exp1.value, sub_exp2.value, new_tmp_var });

		int count = production_right.size();
		while (count--)
			symbolList.pop_back();
		symbolList.push_back({ production_left, new_tmp_var ,-1,-1,-1,-1 });
	}
}
// Factor : number \ ( Exp ) \ identifier \ CallStmt
void SemanticAnalysis::Semantic_Analysis_Factor(const string production_left, const vector<string> production_right)
{
	if (production_right.size() == 1) {
		SemanticSymbol exp = symbolList.back();
		// 如果是ID检查其是否进行过定义
		if (production_right[0] == "identifier") {
			bool existed = false;
			for (int scope_layer = currentTableStack.size() - 1; scope_layer >= 0; scope_layer--) {
				auto current_table = tables[currentTableStack[scope_layer]];
				if (current_table.FindSymbol(exp.value) != -1) {
					existed = true;
					break;
				}
			}
			if (existed == false) {
				cout << "语义分析中发生错误：（" << exp.row << "行，" << exp.col << "列）变量" << exp.value << "未定义" << endl;
				exit(SEMANTIC_ERROR_UNDEFINED);
			}
		}

		// 更新symbolList
		int count = production_right.size();
		while (count--)
			symbolList.pop_back();
		symbolList.push_back({ production_left, exp.value ,exp.row,exp.col,exp.tableIdx,exp.idx });
	}
	else {
		SemanticSymbol exp = symbolList[symbolList.size() - 2];

		// 更新symbolList
		int count = production_right.size();
		while (count--)
			symbolList.pop_back();
		symbolList.push_back({ production_left, exp.value ,exp.row,exp.col,exp.tableIdx,exp.idx });
	}
}
// CallStmt : identifier ( CallFunCheck Args )
void SemanticAnalysis::Semantic_Analysis_CallStmt(const string production_left, const vector<string> production_right)
{
	SemanticSymbol identifier = symbolList[symbolList.size() - 5];
	SemanticSymbol check = symbolList[symbolList.size() - 3];
	SemanticSymbol args = symbolList[symbolList.size() - 2];

	// 检查函数是否定义（在CallFunCheck时已经检查）

	// 检查参数个数
	int para_num = tables[check.tableIdx].table[check.idx].funcParameterNum;
	if (para_num > stoi(args.value)) {
		cout << "语义分析中发生错误：（" << identifier.row << "行，" << identifier.col << "列）函数" << identifier.value << "调用时参数过少" << endl;
		exit(SEMANTIC_ERROR_PARAMETER_NUM);
	}
	else if (para_num < stoi(args.value)) {
		cout << "语义分析中发生错误：（" << identifier.row << "行，" << identifier.col << "列）函数" << identifier.value << "调用时参数过多" << endl;
		exit(SEMANTIC_ERROR_PARAMETER_NUM);
	}

	// 生成函数调用四元式 
	string returnType = tables[0].table[tables[0].FindSymbol(identifier.value)].specifierType;
	string new_tmp_var = returnType == "void" ? "-" : ("T" + to_string(tmpVarCount++));
	quaternary.push_back({ nextQuaternaryIdx++, "call", identifier.value, "-", new_tmp_var });

	int count = production_right.size();
	while (count--)
		symbolList.pop_back();
	// 新的exp的value为临时变量名
	symbolList.push_back({ production_left, new_tmp_var ,-1,-1,-1,-1 });
}


// CallFunCheck : @
void SemanticAnalysis::Semantic_Analysis_CallFunCheck(const string production_left, const vector<string> production_right)
{
	SemanticSymbol fun_id = symbolList[symbolList.size() - 2];

	int fun_id_pos = tables[0].FindSymbol(fun_id.value);

	if (-1 == fun_id_pos) {
		cout << "语义分析中发生错误：（" << fun_id.row << "行，" << fun_id.col << "列）函数" << fun_id.value << "调用未定义" << endl;
		exit(SEMANTIC_ERROR_UNDEFINED);
	}
	if (tables[0].table[fun_id_pos].idType != IdentifierInfo::FUN) {
		cout << "语义分析中发生错误：（" << fun_id.row << "行，" << fun_id.col << "列）函数" << fun_id.value << "调用未定义" << endl;
		exit(SEMANTIC_ERROR_UNDEFINED);
	}
	symbolList.push_back({ production_left, fun_id.value,fun_id.row,fun_id.col, 0, fun_id_pos });
}
// Args : Exp , Args \ Exp \ @
void SemanticAnalysis::Semantic_Analysis_Args(const string production_left, const vector<string> production_right)
{
	if (production_right.size() == 3) {
		SemanticSymbol exp = symbolList[symbolList.size() - 3];
		quaternary.push_back({ nextQuaternaryIdx++, "param", exp.value, "-", "-" });
		int aru_num = stoi(symbolList.back().value) + 1;
		int count = production_right.size();
		while (count--)
			symbolList.pop_back();
		symbolList.push_back({ production_left, to_string(aru_num),-1,-1,-1,-1 });
	}
	else if (production_right[0] == "Exp") {
		SemanticSymbol exp = symbolList.back();
		quaternary.push_back({ nextQuaternaryIdx++, "param", exp.value, "-", "-" });
		int count = production_right.size();
		while (count--)
			symbolList.pop_back();
		symbolList.push_back({ production_left, "1",-1,-1,-1,-1 });
	}
	else if (production_right[0] == "@") {
		symbolList.push_back({ production_left, "0",-1,-1,-1,-1 });
	}
}
// ReturnStmt : return Exp \ return
void SemanticAnalysis::Semantic_Analysis_ReturnStmt(const string production_left, const vector<string> production_right)
{
	if (production_right.size() == 2) {
		// 返回值
		SemanticSymbol return_exp = symbolList.back();
		// 函数表
		SemanticSymbolTable function_table = tables[currentTableStack.back()];

		// 添加四元式
		quaternary.push_back({ nextQuaternaryIdx++,"=",return_exp.value,"-",function_table.table[0].identifierName });

		// 添加四元式
		quaternary.push_back({ nextQuaternaryIdx++ ,"return",function_table.table[0].identifierName,"-",function_table.table_name });

		// 更新symbolList
		int count = production_right.size();
		while (count--)
			symbolList.pop_back();
		symbolList.push_back({ production_left, return_exp.value,-1,-1,-1,-1 });
	}
	else {
		// 函数表
		SemanticSymbolTable function_table = tables[currentTableStack.back()];

		// 检查函数的返回值是否为void
		if (tables[0].table[tables[0].FindSymbol(function_table.table_name)].specifierType != "void") {
			cout << "语义分析中发生错误：（" << symbolList.back().row << "行，" << symbolList.back().col + sizeof("return") << "列）函数" << function_table.table_name << "必须有返回值" << endl;
			exit(SEMANTIC_ERROR_NO_RETURN);
		}

		// 添加四元式
		quaternary.push_back({ nextQuaternaryIdx++ ,"return","-","-",function_table.table_name });

		// 更新symbolList
		int count = production_right.size();
		while (count--)
			symbolList.pop_back();
		symbolList.push_back({ production_left, "",-1,-1,-1,-1 });
	}
}
// Relop : > \ < \ >= \ <= \ == \ !=
void SemanticAnalysis::Semantic_Analysis_Relop(const string production_left, const vector<string> production_right)
{
	SemanticSymbol op = symbolList.back();

	int count = production_right.size();
	while (count--) {
		symbolList.pop_back();
	}
	symbolList.push_back({ production_left, op.token ,-1,-1,-1,-1 });
}
// IfStmt : if IfStmt_m1 ( Exp ) IfStmt_m2 Block IfNext
void SemanticAnalysis::Semantic_Analysis_IfStmt(const string production_left, const vector<string> production_right)
{
	SemanticSymbol ifstmt_m2 = symbolList[symbolList.size() - 3];
	SemanticSymbol ifnext = symbolList[symbolList.size() - 1];

	if (ifnext.value.empty()) {
		// 只有if没有else
		// 真出口
		quaternary[backpatchList.back()].result = ifstmt_m2.value;
		backpatchList.pop_back();

		// 假出口
		quaternary[backpatchList.back()].result = to_string(nextQuaternaryIdx);
		backpatchList.pop_back();
	}
	else {
		// if块出口
		quaternary[backpatchList.back()].result = to_string(nextQuaternaryIdx);
		backpatchList.pop_back();
		// if真出口
		quaternary[backpatchList.back()].result = ifstmt_m2.value;
		backpatchList.pop_back();
		// if假出口
		quaternary[backpatchList.back()].result = ifnext.value;
		backpatchList.pop_back();
	}
	backpatchLevel--;

	// popback
	int count = production_right.size();
	while (count--)
		symbolList.pop_back();
	symbolList.push_back({ production_left,"",-1,-1,-1,-1 });
}
// IfStmt_m1 ::= @
void SemanticAnalysis::Semantic_Analysis_IfStmt_m1(const string production_left, const vector<string> production_right)
{
	backpatchLevel++;
	symbolList.push_back({ production_left,to_string(nextQuaternaryIdx),-1,-1,-1,-1 });
}
// IfStmt_m2 : @
void SemanticAnalysis::Semantic_Analysis_IfStmt_m2(const string production_left, const vector<string> production_right)
{
	SemanticSymbol if_exp = symbolList[symbolList.size() - 2];

	// 待回填四元式 : 假出口
	quaternary.push_back({ nextQuaternaryIdx++,"j=",if_exp.value,"0","" });
	backpatchList.push_back(quaternary.size() - 1);

	// 待回填四元式 : 真出口
	quaternary.push_back({ nextQuaternaryIdx++,"j=","-","-","" });
	backpatchList.push_back(quaternary.size() - 1);

	symbolList.push_back({ production_left,to_string(nextQuaternaryIdx),-1,-1,-1,-1 });
}
// IfNext : @ \ IfStmt_next else Block
void SemanticAnalysis::Semantic_Analysis_IfNext(const string production_left, const vector<string> production_right)
{
	SemanticSymbol if_stmt_next = symbolList[symbolList.size() - 3];

	int count = production_right.size();
	while (count--)
		symbolList.pop_back();

	symbolList.push_back({ production_left,if_stmt_next.value,-1,-1,-1,-1 });
}
// IfStmt_next : @
void SemanticAnalysis::Semantic_Analysis_IfStmt_next(const string production_left, const vector<string> production_right)
{
	// if 的跳出语句(else 之前)(待回填)
	quaternary.push_back({ nextQuaternaryIdx++,"j","-","-","" });
	backpatchList.push_back(quaternary.size() - 1);
	symbolList.push_back({ production_left,to_string(nextQuaternaryIdx),-1,-1,-1,-1 });
}
// WhileStmt : while WhileStmt_m1 ( Exp ) WhileStmt_m2 Block
void SemanticAnalysis::Semantic_Analysis_WhileStmt(const string production_left, const vector<string> production_right)
{
	SemanticSymbol whilestmt_m1 = symbolList[symbolList.size() - 6];
	SemanticSymbol whilestmt_m2 = symbolList[symbolList.size() - 2];

	// 无条件跳转到 while 的条件判断语句处
	quaternary.push_back({ nextQuaternaryIdx++,"j","-","-" ,whilestmt_m1.value });

	// 回填真出口
	quaternary[backpatchList.back()].result = whilestmt_m2.value;
	backpatchList.pop_back();

	// 回填假出口
	quaternary[backpatchList.back()].result = to_string(nextQuaternaryIdx);
	backpatchList.pop_back();

	backpatchLevel--;

	int count = production_right.size();
	while (count--)
		symbolList.pop_back();

	symbolList.push_back({ production_left,"",-1,-1,-1,-1 });

}
// WhileStmt_m1 : @
void SemanticAnalysis::Semantic_Analysis_WhileStmt_m1(const string production_left, const vector<string> production_right)
{
	backpatchLevel++;
	symbolList.push_back({ production_left,to_string(nextQuaternaryIdx),-1,-1,-1,-1 });
}

// WhileStmt_m2 : @
void SemanticAnalysis::Semantic_Analysis_WhileStmt_m2(const string production_left, const vector<string> production_right)
{
	SemanticSymbol while_exp = symbolList[symbolList.size() - 2];

	// 假出口
	quaternary.push_back({ nextQuaternaryIdx++,"j=",while_exp.value,"0","" });
	backpatchList.push_back(quaternary.size() - 1);
	// 真出口
	quaternary.push_back({ nextQuaternaryIdx++ ,"j","-","-" ,"" });
	backpatchList.push_back(quaternary.size() - 1);

	symbolList.push_back({ production_left,to_string(nextQuaternaryIdx),-1,-1,-1,-1 });
}

