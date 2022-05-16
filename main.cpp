#include"functions.h"

void Stringsplit(string str, const const char split, vector<string>& res)
{
	istringstream iss(str);	// 输入流
	string token;			// 接收缓冲区
	while (getline(iss, token, split))	// 以split为分隔符
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
		Stringsplit(str, ' ', strList);	// 将子串存放到strList中
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
			cout << "输入命令错误，请重新输入" << endl;
		}
	}
}