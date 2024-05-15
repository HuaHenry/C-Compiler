#include "1_Lexical_PreProcess.h"
#include <fstream>
#include <string>
/*******************************************************************/
//工具函数
/*******************************************************************/
//删除str两侧的空格
void delSideBlank(string& str) 
{
	int length = str.size();
	int left = 0;
	int right = length-1;
	for (; left < length; left++) {
		if (str[left] != ' ')
			break;
	}
	for (; right >= 0; right--) {
		if (str[right] != ' ')
			break;
	}
	str = str.substr(left,right-left+1);
}

//将str根据seperator划分到substr中
void divideRightStr(string str, char seperator,vector<string>& substr)
{
	int length = str.size();
	string temp;
	for (int i = 0; i < length; i++) {
		if (str[i] == seperator) {
			delSideBlank(temp);//去除左右两端的空格
			substr.push_back(temp);
			temp.clear();
			continue;
		}
		temp += str[i];
	}
	delSideBlank(temp);
	substr.push_back(temp);

}

/*******************************************************************/
//CGrammer类成员函数实现
/*******************************************************************/
CGrammer::CGrammer()
{
	//empty
}

//读取文件中的文法
void CGrammer::readGrammer(string filename)
{
	ifstream infile;
	infile.open(filename);
	if (!infile) {
		cout << "文件打开失败!" << endl;
		exit(-1);
	}

	string line, left, right;
	vector<string> left_and_right;
	while (getline(infile, line, '\n')) {
		//消除注释
		if (line[0] == EXEG_SIGN || line.size() == 0) {
			line.clear();
			continue;
		}
		right.clear();
		left.clear();
		left_and_right.clear();
		
		divideRightStr(line, DERI_SIGN, left_and_right);	//分为左右两部分
		if (left_and_right.size() != 2) {
			cout << "产生式格式不正确!" << endl;
			exit(-1);
		}
		left = left_and_right[0];
		right = left_and_right[1];

		//读取终结符
		if (left == "%VT") {
			vector<string> temp;
			divideRightStr(right, SEPE_SIGN, temp);
			for (vector<string>::iterator iter = temp.begin(); iter < temp.end(); iter++)
			{
				if (*iter != "")
					Terminals.insert(*iter);
			}
		}
		//读取产生式
		else {
			vector<string> temp;
			divideRightStr(right, SEPE_SIGN, temp);
			int len = temp.size();//产生式个数
			for (int i = 0; i < len; i++) {
				productItem tempItem;
				tempItem.left_symbol = left;
				divideRightStr(temp[i], AND_SIGN, tempItem.right_symbol);
				productItems.push_back(tempItem);//插入一个产生式
				if (left.size() == 1 && left[0] == INIT_SIGN)
					startItemPos = productItems.size() - 1;//起始产生式的角标
			}
			//插入非终结符
			NTterminals.insert(left);
		}
		line.clear();
	}
	infile.close();
}


//打印文法到文件中，测试用
void CGrammer::printGrammer(string filename) {
	ofstream outfile;
	outfile.open(filename);
	if (!outfile) {
		cout << "文件打开失败!" << endl;
		exit(-1);
	}
	outfile << "起始产生式位置：" << this->startItemPos << endl;
	outfile << "终结符集 = { ";
	for (auto str : this->Terminals) {
		outfile << str << " ";
	}
	outfile << "}" << endl;

	outfile << "非终结符集 = { ";
	for (auto str : this->NTterminals) {
		outfile << str << " ";
	}
	outfile << "}" << endl;

	outfile << "产生式：" << endl;
	int i = 0;
	for (auto pItem : this->productItems) {
		outfile << i << "：";
		i++;
		outfile << pItem.left_symbol << "-->";
		for (auto str : pItem.right_symbol) {
			outfile << str << " ";
		}
		outfile << endl;
	}
}
