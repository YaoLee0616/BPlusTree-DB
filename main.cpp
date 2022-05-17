#include"functions.h"

FILE* fp;
void Stringsplit(string str, const const char split, vector<string>& res)
{
	istringstream iss(str);	// 输入流
	string token;			// 接收缓冲区
	while (getline(iss, token, split))	// 以split为分隔符
	{
		res.push_back(token);
	}
}

/** 创建数据文件 **/
void createDataFile() {
	fp = OPEN_FILE(FILE_NAME, "rb");
	if (fp== NULL) {

		uint node_pos;
		uint pageCount = 0;

		FileHead fileHead;
		BTNode btNode;
		DataNode dataNode;

		fp = OPEN_FILE(FILE_NAME, "wb+");

		/** 初始化FileHead **/
		fileHead = { PAGE_MAX_LENGTH,3,0,1 };
		FSEEK_END_WRITE(fp, pageCount, fileHead.pageSize,node_pos, (char*)&fileHead, sizeof(fileHead));

		/** 初始化fileHead指向的第一个btNode **/
		btNode = { 1,0 };
		btNode.key[0] = UINT_MAX_VALUE;
		FSEEK_END_WRITE(fp, pageCount, fileHead.pageSize,node_pos, (char*)&btNode, sizeof(btNode));
		fileHead.rootPos = node_pos;

		/** 初始化btNode指向的第一个dataNode **/
		dataNode = { 2,0 };
		FSEEK_END_WRITE(fp, pageCount, fileHead.pageSize, node_pos, (char*)&dataNode, sizeof(dataNode));
		btNode.ptr[0] = node_pos;
		fileHead.pageCount = pageCount;
		FSEEK_HED_WRITE(fp, node_pos, (char*)&fileHead, sizeof(fileHead));
		FSEEK_FIXED_WRITE(fp, fileHead.rootPos, (char*)&btNode, sizeof(btNode));

		CLOSE_FILE(fp);
	}

	else {
		CLOSE_FILE(fp);
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
			createDataFile();
			Data data;
			uint key = stoi(strList[1]);
			strcpy(data.c1, strList[2].c_str());
			strcpy(data.c2, strList[3].c_str());
			strcpy(data.c3, strList[4].c_str());
			strcpy(data.c4, strList[5].c_str());
			insertData(data,key);
		}
		else if (menu.compare(SELECT) == 0) {
			uint key = stoi(strList[1]);
			selectData(key);
		}
		else if (menu.compare(DELETE) == 0) {
			deleteData();
		}
		else if (menu.compare(SHUTDOWN) == 0) {
			return 0;
		}
		else {
			cout << "输入命令错误，请重新输入" << endl;
		}
	}
}