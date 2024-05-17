#include "4_Assembly_Generator.h"

// mips结构下，一共有32个寄存器
vector<string> registers = {
	"$zero",					// $0 常量0
	"$at",						// $1 由汇编器使用
	"$v0","$v1",				// $2-$3 用来存放子程序的返回值
	"$a0","$a1","$a2","$a3",	// $4-$7 用来传递子程序的前四个参数
	"$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",	// $8-$15 暂存器，用来存放子程序计算过程中的临时变量
	"$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",	// $16-$23 存放子程序调用过程中需要保持不变的值
	"$t8","$t9",				// $24-$25 保留
	"$k0","k1",					// $26-$27 操作系统／异常处理保留 
	"$gp",						// $28 全局指针 (Global Pointer)
	"$sp",						// $29 堆栈指针 (Stack Pointer) 指向栈顶上一个-4
	"$fp",						// $30 帧指针   (Frame Pointer)
	"$ra"						// $31 返回地址 (Return Address)
};

bool is_digit(string str)
{
	if (isdigit(str[0]))						// 是正数
		return true;
	else if (str[0] == '-' && isdigit(str[1]))	// 是负数
		return true;

	return false;
}

Assembly_Generator::Assembly_Generator(string out_path, const vector<Quatenary> input_quatenary)
{
	// 1. 打开输出文件
	Asm_stream.open(out_path, ios::out);
	if (!Asm_stream.is_open())
	{
		cout << "ERROR: cannot open the assembly output file!" << endl;
		exit(EXIT_FAILURE);
	}

	// 2. 存入生成的四元式
	quatenary = input_quatenary;

	// 3. 存储标签对应地址
	for (Quatenary cur : quatenary)
		if (cur.operatorType[0] == 'j')					// j j> j< j= ...
			J_DestinationAddress.insert(cur.result);

	// 4. 初始化寄存器信息以及寄存器值
	for (int i = 0; i < REGS_NUMBER; i++)
	{
		RegisterInfo[i] = { i, registers[i], 0 }; // {序号，名字，未使用时间}
		RVALUE[i] = i ? "" : "0";						// 0号寄存器赋值为0，其余不赋值
	}
}

// 汇编代码生成核心模块
// 根据四元式的类型执行不同的生成函数调用
void Assembly_Generator::generate()
{
	// 程序开始前，需要将main函数相关指针赋值，堆栈指针$sp赋栈顶esp，帧指针$fp赋返回地址ebp
	// main参数：argc argv
	Asm_stream << "addi  $sp,$sp," << to_string(STACK_SEG_ADDR - 4 * paramsNumber - 8) << endl;
	Asm_stream << "addi  $fp,$fp," << to_string(STACK_SEG_ADDR - 4 * paramsNumber) << endl;

	// int j = 0;
	for (Quatenary quat : quatenary)
	{
		// cout << j++ << endl;
		// 对于每一条四元式，各个寄存器未使用时间加一
		for (int i = 0; i < REGS_NUMBER; ++i)
		{
			Reg* reg = &RegisterInfo[i];
			if (reg->unusedTime < INT32_MAX)
				++reg->unusedTime;
		}

		// 如果需要输出标签，则输出之（所有标签都按照"Label_ + 地址"的形式命名）
		if (J_DestinationAddress.find(to_string(quat.idx)) != J_DestinationAddress.end()) {
			if (quat.op1 == "-" && quat.op2 == "-" && quat.result == "-") // 如果是过程块定义，则不需要
				;
			else
				Asm_stream << "Label_" << to_string(quat.idx) << " :" << endl;
		}

		// 对于每一条四元式，根据操作的不同，调用不同的处理函数
		string opType = quat.operatorType;
		if (opType == "call") {
			callTypeHandler(quat);
		}
		else if (opType == "return") {
			returnTypeHandler(quat);
		}
		else if (opType == "+" || opType == "-" || opType == "*" || opType == "/") {
			//if (AVALUE.find(quat.result) == AVALUE.end())
			//	AVALUE.insert({ quat.result, {} });    // 如果该变量不在AVALUE中，则插入
			computeTypeHandler(quat);
		}
		else if (opType == "=") {
			//if (AVALUE.find(quat.result) == AVALUE.end())
			//	AVALUE.insert({ quat.result, {} });    // 如果该变量不在AVALUE中，则插入
			assignTypeHandler(quat);
		}
		else if (opType == "param") {
			paramTypeHandler(quat);
		}
		else if (opType == "defpar") {
			defparTypeHandler(quat);
		}
		else if (opType[0] == 'j') {
			jTypeHandler(quat);
		}
		else if (quat.op1 == "-" && quat.op2 == "-" && quat.result == "-") {
			defprocTypeHandler(quat);
		}
	}
}

