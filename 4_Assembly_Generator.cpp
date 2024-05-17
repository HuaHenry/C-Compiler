#include "4_Assembly_Generator.h"

// mips�ṹ�£�һ����32���Ĵ���
vector<string> registers = {
	"$zero",					// $0 ����0
	"$at",						// $1 �ɻ����ʹ��
	"$v0","$v1",				// $2-$3 ��������ӳ���ķ���ֵ
	"$a0","$a1","$a2","$a3",	// $4-$7 ���������ӳ����ǰ�ĸ�����
	"$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",	// $8-$15 �ݴ�������������ӳ����������е���ʱ����
	"$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",	// $16-$23 ����ӳ�����ù�������Ҫ���ֲ����ֵ
	"$t8","$t9",				// $24-$25 ����
	"$k0","k1",					// $26-$27 ����ϵͳ���쳣������ 
	"$gp",						// $28 ȫ��ָ�� (Global Pointer)
	"$sp",						// $29 ��ջָ�� (Stack Pointer) ָ��ջ����һ��-4
	"$fp",						// $30 ָ֡��   (Frame Pointer)
	"$ra"						// $31 ���ص�ַ (Return Address)
};

bool is_digit(string str)
{
	if (isdigit(str[0]))						// ������
		return true;
	else if (str[0] == '-' && isdigit(str[1]))	// �Ǹ���
		return true;

	return false;
}

Assembly_Generator::Assembly_Generator(string out_path, const vector<Quatenary> input_quatenary)
{
	// 1. ������ļ�
	Asm_stream.open(out_path, ios::out);
	if (!Asm_stream.is_open())
	{
		cout << "ERROR: cannot open the assembly output file!" << endl;
		exit(EXIT_FAILURE);
	}

	// 2. �������ɵ���Ԫʽ
	quatenary = input_quatenary;

	// 3. �洢��ǩ��Ӧ��ַ
	for (Quatenary cur : quatenary)
		if (cur.operatorType[0] == 'j')					// j j> j< j= ...
			J_DestinationAddress.insert(cur.result);

	// 4. ��ʼ���Ĵ�����Ϣ�Լ��Ĵ���ֵ
	for (int i = 0; i < REGS_NUMBER; i++)
	{
		RegisterInfo[i] = { i, registers[i], 0 }; // {��ţ����֣�δʹ��ʱ��}
		RVALUE[i] = i ? "" : "0";						// 0�żĴ�����ֵΪ0�����಻��ֵ
	}
}

