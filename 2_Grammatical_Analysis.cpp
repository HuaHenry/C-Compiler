#include "2_Grammatical_Analysis.h"
#include "2_Grammatical_LR1Grammer.h"
#include <stack>

// extern void deleteThreadedList(ThreadedList* Root);
//extern string START_SIGN;

// 传入引用
ThreadedList* grammaticalAnalysis(vector<ANSWER_TYPE>& Answer, map<TABLE_KEY_TYPE, TABLE_VALUE_TYPE>& Table, vector<ITEM_TYPE>& Items, SemanticAnalysis& Semantic)
{
	//定义语义分析
	Semantic = SemanticAnalysis();
	Semantic.AddSymbol2List({ "Program","",-1,-1,-1,-1 });

	TOKEN temp = { "#", "--", -1, -1 };
	Answer.push_back(temp);

	vector<int>statusStack;					// 状态栈
	vector<string>symbolsStack;				// 符号栈
	vector<ThreadedList*>threadNodeStack;   // 穿线表栈，用于绘图
	int answerPointer = 0;					// 字符串指针
	int regulationStepCount = 0;			// 当前进行到第几次规约

	TABLE_VALUE_TYPE tableValue;			// 查Table获取的数据存到这里
	string action;							// 下一步骤，即tableValue.first

	statusStack.push_back(0);					// 初始状态为状态0
	string symbol;
	while (true)
	{
		symbol = Answer[answerPointer].token_key;
		pair<int, string>cur(statusStack.back(), symbol); // 构造map索引
		tableValue = Table[cur];			  // 查action表，获取下一步动作

		string action = tableValue.first;
		if (action == "acc")		// 规约成功
		{
			if (threadNodeStack.size() == 1)
				return threadNodeStack[0];
			else
			{
				deleteThreadedList(threadNodeStack[0]);
				return nullptr;
			}
		}
		else if (action == "")	// 规约失败
		{
			deleteThreadedList(threadNodeStack[0]);
			return nullptr;
		}
		else if (action == "s")		// 移进
		{
			string curSymbol = Answer[answerPointer].token_key;
			symbolsStack.push_back(curSymbol);			       // 将输入串当前字符压入符号栈，并移动指针
			statusStack.push_back(tableValue.second);

			ThreadedList* Node = new(nothrow)ThreadedList;     // 构造新的穿线表结点
			if (!Node)
			{
				deleteThreadedList(threadNodeStack[0]);
				exit(OVERFLOW);
			}
			if (curSymbol == "identifier")		// 由于其是终结符，因此不用在规约时再考虑之
				Node->IdIfExists = Answer[answerPointer].value; // 用于绘制语法树
			Node->symbol = curSymbol;

			threadNodeStack.push_back(Node);

			//将token加入语义分析的符号串中
			Semantic.AddSymbol2List({ Answer[answerPointer].token_key,Answer[answerPointer].value,Answer[answerPointer].row,Answer[answerPointer].col,-1,-1 });

			answerPointer++;
		}
		else if (action == "r")		// 规约
		{
			++regulationStepCount;			// 规约次数+1

			int regulationTerm = tableValue.second;								// 选定规约项
			string regulationLeft = Items[regulationTerm].left_symbol;			// 规约左部，唯一

			int regulationRightLength = Items[regulationTerm].right_symbol.size();	// 右部长度

			if (statusStack.size() < regulationRightLength)
			{
				cout << "ERROR: NOT ENOUGH STATUS!" << endl;
				deleteThreadedList(threadNodeStack[0]);
				exit(EXIT_FAILURE);
			}

			if (Items[regulationTerm].right_symbol[0] == "@")							// 空串特判
				regulationRightLength = 0;

			ThreadedList* Node = new(nothrow)ThreadedList(regulationStepCount);     // 构造新的穿线表结点，此结点为当前规约使用的产生式左部
			if (!Node)
			{
				deleteThreadedList(threadNodeStack[0]);
				exit(OVERFLOW);
			}
			Node->symbol = regulationLeft;

			for (int i = 0; i < regulationRightLength; ++i)  // 弹出待规约的右部并连接穿线表
			{
				symbolsStack.pop_back();
				statusStack.pop_back();

				Node->threads.push_back(threadNodeStack.back());
				threadNodeStack.pop_back();
			}

			symbolsStack.push_back(regulationLeft);				// 压入左部
			threadNodeStack.push_back(Node);					// 将连接好的非终结符结点压回穿线表结点栈

			cur = pair<int, string>(statusStack.back(), symbolsStack.back()); // 构造map索引
			tableValue = Table[cur];			     // 查表(goto)，获取待压入的状态
			statusStack.push_back(tableValue.second);

			//语义分析
			//cout <<"Semantic.Analysis:  "<< Items[regulationTerm].left_symbol << endl;
			Semantic.Analysis(Items[regulationTerm].left_symbol, Items[regulationTerm].right_symbol);

		}
		else // 不明字符
		{
			cout << "ERROR: UNKNOWN OPERATION OF LR1TABLE!" << endl;
			deleteThreadedList(threadNodeStack[0]);
			exit(EXIT_FAILURE);
		}
	}
}

// 删除穿线表
void deleteThreadedList(ThreadedList* Root)
{
	// 非递归遍历，使用栈即可
	stack<ThreadedList*>threadedListStack;
	threadedListStack.push(Root);

	while (!threadedListStack.empty())
	{
		ThreadedList* Node = threadedListStack.top();
		threadedListStack.pop();
		for (int i = 0; i < Node->threads.size(); ++i)
			threadedListStack.push(Node->threads[i]);
		delete Node;
	}
}