// 处理函数调用的汇编代码生成
void Assembly_Generator::callTypeHandler(Quatenary quat) 		// 函数调用
{
	// 1. 调用时更新栈帧 2. 返回后撤销剩余的栈帧（一部分在return处被撤销）
	
	// 栈帧的结构：									        fp↓     sp↓
	//			|    ..   |   0  -4  -8 -12 -16 -20 -24 -28 -32 -36 -40
	//		    |  params |  t0  t1  t2  t3  t4  t5  t6  t7  fp  ra 
	//													    (old)
	// 类似UNIX系统的栈帧结构

	// 函数调用时，需要保存寄存器现场（不考虑硬件现场），参数已经在前面的param四元式压栈
	// mips中没有push / pop指令，因此使用sw / lw形式实现
	{
		Asm_stream << "sw    $t0,($sp)   " << endl;	// 寄存器压栈
		Asm_stream << "sw    $t1,-4($sp) " << endl;
		Asm_stream << "sw    $t2,-8($sp) " << endl;
		Asm_stream << "sw    $t3,-12($sp)" << endl;
		Asm_stream << "sw    $t4,-16($sp)" << endl;
		Asm_stream << "sw    $t5,-20($sp)" << endl;
		Asm_stream << "sw    $t6,-24($sp)" << endl;
		Asm_stream << "sw    $t7,-28($sp)" << endl;
	}

	Asm_stream << "subi  $sp,$sp,32 " << endl; // 1. 分配栈帧
	Asm_stream << "sw    $ra,-4($sp)" << endl; // 2. 保存返回地址
	Asm_stream << "sw    $fp,($sp)  " << endl; // 3. 保存堆栈指针
	Asm_stream << "move  $sp,$fp    " << endl; // 4. 更新堆栈指针
	Asm_stream << "subi  $sp,$sp,8  " << endl; // 5. 更新栈帧

	Asm_stream << "jal   "<< quat.op1 << endl;

	// 跳转返回后的指令
	Asm_stream << "addiu $sp,$sp,8  " << endl;

	{
		Asm_stream << "lw    $t7,4($sp) " << endl;	// 寄存器出栈，恢复现场
		Asm_stream << "lw    $t6,8($sp) " << endl;
		Asm_stream << "lw    $t5,12($sp)" << endl;
		Asm_stream << "lw    $t4,16($sp)" << endl;
		Asm_stream << "lw    $t3,20($sp)" << endl;
		Asm_stream << "lw    $t2,24($sp)" << endl;
		Asm_stream << "lw    $t1,28($sp)" << endl;
		Asm_stream << "lw    $t0,32($sp)" << endl;
	}
	
	// 恢复sp到上一帧的栈顶，其中32是八个寄存器的偏移，剩下的是传入参数的偏移
	Asm_stream << "lw    $sp," << to_string(32 + paramsNumber * 4) << "($fp)  " << endl;
	Asm_stream << "lw    $fp,($fp) " << endl;  // 恢复fp到上一帧的old fp位置（类似UNIX中的old ebp）

	if (quat.result != "-")   // 如果调用时设置了变量接收返回值（考虑到可能有void函数不会接收返回值）
	{
		int returnReg = AVALUE[quat.result];
		// 大于零，说明在寄存器中，放到寄存器里即可
		if (returnReg <= 0) {
			// 为零，说明没有声明过，需要分配一个寄存器（由于下标访问不存在的话会直接插入，所以无需再次插入之）
			// 为-1，说明放到了内存 —— 但是没关系！反正也要覆盖，直接分配寄存器就好了！
			returnReg = allocateReg(quat.result, "-", "-");
		}
		Asm_stream << "move  " + registers[returnReg] + ", $v1" << endl;

		paramsNumber = 0;
	}
}

