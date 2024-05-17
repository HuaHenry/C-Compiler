#include "2_Grammatical_LR1Grammer.h"
#include <fstream>
#include <iomanip>

bool LR1_item::operator==(const LR1_item& item)
{
	if (this->left_symbol != item.left_symbol) return false;
	if (this->dotPos != item.dotPos)return false;
	if (this->seek_symbol != item.seek_symbol)return false;
	if (this->right_symbol.size() != item.right_symbol.size())return false;
	for (int i = 0; i < this->right_symbol.size(); i++) {
		if (this->right_symbol[i] != item.right_symbol[i])
			return false;
	}
	return true;
}

bool LR1_item::operator<(const LR1_item& item) const
{
	if (this->left_symbol != item.left_symbol)
		return this->left_symbol < item.left_symbol;
	else {
		if (this->right_symbol.size() != item.right_symbol.size())
			return this->right_symbol.size() < item.right_symbol.size();
		else {
			for (int i = 0; i < this->right_symbol.size(); i++) {
				if (this->right_symbol[i] != item.right_symbol[i])
					return this->right_symbol[i] < item.right_symbol[i];
			}
		}
		if (this->seek_symbol != item.seek_symbol)
			return this->seek_symbol < item.seek_symbol;
		else
			return this->dotPos < item.dotPos;
	}
}

//打印项目集
void LR1_grammer::printItems(set<LR1_item> inItems) {
	for (auto pitem : inItems) {
		cout << pitem.left_symbol << " -> ";
		int i = 0;
		for (auto r : pitem.right_symbol) {
			if (i++ == pitem.dotPos)
				cout << "·";
			cout << r << " ";
		}
		if(pitem.dotPos==pitem.right_symbol.size())
			cout << " · ";
		cout << " --- " << pitem.seek_symbol << endl;

	}
	cout << "/****************************/" << endl;
}


// 判断是否为两个相同的闭包
bool LR1_grammer::isSameClosure(set<LR1_item>closure1, set<LR1_item>closure2) {
	if (closure1.size() != closure2.size())
		return false;
	int count = 0;
	for (auto tmp : closure1) {
		for (auto tmp_ : closure2) {
			if (tmp == tmp_) {
				++count;
				break;
			}
		}
	}
	return count == closure1.size();
}

// 生成下一个项目集（状态）
// 求当前输入符号为presentStr时，状态fromItems转去的状态toItems
void LR1_grammer::generateNextClosure(set<LR1_item> fromItems, set<LR1_item>& toItems, string presentStr) {
	if (presentStr == "#") // 不考虑#
		return;
	for (auto lItem : fromItems) {   // 扫描项目集fromItems的每一个项目
		if (lItem.dotPos >= lItem.right_symbol.size()) // dot在末尾
			continue;
		if (lItem.right_symbol[lItem.dotPos] != presentStr) // dot后一个符号不是presentStr
			continue;
		// dot 后是presentStr
		LR1_item temp = lItem;
		temp.dotPos = lItem.dotPos + 1;  // dot后移一位
		toItems.insert(temp);   // 把该项目插入到新的项目集
	}
	getClosure(toItems, toItems);  // 求新项目集的闭包
}