// ���������ɺ���ģ��
// ������Ԫʽ������ִ�в�ͬ�����ɺ�������
void Assembly_Generator::generate()
{
	// ����ʼǰ����Ҫ��main�������ָ�븳ֵ����ջָ��$sp��ջ��esp��ָ֡��$fp�����ص�ַebp
	// main������argc argv
	Asm_stream << "addi  $sp,$sp," << to_string(STACK_SEG_ADDR - 4 * paramsNumber - 8) << endl;
	Asm_stream << "addi  $fp,$fp," << to_string(STACK_SEG_ADDR - 4 * paramsNumber) << endl;

	// int j = 0;
	for (Quatenary quat : quatenary)
	{
		// cout << j++ << endl;
		// ����ÿһ����Ԫʽ�������Ĵ���δʹ��ʱ���һ
		for (int i = 0; i < REGS_NUMBER; ++i)
		{
			Reg* reg = &RegisterInfo[i];
			if (reg->unusedTime < INT32_MAX)
				++reg->unusedTime;
		}

		// �����Ҫ�����ǩ�������֮�����б�ǩ������"Label_ + ��ַ"����ʽ������
		if (J_DestinationAddress.find(to_string(quat.idx)) != J_DestinationAddress.end()) {
			if (quat.op1 == "-" && quat.op2 == "-" && quat.result == "-") // ����ǹ��̿鶨�壬����Ҫ
				;
			else
				Asm_stream << "Label_" << to_string(quat.idx) << " :" << endl;
		}

		// ����ÿһ����Ԫʽ�����ݲ����Ĳ�ͬ�����ò�ͬ�Ĵ�����
		string opType = quat.operatorType;
		if (opType == "call") {
			callTypeHandler(quat);
		}
		else if (opType == "return") {
			returnTypeHandler(quat);
		}
		else if (opType == "+" || opType == "-" || opType == "*" || opType == "/") {
			//if (AVALUE.find(quat.result) == AVALUE.end())
			//	AVALUE.insert({ quat.result, {} });    // ����ñ�������AVALUE�У������
			computeTypeHandler(quat);
		}
		else if (opType == "=") {
			//if (AVALUE.find(quat.result) == AVALUE.end())
			//	AVALUE.insert({ quat.result, {} });    // ����ñ�������AVALUE�У������
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

// ���������õĻ���������
void Assembly_Generator::callTypeHandler(Quatenary quat) 		// ��������
{
	// 1. ����ʱ����ջ֡ 2. ���غ���ʣ���ջ֡��һ������return����������
	
	// ջ֡�Ľṹ��									        fp��     sp��
	//			|    ..   |   0  -4  -8 -12 -16 -20 -24 -28 -32 -36 -40
	//		    |  params |  t0  t1  t2  t3  t4  t5  t6  t7  fp  ra 
	//													    (old)
	// ����UNIXϵͳ��ջ֡�ṹ

	// ��������ʱ����Ҫ����Ĵ����ֳ���������Ӳ���ֳ����������Ѿ���ǰ���param��Ԫʽѹջ
	// mips��û��push / popָ����ʹ��sw / lw��ʽʵ��
	{
		Asm_stream << "sw    $t0,($sp)   " << endl;	// �Ĵ���ѹջ
		Asm_stream << "sw    $t1,-4($sp) " << endl;
		Asm_stream << "sw    $t2,-8($sp) " << endl;
		Asm_stream << "sw    $t3,-12($sp)" << endl;
		Asm_stream << "sw    $t4,-16($sp)" << endl;
		Asm_stream << "sw    $t5,-20($sp)" << endl;
		Asm_stream << "sw    $t6,-24($sp)" << endl;
		Asm_stream << "sw    $t7,-28($sp)" << endl;
	}

	Asm_stream << "subi  $sp,$sp,32 " << endl; // 1. ����ջ֡
	Asm_stream << "sw    $ra,-4($sp)" << endl; // 2. ���淵�ص�ַ
	Asm_stream << "sw    $fp,($sp)  " << endl; // 3. �����ջָ��
	Asm_stream << "move  $sp,$fp    " << endl; // 4. ���¶�ջָ��
	Asm_stream << "subi  $sp,$sp,8  " << endl; // 5. ����ջ֡

	Asm_stream << "jal   "<< quat.op1 << endl;

	// ��ת���غ��ָ��
	Asm_stream << "addiu $sp,$sp,8  " << endl;

	{
		Asm_stream << "lw    $t7,4($sp) " << endl;	// �Ĵ�����ջ���ָ��ֳ�
		Asm_stream << "lw    $t6,8($sp) " << endl;
		Asm_stream << "lw    $t5,12($sp)" << endl;
		Asm_stream << "lw    $t4,16($sp)" << endl;
		Asm_stream << "lw    $t3,20($sp)" << endl;
		Asm_stream << "lw    $t2,24($sp)" << endl;
		Asm_stream << "lw    $t1,28($sp)" << endl;
		Asm_stream << "lw    $t0,32($sp)" << endl;
	}
	
	// �ָ�sp����һ֡��ջ��������32�ǰ˸��Ĵ�����ƫ�ƣ�ʣ�µ��Ǵ��������ƫ��
	Asm_stream << "lw    $sp," << to_string(32 + paramsNumber * 4) << "($fp)  " << endl;
	Asm_stream << "lw    $fp,($fp) " << endl;  // �ָ�fp����һ֡��old fpλ�ã�����UNIX�е�old ebp��

	if (quat.result != "-")   // �������ʱ�����˱������շ���ֵ�����ǵ�������void����������շ���ֵ��
	{
		int returnReg = AVALUE[quat.result];
		// �����㣬˵���ڼĴ����У��ŵ��Ĵ����Ｔ��
		if (returnReg <= 0) {
			// Ϊ�㣬˵��û������������Ҫ����һ���Ĵ����������±���ʲ����ڵĻ���ֱ�Ӳ��룬���������ٴβ���֮��
			// Ϊ-1��˵���ŵ����ڴ� ���� ����û��ϵ������ҲҪ���ǣ�ֱ�ӷ���Ĵ����ͺ��ˣ�
			returnReg = allocateReg(quat.result, "-", "-");
		}
		Asm_stream << "move  " + registers[returnReg] + ", $v1" << endl;

		paramsNumber = 0;
	}
}

void Assembly_Generator::returnTypeHandler(Quatenary quat) 	// ����
{
	// 1. ���淵��ֵ 2. ��������ջ֡
	if (quat.op1 != "-")  // ����з���ֵ��������ֵѹջ��������Ҫѹջ
		Asm_stream << "move  $v1," + registers[AVALUE[quat.op1]] << endl;

	// �ͷžֲ�����ռ�õļĴ������޸�AVALUE��RVALUE
	for (auto localVar = localVariantAddressTable.begin(); localVar != localVariantAddressTable.end(); ++localVar)
	{
		int reg = AVALUE[localVar->first];
		if (reg > 0)						// ������ռ�õļĴ������
			RVALUE[reg] = "";
		AVALUE.erase(localVar->first);
	}

	Asm_stream << "move  $sp,$fp    " << endl;
	Asm_stream << "subi  $sp,$sp,8  " << endl;
	Asm_stream << "lw    $ra,-4($fp)" << endl;   // ���ص�ַ����$ra

	// �������main��������ת
	if (procedureName != "main")
		Asm_stream << "jr    $ra" << endl;
	else
		Asm_stream << "break" << endl;

	localVarOffset = LOCAL_VAR_OFFSET;
	localVariantAddressTable.clear();
	procedureName = "";
}

void Assembly_Generator::jTypeHandler(Quatenary quat) 		// ��ת������j j< j> j= j<= j>=��
{
	if (quat.operatorType == "j")
	{
		//Asm_stream << "j ";

		//// ��Ԫʽ��������ַ������֣�˵��Ϊ������������Ϊ��ǩ��ַ�����ں�����������ת����������ת���������������
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

		// ���A������������A����$t8�������ҵ�A���ڵļĴ�����ΪA����Ĵ���
		if (isdigit(quat.op1[0])) {
			Asm_stream << "addi  $t8,$zero," << quat.op1 << endl;
			A_Reg = 24;
		}
		else {
			A_Reg = AVALUE[quat.op1];
			if (A_Reg == -1)  // ˵����Ҫ���������ڴ滻��
				A_Reg = loadVar(quat.op1);
			else if (A_Reg == 0)
				A_Reg = allocateReg(quat.op1);
		}

		// Bͬ��
		if (isdigit(quat.op2[0])) {
			Asm_stream << "addi  $t9,$zero," << quat.op2 << endl;
			B_Reg = 25;
		}
		// �����ҵ�A���ڵļĴ��� / ΪA��ռ�Ĵ���
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

// ���㣨����+ - * /���Ļ���������
void Assembly_Generator::computeTypeHandler(Quatenary quat) 	
{
	// 1. ����Ĵ�������B��C���ڴ滻��
	
	// A := B +-*/ C 
	// ����AVALUEΪ-1�ı���B/C�������ڴ��Ƿ������������Ӧ�����еģ�����Ҫlwָ�����Ĵ���
	int A_Reg = AVALUE[quat.result];
	int B_Reg = AVALUE[quat.op1];
	int C_Reg = -1; // C��������������Ҫ���жϣ���B��������������

	if (B_Reg == -1)  // ˵��B��ֵ���ڴ棬��Ҫ����
		B_Reg = loadVar(quat.op1, A_Reg, C_Reg);
	else if (B_Reg == 0) // ˵��û�г��ֹ�B�����������B������ȫ�ֱ���������ȫ�ֱ�������
	{
		// �Ƚ�ȫ�ֱ���Bע�ᵽ����
		globalVariantAddressTable.insert({ quat.op1, localVarOffset });
		globalVarAddr += 4; // ��������

		// ΪB����һ���Ĵ���
		B_Reg = allocateReg(quat.op1, (A_Reg > 0) ? quat.result : "-");
	}

	if (!is_digit(quat.op2)) // C����������������Ҫ���ǼĴ�������
	{
		C_Reg = AVALUE[quat.op2];
		if (C_Reg == -1)  // ˵��C��ֵ���ڴ棬��Ҫ����
			C_Reg = loadVar(quat.op2, A_Reg, B_Reg);
		RegisterInfo[C_Reg].unusedTime = 0;
	}

	//if (A_Reg == -1) // ������ڴ棬��Ҫ�����Ĵ�����
	//	A_Reg = loadVar(quat.result, B_Reg, C_Reg);
	if (A_Reg <= 0) // ��һ��ʹ�ã���Ҫ����Ĵ���
	{
		if(is_digit(quat.op2))
			A_Reg = allocateReg(quat.result, quat.op1);
		else 
			A_Reg = allocateReg(quat.result, quat.op1, quat.op2);
	}

	// 2. ��д�Ӽ��˳��Ļ��
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
	else if (quat.operatorType == "*") // mips��multָ���Ƚ�������浽{hi��lo}������ʹ��mfhi&mfloȡ�����˴�ʹ��mulָ���
	{
		if (is_digit(quat.op2)) // MIPS��û���������˷��������Ȱ�������C��ֵ��һ����ʱ�Ĵ���
		{
			Asm_stream << "addi  $t8,$zero," << quat.op2 << endl;
			C_Reg = 24;
		}
		Asm_stream << "mul   " << registers[A_Reg] << "," << registers[B_Reg] << "," << registers[C_Reg] << endl;
	}
	else if (quat.operatorType == "/") 
	{		
		if (is_digit(quat.op2)) // MIPS��û���������˷��������Ȱ�������C��ֵ��һ����ʱ�Ĵ���
		{
			Asm_stream << "addi  $t8,$zero," << quat.op2 << endl;
			C_Reg = 24;
		}
		Asm_stream << "div   " << registers[B_Reg] << "," << registers[C_Reg] << endl;
		Asm_stream << "mov   " << registers[A_Reg] << ",$lo" << endl;
	}

	RegisterInfo[B_Reg].unusedTime = 0;
}

void Assembly_Generator::assignTypeHandler(Quatenary quat) 	// ��ֵ
{
	// A := B
	// ��ֵ��������Ҫ�����������ͱ���������������ڱ���������Ҫ���������ڴ滻��
	int A_Reg = AVALUE[quat.result];
	int B_Reg = -1;

	if (!is_digit(quat.op1))		// �����������������Ҫ���ǼĴ�����������
		B_Reg = AVALUE[quat.op1];   // ˵��B��ֵ���ڴ棬������Ҫ���룬ʹ��lwָ���

	if (A_Reg <= 0)						// Ϊ�㣬˵�����±���������һ���Ĵ����������ܺ�B��ͻ
	{
		if (is_digit(quat.op1))
			A_Reg = allocateReg(quat.result);
		else
			A_Reg = allocateReg(quat.result, quat.op1);
	}
	//else if (A_Reg == -1)			// Ϊ-1��˵��ֵ���ڴ棬��Ҫ���루ʵ���ϲ���Ҫ���룬��Ϊ�������ǣ���Ҫ����һ���Ĵ�����
	//	A_Reg = loadVar(quat.result, B_Reg);

	// ���и�ֵ������������
	if (is_digit(quat.op1))
		Asm_stream << "addi  " << registers[A_Reg] << ",$zero," << quat.op1 << endl;
	else if (B_Reg != -1) // ���B�ڼĴ����У�ʹ��moveָ��
		Asm_stream << "move  " << registers[A_Reg] << "," << registers[B_Reg] << endl;
	else // ���B���ڴ��У�ʹ��lwָ��
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

void Assembly_Generator::paramTypeHandler(Quatenary quat) 	// ʵ������
{
	// ʵ������ʱ������Ҫ��Ϊ������ֵѹ��ջ֡

	// ջ֡�Ľṹ��			sp��
	//			|    ..   |   0  -4  -8 -12 
	//		    |  locals |  p1  p2  p3  ...
	// ����UNIXϵͳ��ջ֡�ṹ

	// 1. ׼��������ȷ�������ڼĴ�����
	int reg = AVALUE[quat.op1];
	if (reg == -1) // �������ڼĴ����У�������ڴ滻���Ĵ�����
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

	// 2. ����ѹջ������
	Asm_stream << "sw    " << registers[reg] << ",0($sp)" << endl;
	Asm_stream << "subi  $sp,$sp,4" << endl;

	RegisterInfo[reg].unusedTime = 0;
	++paramsNumber;  // Ϊ����call��������ʱ������ָ��Ļָ���λ�ṩƫ��
}

void Assembly_Generator::defparTypeHandler(Quatenary quat) 	// �β�����
{
	// ջ֡�Ľṹ��									        fp��     sp��
	//			|    ..   |   0  -4  -8 -12 -16 -20 -24 -28 -32 -36 -40
	//		    |  params |  t0  t1  t2  t3  t4  t5  t6  t7  fp  ra 
	//													    (old)
	// ����UNIXϵͳ��ջ֡�ṹ
	
	// Ϊ�β��ҵ���Ӧ��ʵ�ε�ַƥ��
	localVariantAddressTable.insert({ quat.result, 4 * (paramsNumber + 1 + 8) }); // +8 ָ�˸��Ĵ����ֳ�
	++paramsNumber;
	AVALUE.insert({quat.result, -1}); // �βζ����ڴ��У���˲���-1
}

void Assembly_Generator::defprocTypeHandler(Quatenary quat) 	// ���̿�����
{
	paramsNumber = 0;
	procedureName = quat.operatorType;
	localVariantAddressTable.clear();
	localVarOffset = LOCAL_VAR_OFFSET;

	//���̱��
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

	// ���B������ֵ��ĳ���Ĵ���Ri�У�RVALUE[Ri]��ֻ����B��
	// ���⣬����B��A��ͬһ����ʶ��������B������ֵ��ִ����ԪʽA:=B op C֮�󲻻������ã�
	// ��ѡȡRiΪ����Ҫ�ļĴ���R
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

	// �������δ����ļĴ����������ѡȡһ��RiΪ����Ҫ�ļĴ���R
	for (int i = 8; i <= 15; i++)
	{
		if (RVALUE[i].empty())	// ���Ϊ�գ�˵������
		{
			AVALUE[A] = i;
			RVALUE[i] = A;		// ����Ĵ������޸�RVALUE��AVALUE
			RegisterInfo[i].unusedTime = 0;
			return i;
		}
	}

	int seized_reg_index = LRU_GetRegister(B_Reg, C_Reg);    // ���û��B��C�����Ӧ��regΪ-1����Ӱ�����ﻻ�Ĵ���
	string M = RVALUE[seized_reg_index];	// ��ñ���ռ�ı�������

	if (M == A && M != B && M != C)			// ���M��A���Ҳ���B��C������Ҫ����
		;
	else
		saveVar(M);

	// ����AVALUE��RVALUE
	AVALUE[M] = -1;					// -1��˵���Ѿ��������ڴ���
	AVALUE[A] = seized_reg_index;
	RVALUE[seized_reg_index] = A;

	RegisterInfo[seized_reg_index].unusedTime = 0;
	return seized_reg_index;
}

void Assembly_Generator::saveVar(string var)
{
	int reg_index = AVALUE[var];

	// ��var�����ڴ��У�����Ҫ�ȷ����ڴ�ռ�
	if (localVariantAddressTable.find(var) == localVariantAddressTable.end() && 
		globalVariantAddressTable.find(var) == globalVariantAddressTable.end())
	{
		// ���̿�����Ϊ""��˵����ȫ�ֱ����������Ǿֲ�����
		if (procedureName != "")
		{
			localVariantAddressTable.insert({ var, localVarOffset });
			localVarOffset -= 4; // ��������
			Asm_stream << "subi  $sp,$sp,4" << endl; // �ֲ���������ջ��
		}
		else
		{
			globalVariantAddressTable.insert({ var, globalVarAddr });
			globalVarAddr += 4; // ��������
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
	// 1. ���ҿ��Ի��ļĴ��������Ĵ�����ԭ�������ݣ�����У������ڴ�

	int reg_index = 0;	// ����ѡһ���Ĵ�������Ŵ�������

	// �������δ����ļĴ����������ѡȡһ��RiΪ����Ҫ�ļĴ���R
	for (int i = 8; i <= 15; i++)
	{
		if (RVALUE[i].empty())	// ���Ϊ�գ�˵������
		{
			reg_index = i;
			break;
		}
	}

	if (!reg_index)  // û�п��мĴ���������ռһ��
	{
		reg_index = LRU_GetRegister(except1, except2);

		string M = RVALUE[reg_index];	// ��ñ���ռ�ı�������
		saveVar(M);						// ������ռ�ı����浽�ڴ�
		AVALUE[M] = -1;					// -1��˵���Ѿ��������ڴ���
	}
	
	// 2. �ж���ȫ�ֱ������Ǿֲ����������lwȡ��ָ��������������Ĵ�����

	AVALUE[var] = reg_index;
	RVALUE[reg_index] = var;		// ����Ĵ������޸�RVALUE��AVALUE
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
		int max_unusedTime = -1;		// ͨ��LRU�㷨���õ���Ҫ��ռ�ļĴ���
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
		cout << "û���ҵ�����ļ�" << endl;
		exit(EXIT_FAILURE);
	}

	string s;
	while (getline(AssemblyFile, s))
	{
		cout << s << endl;
	}
	AssemblyFile.close();             //�ر��ļ�������
}

Assembly_Generator::~Assembly_Generator()
{
	Asm_stream.close();
}