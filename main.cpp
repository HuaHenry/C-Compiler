#include <iostream>
#include <iomanip>
#include <sstream>
#include "1_Lexical_PreProcess.h"
#include "2_Grammatical_Analysis.h"
#include "2_Grammatical_LR1Grammer.h"
#include "4_Assembly_Generator.h"

using namespace std;

extern void lexAnalysis(const string path);           // �ʷ������ӿں���
extern vector<TOKEN>lexAnswer;						  // 1���ʷ�����������
map<pair<int, string>, pair<string, int>> LR1Table;   // 2��LR1������
std::vector<productItem>productionItems;              // 3������ʽ
SemanticAnalysis semantic;							  // 4���������
string START_SIGN = "Program";// ԭ�ķ�����ʼ��


// ��ӡϵͳͷ����Ϣ
void printInfo_start() {
	system("cls");
	cout << endl;
	cout << "  �������������[     �������������[ �������������[ �������[   �������[�������������[ �����[�����[     ���������������[�������������[ " << endl;
	cout << " �����X�T�T�T�T�a    �����X�T�T�T�T�a�����X�T�T�T�����[���������[ ���������U�����X�T�T�����[�����U�����U     �����X�T�T�T�T�a�����X�T�T�����[" << endl;
	cout << " �����U         �����U     �����U   �����U�����X���������X�����U�������������X�a�����U�����U     �����������[  �������������X�a" << endl;
	cout << " �����U         �����U     �����U   �����U�����U�^�����X�a�����U�����X�T�T�T�a �����U�����U     �����X�T�T�a  �����X�T�T�����[" << endl;
	cout << " �^�������������[    �^�������������[�^�������������X�a�����U �^�T�a �����U�����U     �����U���������������[���������������[�����U  �����U" << endl;
	cout << "  �^�T�T�T�T�T�a     �^�T�T�T�T�T�a �^�T�T�T�T�T�a �^�T�a     �^�T�a�^�T�a     �^�T�a�^�T�T�T�T�T�T�a�^�T�T�T�T�T�T�a�^�T�a  �^�T�a" << endl;
	
	cout << endl << "                      ��C���Ա����� -- 2151127 ������                      " << endl << endl;
	cout << "------------------------------- �� �� �� �� -------------------------------" << endl;
	cout << "                                                               " << endl;
	cout << "�߼����Գ���sourceProgram.c                       " << endl;
	cout << "�﷨�ļ���    grammer.txt                           " << endl;
	cout << "                                                               " << endl;
	cout << "------------------------------- �� �� �� �� -------------------------------" << endl;
	cout << "                                                               " << endl;
	cout << "�ʷ����������1_Lexical_AnalysisAnswer.txt         " << endl;
	cout << "�﷨�����ķ���2_Grammatical_Grammer.txt            " << endl;
	cout << "LR1������   2_Grammatical_LR1Table.txt           " << endl;
	cout << "�﷨���ű���  syntaxTree.dot						" << endl;
	cout << "�﷨��ͼƬ��  syntaxTree.png						" << endl;
	cout << "�м���Ԫʽ��  3_Semantic_Quaternary.txt			" << endl;
	cout << "                                                               " << endl;
	cout << "---------------------------------------------------------------------------" << endl;
	cout << "                                                               " << endl;
	system("pause");
}

// �ʷ�������ӡ��Ԫ�麯��
void lexAnalysis_printRes(string ch) {
	if (ch == "y" || ch == "Y") {
		for (TOKEN tempPair : lexAnswer) {                          // ��ӡ�ʷ������õ����ķ���������
			cout << setw(15) << tempPair.token_key << " " << setw(15) << tempPair.value <<
				setw(10) << " ROW:" << setw(3) << tempPair.row << setw(10) << " COL:" << setw(3) << tempPair.col << endl;
		}
	}
}