// 生成项目集族
void LR1_grammer::generateClosureFamily() {
	set<LR1_item> startClosure;   // 初始项目集
	LR1_item startItem;           // 初始项目
	productItem startProductItem; // 初始产生式
	startProductItem = this->grammer.productItems[this->grammer.startItemPos];
	
	// 根据初始产生式构造初始项目
	startItem.left_symbol = startProductItem.left_symbol;
	for (auto str : startProductItem.right_symbol) {
		startItem.right_symbol.push_back(str);
	}
	startItem.dotPos = 0;
	startItem.seek_symbol = "#";
	
	startClosure.insert(startItem);      // 将初始项目加入到初始项目集
	this->getClosure(startClosure, startClosure);  // 求初始项目的闭包
	closureFamily.push_back(startClosure); // 将初始项目集的闭包加入项目集族
	
	// 遍历项目集族中的每一个项目集
	for (int i = 0; i < closureFamily.size(); i++) {
		// 对每个项目集 遍历所有终结符
		for (auto str : this->grammer.Terminals) {
			if (str == "#")  // 显然不需要考虑 #
				continue;
			
			set<LR1_item> tempClosure;  // 输入str，当前状态会转移到下一个项目集tempClosure
			generateNextClosure(closureFamily[i], tempClosure, str); // 生成下一个项目集的闭包
			if (tempClosure.empty())   // 下一个项目集为空，说明当前项目集不存在遇到str的转移
				continue;

			// 检查项目集族中是否已经存在相同的状态
			int existIndex = -1;
			for (int j = 0; j < closureFamily.size(); j++) {
				if (isSameClosure(closureFamily[j], tempClosure)) {
					existIndex = j;
					break;
				}
			}
			if (existIndex != -1) {   // 存在相同状态则不需要插入
				pair<int, string>in = pair<int, string>(i, str);
				if (LR1_table.count(in) == 0) {   // 检查Action表的对应移进动作是否有冲突
					// 填写LR1分析表中的移进动作
					LR1_table[pair<int, string>(i, str)] = pair<string, int>("s", existIndex);
				}
				else 
					cout << "ERROR:入口冲突!";
			}
			else {   // 不存在相同状态则需要插入
				closureFamily.push_back(tempClosure);// 将下一个项目集的闭包加入项目集族

				pair<int, string>in = pair<int, string>(i, str);
				if (LR1_table.count(in) == 0) {   // 检查Action表的对应移进动作是否有冲突
					// 填写LR1分析表中的移进动作
					LR1_table[pair<int, string>(i, str)] = pair<string, int>("s", closureFamily.size() - 1);
				}
				else 
					cout << "ERROR:入口冲突!";
			}
		}
		// 对每个项目集 遍历所有非终结符
		for (auto str : this->grammer.NTterminals) {
			set<LR1_item> tempClosure;  // 输入str，当前状态会转移到下一个项目集tempClosure
			generateNextClosure(closureFamily[i], tempClosure, str);  // 生成下一个项目集的闭包
			if (tempClosure.empty())   // 下一个项目集为空，说明当前项目集不存在遇到str的转移
				continue;

			// 检查项目集族中是否已经存在相同的状态
			int existIndex = -1;
			for (int j = 0; j < closureFamily.size(); j++) {
				if (isSameClosure(closureFamily[j], tempClosure)) {
					existIndex = j;
					break;
				}
			}
			if (existIndex != -1) {  // 存在相同状态则不需要插入
				pair<int, string>in = pair<int, string>(i, str);  
				if (LR1_table.count(in) == 0) {   // 检查GOTO表的对应转移动作是否有冲突
					// 填写LR1分析表中的移进动作
					LR1_table[pair<int, string>(i, str)] = pair<string, int>("null", existIndex);
				}
				else 
					cout << "ERROR:入口冲突!";
			}
			else {  // 不存在相同状态则需要插入
				closureFamily.push_back(tempClosure);  // 将下一个项目集的闭包加入项目集族

				pair<int, string>in = pair<int, string>(i, str);
				if (LR1_table.count(in) == 0) {   // 检查GOTO表的对应转移动作是否有冲突
					// 填写LR1分析表中的移进动作
					LR1_table[pair<int, string>(i, str)] = pair<string, int>("null", closureFamily.size() - 1);
				}
				else 
					cout << "ERROR:入口冲突!";
			}
		}
	}
}

// 求所有文法符号的FIRST集
void LR1_grammer::generateFirst() {
	// 求终结符的first集
	for (auto str : grammer.Terminals) {
		// 终结符的FIRST集是它自身
		set<string> firstSet;
		firstSet.insert(str);
		this->FIRST[str] = firstSet;
	}
	
	// 求非终结符的first集
	bool changed;
	while (true) {
		changed = false;
		// 遍历所有非终结符
		for (auto ntStr : grammer.NTterminals) {
			// 遍历所有产生式
			for (auto pItem : grammer.productItems) {
				// 如果产生式左边不为当前非终结符str，则跳过此产生式
				if (pItem.left_symbol != ntStr)
					continue;

				// 找到可用产生式，遍历产生式右部
				int pos = 0;

				// 产生式右部第一个符号是终结符（包括epsilon）
				if (grammer.Terminals.find(pItem.right_symbol[pos]) != grammer.Terminals.end()) {
					changed = this->FIRST[ntStr].insert(pItem.right_symbol[pos]).second || changed;
					continue;
				}

				// 产生式右部以非终结符开始
				bool flag = true; // 可推导出空串的标志
				for (; pos < pItem.right_symbol.size(); pos++) {
					// 如果是终结符，则停止迭代
					if (grammer.Terminals.find(pItem.right_symbol[pos]) != grammer.Terminals.end()) {
						flag = false;
						changed = this->FIRST[ntStr].insert(pItem.right_symbol[pos]).second || changed;
						break;
					}

					// 是非终结符，则把该非终结符除epsilon以外的first集合并到first[ntStr]
					set<string> temp = this->FIRST[pItem.right_symbol[pos]];
					temp.erase("@");   //删除epsilon
					int size1 = this->FIRST[ntStr].size();
					this->FIRST[ntStr].insert(temp.begin(), temp.end());
					int size2 = this->FIRST[ntStr].size();
					changed = size2 > size1 ? (changed || 1) : (changed || 0);   // first集是否发生变化

					// 该非终结符是否能推出空串，如是，则继续迭代
					flag = flag && this->FIRST[pItem.right_symbol[pos]].find("@") != this->FIRST[pItem.right_symbol[pos]].end();

					if (!flag)
						break;
				}
				
				// 产生式右部所有符号都可以推出非终结符，则first集插入Epsilon
				if (flag && pos == pItem.right_symbol.size())
					changed = this->FIRST[ntStr].insert("@").second || changed;
			}
		}
		
		//如果没有改变，说明求first集结束，退出
		if (!changed)
			break;
	}
}