void Assembly_Generator::returnTypeHandler(Quatenary quat) 	// 返回
{
	// 1. 保存返回值 2. 撤销部分栈帧
	if (quat.op1 != "-")  // 如果有返回值，将返回值压栈，否则不需要压栈
		Asm_stream << "move  $v1," + registers[AVALUE[quat.op1]] << endl;

	// 释放局部变量占用的寄存器，修改AVALUE与RVALUE
	for (auto localVar = localVariantAddressTable.begin(); localVar != localVariantAddressTable.end(); ++localVar)
	{
		int reg = AVALUE[localVar->first];
		if (reg > 0)						// 将还在占用的寄存器清除
			RVALUE[reg] = "";
		AVALUE.erase(localVar->first);
	}

	Asm_stream << "move  $sp,$fp    " << endl;
	Asm_stream << "subi  $sp,$sp,8  " << endl;
	Asm_stream << "lw    $ra,-4($fp)" << endl;   // 返回地址赋给$ra

	// 如果不是main函数则跳转
	if (procedureName != "main")
		Asm_stream << "jr    $ra" << endl;
	else
		Asm_stream << "break" << endl;

	localVarOffset = LOCAL_VAR_OFFSET;
	localVariantAddressTable.clear();
	procedureName = "";
}

void Assembly_Generator::jTypeHandler(Quatenary quat) 		// 跳转（包括j j< j> j= j<= j>=）
{
	if (quat.operatorType == "j")
	{
		//Asm_stream << "j ";

		//// 四元式结果项首字符非数字，说明为函数名，否则为标签地址（对于后续的条件跳转，不会有跳转函数的情况发生）
		//if (!isdigit(quat.result[0])) {
		//	Asm_stream << functionAddressTable[quat.result] << endl;
		//}
		//else {
		//	Asm_stream << "Label_" << quat.result << endl;
		//}
		
		Quatenary dst = quatenary[atoi(quat.result.c_str())];
		if (dst.op1 == "-" && dst.op2 == "-" && dst.result == "-")
			Asm_stream << "j     " << dst.operatorType << endl;
		else
			Asm_stream << "j     Label_" << quat.result << endl;
	}
	else
	{
		int A_Reg = -1, B_Reg = -1;

		// 如果A是立即数，把A放在$t8，否则找到A所在的寄存器或为A分配寄存器
		if (isdigit(quat.op1[0])) {
			Asm_stream << "addi  $t8,$zero," << quat.op1 << endl;
			A_Reg = 24;
		}
		else {
			A_Reg = AVALUE[quat.op1];
			if (A_Reg == -1)  // 说明需要将参数从内存换入
				A_Reg = loadVar(quat.op1);
			else if (A_Reg == 0)
				A_Reg = allocateReg(quat.op1);
		}

		// B同理
		if (isdigit(quat.op2[0])) {
			Asm_stream << "addi  $t9,$zero," << quat.op2 << endl;
			B_Reg = 25;
		}
		// 否则找到A所在的寄存器 / 为A抢占寄存器
		else {
			B_Reg = allocateReg(quat.op2);
		}

		if (quat.operatorType == "j>") // J if A > B
			Asm_stream << "bgt   ";
		else if (quat.operatorType == "j=") // J if A = B
			Asm_stream << "beq   ";
		else if (quat.operatorType == "j<") // J if A < B
			Asm_stream << "blt   ";
		else if (quat.operatorType == "j>=") // J if A >= B
			Asm_stream << "bge   ";
		else if (quat.operatorType == "j<=") // J if A <= B
			Asm_stream << "ble   ";
		Asm_stream << registers[A_Reg] << "," << registers[B_Reg] << ",Label_" << quat.result << endl;
	}
}

