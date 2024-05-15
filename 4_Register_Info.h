#include <string>

using namespace std;

//记录Reg的名称、编号、未使用时间
struct Reg
{
	int index = 0;		// 编号
	string name;		// 名称
	int unusedTime = 0;	// 未使用时间

	bool operator < (const Reg& b) {
		return unusedTime < b.unusedTime;
	};
};