// 求闭包
void LR1_grammer::getClosure(set<LR1_item> inItems, set<LR1_item>& outItems){
	// cout << "***输入集合：" << endl;
	// printItems(inItems); //////////////////////////////////////////////////////////////////////////////////////暂时注释
	//闭包包含所有inItems中的项目
	vector<LR1_item> outvec;
	for (auto temp : inItems)
		outvec.push_back(temp);
	//outItems = inItems;
	int item_size = inItems.size();
	//对inItems的每一个形如S->a.Bc ，x 的产生式
	for (int pitem = 0; pitem < outvec.size();pitem++) {
		//iter->right_symbol[iter->dotPos]：点后的符号
		//归约项·在末尾 直接跳过
		if (outvec[pitem].dotPos == outvec[pitem].right_symbol.size())
			continue;
		//如果是空项，则转为归约项目
		if (outvec[pitem].right_symbol[0][0] == grammer.NULL_SIGN) {
			outvec[pitem].dotPos++;
			continue;
		}
		//其余项，检查·后面是不是NT
		if (grammer.NTterminals.find(outvec[pitem].right_symbol[outvec[pitem].dotPos]) != grammer.NTterminals.end()) {
			vector<string> cx;
			for (int i = outvec[pitem].dotPos + 1; i < outvec[pitem].right_symbol.size(); i++) {
				cx.push_back(outvec[pitem].right_symbol[i]);
			}
			cx.push_back(outvec[pitem].seek_symbol);
			for (auto pgram : grammer.productItems) {
				//寻找以pitem.right_symbol[pitem.dotPos]为左部的产生式
				if (pgram.left_symbol == outvec[pitem].right_symbol[outvec[pitem].dotPos]) {
					//对文法productItems的每一个产生式B->b
					set<string> colFirstSet = getStrFirstSet(cx);
					//{cx}的FIRST集的每一个元素t，将B->b，t 加入 outItems
					LR1_item temp;
					temp.left_symbol = pgram.left_symbol;
					temp.right_symbol.assign(pgram.right_symbol.begin(), pgram.right_symbol.end());
					temp.dotPos = (temp.right_symbol[0][0] == grammer.NULL_SIGN) ? 1 : 0;
					for (auto pfirst : colFirstSet) {
						temp.seek_symbol = pfirst;
						//判断是否已经在outvec中
						bool is_push = 1;
						int vecsize = outvec.size();
						for (int i = 0; i < vecsize; i++) {
							if (outvec[i] == temp) {
								is_push = 0;
								break;
							}
						}
						if (is_push) {
							outvec.push_back(temp);
						//cout << temp.left_symbol << "->";
						//int cnt = 0;
						//for (auto t : temp.right_symbol) {
						//	if (cnt++ == temp.dotPos)
						//		cout << "·";
						//	cout << t << " ";
						//}
						//if (temp.dotPos == temp.right_symbol.size())
						//	cout << "·";
						//cout << "---" << temp.seek_symbol << endl;
						//
						}

					}
				}
			}
		}
	}
	for (auto i : outvec)
		outItems.insert(i);
	// cout << "***闭包集合：" << endl;
	// printItems(outItems); //////////////////////////////////////////////////////////////////////////////////////暂时注释
}

