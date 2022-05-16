#include"functions.h"

void Stringsplit(string str, const const char split, vector<string>& res)
{
	istringstream iss(str);	// ������
	string token;			// ���ջ�����
	while (getline(iss, token, split))	// ��splitΪ�ָ���
	{
		res.push_back(token);
	}
}

int main() {
	while (true)
	{
		cout << '>';
		string str;
		getline(cin, str);
		vector<string> strList;
		Stringsplit(str, ' ', strList);	// ���Ӵ���ŵ�strList��
		string menu = strList[0];
		if (menu.compare(INSERT) == 0) {
			insertValue();
		}
		else if (menu.compare(SELECT) == 0) {
			selectValue();
		}
		else if (menu.compare(DELETE) == 0) {
			deleteValue();
		}
		else if (menu.compare(SHUTDOWN) == 0) {
			return 0;
		}
		else {
			cout << "���������������������" << endl;
		}
	}
}