// 计算（包括+ - * /）的汇编代码生成
void Assembly_Generator::computeTypeHandler(Quatenary quat) 	
{
	// 1. 分配寄存器，将B和C从内存换入
	
	// A := B +-*/ C 
	// 对于AVALUE为-1的变量B/C，考察内存是否有这个变量（应该是有的），需要lw指令存进寄存器
	int A_Reg = AVALUE[quat.result];
	int B_Reg = AVALUE[quat.op1];
	int C_Reg = -1; // C可能是立即数，要先判断（但B不会是立即数）

	if (B_Reg == -1)  // 说明B的值在内存，需要换入
		B_Reg = loadVar(quat.op1, A_Reg, C_Reg);
	else if (B_Reg == 0) // 说明没有出现过B，这种情况下B可能是全局变量，按照全局变量处理
	{
		// 先将全局变量B注册到表中
		globalVariantAddressTable.insert({ quat.op1, localVarOffset });
		globalVarAddr += 4; // 向下增长

		// 为B申请一个寄存器
		B_Reg = allocateReg(quat.op1, (A_Reg > 0) ? quat.result : "-");
	}

	if (!is_digit(quat.op2)) // C不是立即数，则需要考虑寄存器问题
	{
		C_Reg = AVALUE[quat.op2];
		if (C_Reg == -1)  // 说明C的值在内存，需要换入
			C_Reg = loadVar(quat.op2, A_Reg, B_Reg);
		RegisterInfo[C_Reg].unusedTime = 0;
	}

	//if (A_Reg == -1) // 如果在内存，需要换到寄存器中
	//	A_Reg = loadVar(quat.result, B_Reg, C_Reg);
	if (A_Reg <= 0) // 第一次使用，需要分配寄存器
	{
		if(is_digit(quat.op2))
			A_Reg = allocateReg(quat.result, quat.op1);
		else 
			A_Reg = allocateReg(quat.result, quat.op1, quat.op2);
	}

	// 2. 编写加减乘除的汇编
	if (quat.operatorType == "+")
	{
		if (is_digit(quat.op2))
			Asm_stream << "addi  " << registers[A_Reg] << "," << registers[B_Reg] << "," << quat.op2 << endl;
		else
			Asm_stream << "add   " << registers[A_Reg] << "," << registers[B_Reg] << "," << registers[C_Reg] << endl;
	}
	else if (quat.operatorType == "-")
	{
		if (is_digit(quat.op2))
			Asm_stream << "subi  " << registers[A_Reg] << "," << registers[B_Reg] << "," << quat.op2 << endl;
		else
			Asm_stream << "sub   " << registers[A_Reg] << "," << registers[B_Reg] << "," << registers[C_Reg] << endl;
	}
	else if (quat.operatorType == "*") // mips中mult指令先将结果保存到{hi，lo}，而后使用mfhi&mflo取出，此处使用mul指令简化
	{
		if (is_digit(quat.op2)) // MIPS中没有立即数乘法，必须先把立即数C赋值到一个临时寄存器
		{
			Asm_stream << "addi  $t8,$zero," << quat.op2 << endl;
			C_Reg = 24;
		}
		Asm_stream << "mul   " << registers[A_Reg] << "," << registers[B_Reg] << "," << registers[C_Reg] << endl;
	}
	else if (quat.operatorType == "/") 
	{		
		if (is_digit(quat.op2)) // MIPS中没有立即数乘法，必须先把立即数C赋值到一个临时寄存器
		{
			Asm_stream << "addi  $t8,$zero," << quat.op2 << endl;
			C_Reg = 24;
		}
		Asm_stream << "div   " << registers[B_Reg] << "," << registers[C_Reg] << endl;
		Asm_stream << "mov   " << registers[A_Reg] << ",$lo" << endl;
	}

	RegisterInfo[B_Reg].unusedTime = 0;
}

void Assembly_Generator::assignTypeHandler(Quatenary quat) 	// 赋值
{
	// A := B
	// 赋值操作，需要考虑立即数和变量两种情况；对于变量，则还需要将变量从内存换入
	int A_Reg = AVALUE[quat.result];
	int B_Reg = -1;

	if (!is_digit(quat.op1))		// 如果不是立即数，需要考虑寄存器分配问题
		B_Reg = AVALUE[quat.op1];   // 说明B的值在内存，但不需要换入，使用lw指令即可

	if (A_Reg <= 0)						// 为零，说明是新变量，分配一个寄存器，但不能和B冲突
	{
		if (is_digit(quat.op1))
			A_Reg = allocateReg(quat.result);
		else
			A_Reg = allocateReg(quat.result, quat.op1);
	}
	//else if (A_Reg == -1)			// 为-1，说明值在内存，需要换入（实际上不需要换入，因为即将覆盖，需要分配一个寄存器）
	//	A_Reg = loadVar(quat.result, B_Reg);

	// 进行赋值操作汇编的生成
	if (is_digit(quat.op1))
		Asm_stream << "addi  " << registers[A_Reg] << ",$zero," << quat.op1 << endl;
	else if (B_Reg != -1) // 如果B在寄存器中，使用move指令
		Asm_stream << "move  " << registers[A_Reg] << "," << registers[B_Reg] << endl;
	else // 如果B在内存中，使用lw指令
	{
		if (localVariantAddressTable.find(quat.op1) != localVariantAddressTable.end()) {
			int offset = localVariantAddressTable[quat.op1];
			Asm_stream << "lw    " << registers[A_Reg] << "," << to_string(offset) << "($fp)" << endl;
		}
		else if (globalVariantAddressTable.find(quat.op1) != globalVariantAddressTable.end()) {
			int addr = globalVariantAddressTable[quat.op1];
			Asm_stream << "lw    " << registers[A_Reg] << "," << to_string(addr) << "($zero)" << endl;
		}
	}

	if (B_Reg != -1)
		RegisterInfo[B_Reg].unusedTime = 0;
}

