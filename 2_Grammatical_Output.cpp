#include "2_Grammatical_Analysis.h"
#include "2_Grammatical_LR1Grammer.h"
#include <fstream>
#include <queue>

#define QUEUE_TYPE pair<ThreadedList*, int>

void addNode(ofstream& outfile, int cnt, ThreadedList* Node);
void addEdge(ofstream& outfile, int start, int end);
void addIdentifier(ofstream& outfile, int cnt, QUEUE_TYPE identifier);

string dotPath = "../outfiles/syntaxTree.dot";
string pngPath = "../outfiles/syntaxTree.png";
// 将符合dot脚本语言规范的归结过程输出到.dot文件中
// 再调用接口函数转换成归结树的图片
void generateTreeDot(ThreadedList* Root) {
	ofstream outfile(dotPath, ios::out);
	outfile << "digraph all{" << endl;
	outfile << "    rankdir = LR" << endl;
	outfile << "    node[shape = ellipse, fontname = \"Times-Roman\"]" << endl;
	outfile << "    edge[arrow = 0.5]" << endl;

	// 使用label属性，避免重复结点
	// 方法：对整棵树进行层次遍历，为每个结点创建一个node
	// 使用nullptr作为换层标识
	int nodeCount = 0;

	queue <QUEUE_TYPE> threadedListQueue;
	threadedListQueue.push(QUEUE_TYPE(Root, 0));
	addNode(outfile, nodeCount, Root);				// 写入根结点

	while (!threadedListQueue.empty())
	{
		// 每次弹出队首，进行扩展连接，再将压入队列
		// 扩展时，先添加结点声明，再添加连线
		QUEUE_TYPE elem = threadedListQueue.front();
		threadedListQueue.pop();
		if (elem.first->IdIfExists != "")
		{
			++nodeCount;		// 添加新的结点前，需要count自增
			addIdentifier(outfile, nodeCount, elem);
		}

		for (int i = 0; i < elem.first->threads.size(); ++i)
		{
			++nodeCount;		// 添加新的结点前，需要count自增
			QUEUE_TYPE newNode(elem.first->threads[i], nodeCount);

			addNode(outfile, nodeCount, newNode.first);
			addEdge(outfile, elem.second, nodeCount);
			threadedListQueue.push(newNode);
		}
	}
	outfile << "}" << endl;
	outfile.close();

	string command;
	command += "dot -Tpng " + dotPath + " -o " + pngPath;
	system(command.c_str());
}

void addIdentifier(ofstream& outfile, int cnt, QUEUE_TYPE identifier)
{
	outfile << "node" << to_string(cnt) << '[' << "label = " << '\"';
	outfile << identifier.first->IdIfExists;
	outfile << '\"' << ']' << ';' << endl;

	outfile << "node" << identifier.second << "->" << "node" << cnt;
	outfile << "[style = \"dotted\", arrowtail = \"none\", arrowhead = \"none\"]" << ';' << endl;
}

void addNode(ofstream &outfile, int cnt, ThreadedList* Node)
{
	outfile << "node" << to_string(cnt) << '[' << "label = " << '\"';

	if (Node->regulationStep)
		outfile << '(' << Node->regulationStep << ')' << ' ';
	outfile << Node->symbol;

	outfile << '\"' << ']' << ';' << endl;
}

void addEdge(ofstream& outfile, int start, int end)
{
	outfile << "node" << start << "->" << "node" << end << ';' << endl;
}