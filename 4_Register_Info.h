#include <string>

using namespace std;

//��¼Reg�����ơ���š�δʹ��ʱ��
struct Reg
{
	int index = 0;		// ���
	string name;		// ����
	int unusedTime = 0;	// δʹ��ʱ��

	bool operator < (const Reg& b) {
		return unusedTime < b.unusedTime;
	};
};