void Assembly_Generator::paramTypeHandler(Quatenary quat) 	// 实参声明
{
	// 实参声明时，将需要作为参数的值压入栈帧

	// 栈帧的结构：			sp↓
	//			|    ..   |   0  -4  -8 -12 
	//		    |  locals |  p1  p2  p3  ...
	// 类似UNIX系统的栈帧结构

	// 1. 准备工作：确保参数在寄存器中
	int reg = AVALUE[quat.op1];
	if (reg == -1) // 参数不在寄存器中，则将其从内存换至寄存器中
	{
		reg = loadVar(quat.op1);
		if (localVariantAddressTable.find(quat.op1) != localVariantAddressTable.end()) {
			int offset = localVariantAddressTable[quat.op1];
			Asm_stream << "lw    " << registers[reg] << "," << to_string(offset) << "($fp)" << endl;
		}
		else if (globalVariantAddressTable.find(quat.op1) != globalVariantAddressTable.end()) {
			int addr = globalVariantAddressTable[quat.op1];
			Asm_stream << "lw    " << registers[reg] << "," << to_string(addr) << "($zero)" << endl;
		}
	}

	// 2. 生成压栈汇编代码
	Asm_stream << "sw    " << registers[reg] << ",0($sp)" << endl;
	Asm_stream << "subi  $sp,$sp,4" << endl;

	RegisterInfo[reg].unusedTime = 0;
	++paramsNumber;  // 为后续call函数处理时，对于指针的恢复移位提供偏移
}

void Assembly_Generator::defparTypeHandler(Quatenary quat) 	// 形参声明
{
	// 栈帧的结构：									        fp↓     sp↓
	//			|    ..   |   0  -4  -8 -12 -16 -20 -24 -28 -32 -36 -40
	//		    |  params |  t0  t1  t2  t3  t4  t5  t6  t7  fp  ra 
	//													    (old)
	// 类似UNIX系统的栈帧结构
	
	// 为形参找到对应的实参地址匹配
	localVariantAddressTable.insert({ quat.result, 4 * (paramsNumber + 1 + 8) }); // +8 指八个寄存器现场
	++paramsNumber;
	AVALUE.insert({quat.result, -1}); // 形参都在内存中，因此插入-1
}

void Assembly_Generator::defprocTypeHandler(Quatenary quat) 	// 过程块声明
{
	paramsNumber = 0;
	procedureName = quat.operatorType;
	localVariantAddressTable.clear();
	localVarOffset = LOCAL_VAR_OFFSET;

	//过程标号
	Asm_stream << quat.operatorType << " :" << endl;
}

int Assembly_Generator::allocateReg(string A, string B, string C)
{
	int A_Reg = AVALUE[A];
	if (A_Reg > 0)
	{
		RegisterInfo[A_Reg].unusedTime = 0;
		return A_Reg;
	}

	// 如果B的现行值在某个寄存器Ri中，RVALUE[Ri]中只包含B，
	// 此外，或者B与A是同一个标识符，或者B的现行值在执行四元式A:=B op C之后不会再引用，
	// 则选取Ri为所需要的寄存器R
	int B_Reg = 0, C_Reg = 0;
	if (B != "-")
		B_Reg = AVALUE[B];
	if (C != "-")
		C_Reg = AVALUE[C];

	if (B_Reg > 0 && A == B)
	{
		RegisterInfo[B_Reg].unusedTime = 0;
		return B_Reg;
	}

	// 如果有尚未分配的寄存器，则从中选取一个Ri为所需要的寄存器R
	for (int i = 8; i <= 15; i++)
	{
		if (RVALUE[i].empty())	// 如果为空，说明空闲
		{
			AVALUE[A] = i;
			RVALUE[i] = A;		// 分配寄存器，修改RVALUE和AVALUE
			RegisterInfo[i].unusedTime = 0;
			return i;
		}
	}

	int seized_reg_index = LRU_GetRegister(B_Reg, C_Reg);    // 如果没有B和C，则对应的reg为-1，不影响这里换寄存器
	string M = RVALUE[seized_reg_index];	// 获得被抢占的变量名称

	if (M == A && M != B && M != C)			// 如果M是A，且不是B或C，不需要存数
		;
	else
		saveVar(M);

	// 更新AVALUE和RVALUE
	AVALUE[M] = -1;					// -1则说明已经被换到内存中
	AVALUE[A] = seized_reg_index;
	RVALUE[seized_reg_index] = A;

	RegisterInfo[seized_reg_index].unusedTime = 0;
	return seized_reg_index;
}