// 求一个文法符号串的FIRST集
// 求闭包时使用 因为不可能有@ 所以没考虑
set<string>  LR1_grammer::getStrFirstSet(vector<string> str)
{
	int num = str.size();
	set<string> strFirst;
	if (str.empty())
		return strFirst;
	//bool is_epsilon = true;

	//依次遍历每一个符号
	for (auto itstr : str) {
		//终结符
		if (grammer.Terminals.find(itstr) != grammer.Terminals.end()) {
			strFirst.insert(itstr);
			break;
		}
		//非终结符
		auto it = grammer.NTterminals.find(itstr);
		if (it != grammer.NTterminals.end()) {
			bool is_continue = 0;
			for (auto t : FIRST[*it]) {
				if (t[0] != grammer.NULL_SIGN)
					strFirst.insert(t);
				else
					is_continue = 1;
			}
			if (is_continue)
				continue;
			else
				break;
		}
		else {
			cout << "ERROR!";
			break;
		}
	}
	return strFirst;
}


// 打印first集
void LR1_grammer::printFirst() {
	for (auto str : grammer.Terminals) {
		cout << "FIRST(" << str << ") = { ";
		for (auto str1 : this->FIRST[str]) {
			cout << str1 << " ";
		}
		cout << "}" << endl;
	}
	for (auto str : grammer.NTterminals) {
		cout << "FIRST(" << str << ") = { ";
		for (auto str1 : this->FIRST[str]) {
			cout << str1 << " ";
		}
		cout << "}" << endl;
	}
}

void LR1_grammer::fillGuiYueTable()
{
	int setsize = closureFamily.size();
	for (int i = 0; i < setsize; i++) {
		for (auto it = closureFamily[i].begin(); it != closureFamily[i].end(); it++) {
			//归约项目
			if (it->right_symbol.size() == it->dotPos) {

				int gramNum = -1;
				int proSize = grammer.productItems.size();
				for (int t = 0; t < proSize; t++) {
					auto gram = grammer.productItems[t];
					if (gram.left_symbol == it->left_symbol && gram.right_symbol.size() == it->right_symbol.size()) {
						int nsize = gram.right_symbol.size();
						int n = 0;
						for (; n < nsize; n++) {
							if (gram.right_symbol[n] != it->right_symbol[n])
								break;
						}
						if (n == nsize) {
							gramNum = t;
							break;
						}
					}
				}
				if (gramNum == -1) {
					cout << "归约出错!" << endl;
					exit(-1);
				}
				pair<int, string>in = pair<int, string>(i, it->seek_symbol);
				//ACC
				if (LR1_table.count(in) == 0) {//没有冲突
					if (gramNum == grammer.startItemPos) {
						LR1_table[in] = pair<string, int>("acc", gramNum);
					}
					else {
						LR1_table[in] = pair<string, int>("r", gramNum);
					}
				}
				else {
					cout << "ERROR:入口冲突!";
				}
			}
		}
	}
}

void LR1_grammer::printLR1Table(string filename)
{
	ofstream outfile;
	outfile.open(filename);
	if (!outfile) {
		cout << "文件打开失败!" << endl;
		exit(-1);
	}

	pair<int, string> in;
	int stateNum = closureFamily.size();
	int vtNum = grammer.Terminals.size();
	int nvtNum = grammer.NTterminals.size();
	int colNum = vtNum + nvtNum;

	outfile << setiosflags(ios::left) << setw(7) << " ";
	for (auto str : grammer.Terminals) {
		outfile << setiosflags(ios::left) << setw(10) << str;
	}
	for (auto str : grammer.NTterminals) {
		outfile << setiosflags(ios::left) << setw(10) << str;
	}
	outfile << endl;

	//打表
	for (int row = 0; row < stateNum; row++) {
		outfile << setiosflags(ios::left) << setw(5) << row << ": ";
		auto pt = grammer.Terminals.begin();
		auto pnt = grammer.NTterminals.begin();

		for (int col = 0; col < colNum; col++) {
			string symbol;
			if (col < vtNum) {
				symbol = *pt;
				pt++;
			}
			else {
				symbol = *pnt;
				pnt++;
			}
			in = pair<int, string>(row, symbol);
			if (LR1_table.count(in) == 0) {//REJ
				outfile << setiosflags(ios::left) << setw(10) << " ";
			}
			else {
				if (LR1_table[in].first != "null") {
					string temp;
					if (LR1_table[in].first != "acc")
						temp = LR1_table[in].first + to_string(LR1_table[in].second);
					else
						temp = LR1_table[in].first;

					outfile << setiosflags(ios::left) << setw(10) << temp;
				}
				else
					outfile << setiosflags(ios::left) << setw(10) << LR1_table[in].second;
			}
		}
		outfile << endl;
	}
}