// �ʷ��������ƺ���
void lexAnalysis_controller() {
	system("cls");
	cout << "------------------- STEP1 �ʷ����� --------------------" << endl;
	cout << "���ڽ��дʷ���������" << endl;
	lexAnalysis("../infiles/sourceProgram.c");               // �ʷ�����
	string ch;
	cout << "�ʷ������ɹ���" << endl;
	cout << "�Ƿ���Ҫ��ӡ�ķ��������е���Ļ�� y/Y - �� ";
	cin >> ch;
	lexAnalysis_printRes(ch);
	system("pause");
}

// �﷨�������ƺ���
void GrammaticalAnalysis_controller() {
	system("cls");
	cout << "----------------- STEP2 �﷨���� ------------------" << endl;
	LR1_grammer g;
	cout << "���ڶ�ȡ��C�����ķ�����" << endl;
	g.grammer.readGrammer("../infiles/grammer.txt");        // ����C�����ķ��ķ�
	g.grammer.printGrammer("../outfiles/2_Grammatical_Grammer.txt");   // ��ӡ�ķ���������ļ���
	cout << "��������FIRST������" << endl;
	g.generateFirst();                                       // �������з��ŵ�first�� 
	cout << "����������Ŀ���塭��" << endl;
	g.generateClosureFamily();                               // ������Ŀ���壨�����л�����LR1��������ƽ����ֺ�GoTo���֣�
	g.fillGuiYueTable();								     // ��дLR1������Ĺ�Լ����
	
	LR1Table = g.LR1_table;
	productionItems = g.grammer.productItems;

	cout << "��������LR1��������" << endl;
	g.printLR1Table("../outfiles/2_Grammatical_LR1Table.txt");                // ��ӡLR1������

	// �﷨������ִ�й�Լ��+ �﷨������
	ThreadedList* Root = grammaticalAnalysis(lexAnswer, LR1Table, productionItems, semantic);
	if (!Root) {
		cout << "ERROR: ��Լʧ��!" << endl;
		exit(EXIT_FAILURE);
	}
	else {
		cout << "��Լ�ɹ�!" << endl;
	}
	cout << "���ڻ����﷨������" << endl;
	generateTreeDot(Root);            // �����﷨��
	cout << "�﷨�����ɹ���" << endl;
	system("pause");
}

// ����������ƺ���
void SemanticAnalysis_controller() {
	system("cls");
	cout << "------------------- STEP3 ������� -------------------" << endl;
	//���﷨�������Ѿ�ִ�����������������˴�������ʾ���
	cout << "��������ɹ���" << endl;
	cout << "�м�������ɳɹ���" << endl;
	semantic.PrintQuaternary("../outfiles/3_Semantic_Quaternary.txt");
	cout << "�Ƿ��ӡ��Ԫʽ�������Ļ�� y/Y - �� ";
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

// ����������
void AssemblyGeneration_controller() {
	system("cls");
	cout << "------------------ STEP4 Assembly Generation ------------------" << endl;
	cout << "���ڽ��л���������" << endl;
	ifstream quat_stream;
	quat_stream.open("../outfiles/3_Semantic_Quaternary.txt", ios::in);

	Assembly_Generator generator("../outfiles/4_Assembly.asm", semantic.quaternary);
	// Assembly_Generator generator("../outfiles/4_Assembly.asm", debug);
	generator.generate();
	cout << "���������ɳɹ���" << endl;
	cout << "---------------------------------------------------------------" << endl;
	generator.Print_Assembly("../outfiles/4_Assembly.asm");
	cout << "---------------------------------------------------------------" << endl;
	system("pause");
}

// ����ɹ��󷵻���ҳ��ӡ��Ϣ
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
	cout << endl << endl << "����ɹ�" << endl;
	system("pause");
}

int main()
{
	// ��ӡϵͳ��Ϣ
	printInfo_start();
	
	// �ʷ�����
	lexAnalysis_controller();

	// �﷨����
	GrammaticalAnalysis_controller();

	// �������
	SemanticAnalysis_controller();

	// ����������
	AssemblyGeneration_controller();
	
	// ������ҳ
	printInfo_end();

	return 0;
}