void Assembly_Generator::saveVar(string var)
{
	int reg_index = AVALUE[var];

	// 若var不在内存中，则需要先分配内存空间
	if (localVariantAddressTable.find(var) == localVariantAddressTable.end() && 
		globalVariantAddressTable.find(var) == globalVariantAddressTable.end())
	{
		// 过程块名称为""，说明是全局变量，否则是局部变量
		if (procedureName != "")
		{
			localVariantAddressTable.insert({ var, localVarOffset });
			localVarOffset -= 4; // 向上增长
			Asm_stream << "subi  $sp,$sp,4" << endl; // 局部变量放在栈区
		}
		else
		{
			globalVariantAddressTable.insert({ var, globalVarAddr });
			globalVarAddr += 4; // 向下增长
		}
	}

	if (localVariantAddressTable.find(var) != localVariantAddressTable.end()) {
		int offset = localVariantAddressTable[var];
		Asm_stream << "sw    " << registers[reg_index] << "," << to_string(offset) << "($fp)" << endl;
	}
	else if (globalVariantAddressTable.find(var) != globalVariantAddressTable.end()) {
		int addr = globalVariantAddressTable[var];
		Asm_stream << "sw    " << registers[reg_index] << "," << to_string(addr) << "($zero)" << endl;
	}
}

int Assembly_Generator::loadVar(string var, int except1, int except2)
{
	// 1. 查找可以换的寄存器，将寄存器中原本的内容（如果有）换到内存

	int reg_index = 0;	// 下面选一个寄存器，序号存在这里

	// 如果有尚未分配的寄存器，则从中选取一个Ri为所需要的寄存器R
	for (int i = 8; i <= 15; i++)
	{
		if (RVALUE[i].empty())	// 如果为空，说明空闲
		{
			reg_index = i;
			break;
		}
	}

	if (!reg_index)  // 没有空闲寄存器，则抢占一个
	{
		reg_index = LRU_GetRegister(except1, except2);

		string M = RVALUE[reg_index];	// 获得被抢占的变量名称
		saveVar(M);						// 将被抢占的变量存到内存
		AVALUE[M] = -1;					// -1则说明已经被换到内存中
	}
	
	// 2. 判断是全局变量还是局部变量，添加lw取数指令，将其真正换到寄存器内

	AVALUE[var] = reg_index;
	RVALUE[reg_index] = var;		// 分配寄存器，修改RVALUE和AVALUE
	RegisterInfo[reg_index].unusedTime = 0;

	if (localVariantAddressTable.find(var) != localVariantAddressTable.end()) {
		int offset = localVariantAddressTable[var];
		Asm_stream << "lw    " << registers[reg_index] << "," << to_string(offset) << "($fp)" << endl;
	}
	else if (globalVariantAddressTable.find(var) != globalVariantAddressTable.end()) {
		int addr = globalVariantAddressTable[var];
		Asm_stream << "lw    " << registers[reg_index] << "," << to_string(addr) << "($zero)" << endl;
	}

	return reg_index;
}

int Assembly_Generator::LRU_GetRegister(int except1 = -1, int except2 = -1)
{
	int seized_reg_index = -1;
	{
		int max_unusedTime = -1;		// 通过LRU算法，得到将要抢占的寄存器
		for (int i = 8; i <= 15; i++) {
			if (RegisterInfo[i].unusedTime > max_unusedTime && except1 != i && except2 != i)
			{
				seized_reg_index = i;
				max_unusedTime = RegisterInfo[i].unusedTime;
			}
		}
	}
	return seized_reg_index;
}

void Assembly_Generator::Print_Assembly(string path)
{
	ifstream AssemblyFile;
	AssemblyFile.open(path, ios::in);
	if (!AssemblyFile.is_open())
	{
		cout << "没有找到汇编文件" << endl;
		exit(EXIT_FAILURE);
	}

	string s;
	while (getline(AssemblyFile, s))
	{
		cout << s << endl;
	}
	AssemblyFile.close();             //关闭文件输入流
}

Assembly_Generator::~Assembly_Generator()
{
	Asm_stream.close();
}