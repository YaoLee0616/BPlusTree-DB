#include"functions.h"

/* 全局变量 */
FILE* fp; // 全局变量fp

// file_head
FileHead buffer_pool_file_head;

// all_list
BTPageListItem bt_page_all_list_head;
DataPageListItem data_page_all_list_head;
int bt_page_all_list_count;
int data_page_all_list_count;

// flush_list
BTPageListItem bt_page_flush_list_head;
DataPageListItem data_page_flush_list_head;
int bt_page_flush_list_count;
int data_page_flush_list_count;

// hash_map
map<int, BTPage*> btPageMap;
map<int, DataPage*> dataPageMap;

// flush_list_hash_map
map<int, BTPage*> flushListBTPageMap;
map<int, DataPage*> flushListDataPageMap;


void Stringsplit(string str, const char split, vector<string>& res) // 分割字符串
{
	istringstream iss(str);	// 输入流
	string token;			// 接收缓冲区
	while (getline(iss, token, split))	// 以split为分隔符
	{
		res.push_back(token);
	}
}
void initialBufferPool() {

	free();

	// all_list
	bt_page_all_list_count = 0;
	data_page_all_list_count = 0;
	bt_page_all_list_head.next = NULL;
	data_page_all_list_head.next = NULL;

	// flush_list
	bt_page_flush_list_count = 0;
	data_page_flush_list_count = 0;
	bt_page_flush_list_head.next = NULL;
	data_page_flush_list_head.next = NULL;

	// hash_map
	btPageMap.erase(btPageMap.begin(), btPageMap.end());
	dataPageMap.erase(dataPageMap.begin(), dataPageMap.end());
	flushListBTPageMap.erase(flushListBTPageMap.begin(), flushListBTPageMap.end());
	flushListDataPageMap.erase(flushListDataPageMap.begin(), flushListDataPageMap.end());

	// buffer_pool_file_head
	fp = OPEN_FILE(FILE_NAME, "rb");
	MALLOC_PAGE(fileHead, FileHead);
	FSEEK_FIXED_READ(fp, 0, fileHead, sizeof(FileHead));
	CLOSE_FILE(fp);
	buffer_pool_file_head = *fileHead;

}

/* 创建数据文件 */
void createDataFile() {
	fp = OPEN_FILE(FILE_NAME, "rb");
	if (fp == NULL) {
		fp = OPEN_FILE(FILE_NAME, "wb+");

		MALLOC_PAGE(fileHead, FileHead);
		MALLOC_PAGE(btPage, BTPage);
		MALLOC_PAGE(dataPage, DataPage);

		/* 初始化fileHead */
		fileHead->order = DATA_PAGE_KEY_MAX;
		fileHead->page_size = PAGE_MAX_LENGTH;
		fileHead->page_count = 3;
		fileHead->root_page_id = 1;

		FSEEK_END_WRITE(fp, fileHead, sizeof(FileHead), 0);

		/** 初始化fileHead指向的第一个btPage **/
		btPage->page_type = BT_PAGE_TYPE;
		btPage->key_count = 1;

		btPage->page_id = 1;

		btPage->key[0] = 0;
		btPage->child_page_id[0] = 2;
		FSEEK_END_WRITE(fp, btPage, sizeof(BTPage), 1);

		/** 初始化btPage指向的第一个dataPage **/

		dataPage->page_type = DATA_PAGE_TYPE;
		dataPage->key_count = 0;
		dataPage->is_inc_insert = true;
		dataPage->is_dec_insert = true;

		dataPage->page_id = 2;
		dataPage->parent_page_id = 1;
		FSEEK_END_WRITE(fp, dataPage, sizeof(DataPage), 2);

		CLOSE_FILE(fp);
		FREE_PAGE(fileHead);
		FREE_PAGE(btPage);
		FREE_PAGE(dataPage);
	}
	else {
		CLOSE_FILE(fp);
	}
}

/* 判断需要哪些字段 */
vector<bool> fieldChoice(string str) {
	string set[5] = { "a","c1","c2","c3","c4" };
	vector<bool> field(5);
	if (str == "*") {
		for (int i = 0; i < 5; i++) field[i] = true;
	}
	vector<string> fieldList;
	Stringsplit(str, ',', fieldList);	// 将子串存放到fieldList中
	for (int i = 0; i < fieldList.size(); i++) {
		for (int j = 0; j < 5; j++) {
			if (fieldList[i] == set[j])
				field[j] = true;
		}
	}
	return field;
}

/* 帮助文档 */
void help() {
	cout << "******************************************************************************************************************" << endl;
	cout << "  .help                                                                                print help message;" << endl;
	cout << "  .reset                                                                               delete db file;" << endl;
	cout << "  shutdown                                                                             exit program;" << endl;
	cout << "  insert {a} {c1} {c2} {c3} {c4}                                                       insert record;" << endl;
	cout << "  insert {infile}                                                                      load records from file;" << endl;
	cout << "  select * where a={index}                                                             search a record by index;" << endl;
	cout << "  select * where a between {minIndex} and {maxIndex}                                   search records between indexs;" << endl;
	cout << "  select into {outfile} where a between {minIndex} and {maxIndex}                      export records between indexs;" << endl;
	cout << "******************************************************************************************************************" << endl;
}

/* 欢迎界面 */
void welcome()
{
	cout << "******************************************************************************************************************" << endl;
	cout << "                               Welcome to the bplus_tree_db                           " << endl;
	cout << "                               db file locates in \"./data.ibd\"                 " << endl;
	help();
}

int main() {
	welcome();
	createDataFile(); // 没有数据文件时, 需要创建数据文件
	initialBufferPool();
	while (true)
	{
		cout << '>';
		string str;
		getline(cin, str);
		vector<string> strList;
		Stringsplit(str, ' ', strList);
		string menu = strList[0];

		/* 插入数据 */
		if (menu.compare(INSERT) == 0) {

			if (strList.size() == 6) { // 插入单条数据
				Data data;

				uint key = stoi(strList[1]);
				strcpy(data.c1, strList[2].c_str());
				strcpy(data.c2, strList[3].c_str());
				strcpy(data.c3, strList[4].c_str());
				strcpy(data.c4, strList[5].c_str());

				clock_t start, end;
				start = clock();

				if (insertData(&data, key)) {
					flush();
					end = clock();
					cout << "executed insert index:" << key << ", time:" << float(end - start) / CLOCKS_PER_SEC << " seconds" << endl;
				}
			}
			else if (strList.size() == 2) { // 以文件导入的方式插入大量数据
				clock_t start, end;
				start = clock();

				cout << "load data begin. . ." << endl;
				int cnt = loadCsvData(strList[1]);
				flush();
				end = clock();
				cout << "load data end, " << cnt << " rows are inserted, time:" << float(end - start) / CLOCKS_PER_SEC << " seconds" << endl;
			}
			else {
				cout << ERROR << endl;
			}
		}

		/* 查询数据 */
		else if (menu.compare(SELECT) == 0) {
			if (strList.size() == 4) { // 主键等值查询数据
				vector<bool> field = fieldChoice(strList[1]);
				string str = strList[3];
				vector<string> strList;
				Stringsplit(str, '=', strList);

				clock_t start, end;
				start = clock();

				if (selectData(stoi(strList[1]), field)) {
					end = clock();
					cout << "executed search, time:" << float(end - start) / CLOCKS_PER_SEC << " seconds" << endl;
				}
				else {
					end = clock();
					cout << "index:" << stoi(strList[1]) << " doesn't exist, time:" << float(end - start) / CLOCKS_PER_SEC << " seconds" << endl;
				}
			}
			else if (strList.size() == 8) { // 主键索引范围扫描查询数据
				vector<bool> field = fieldChoice(strList[1]);

				clock_t start, end;
				start = clock();

				selectRangeData(stoi(strList[5]), stoi(strList[7]), field);

				end = clock();
				cout << "executed search, time:" << float(end - start) / CLOCKS_PER_SEC << " seconds" << endl;
			}

			else if (strList.size() == 9) { // 主键索引范围扫描的查询导出数据
				cout << "export data start..." << endl;
				clock_t start, end;
				start = clock();
				outputCsvData(stoi(strList[6]), stoi(strList[8]), strList[2]);
				end = clock();
				cout << "export data end, time:" << float(end - start) / CLOCKS_PER_SEC << " seconds" << endl;  //输出时间（单位:ｓ）
			}
			else {
				cout << ERROR << endl;
			}

		}

		/* 删除数据TODO */
		else if (menu.compare(DELETE) == 0) {
			deleteData(); // TODO
		}

		/* 重置 */
		else if (menu.compare(RESET) == 0) {
			remove(FILE_NAME);
			createDataFile();
			initialBufferPool();
			cout << RESET_SUCCESSFUL << endl;
		}

		/* 帮助文档 */
		else if (menu.compare(HELP) == 0) {
			help();
		}

		/* 关闭程序 */
		else if (menu.compare(SHUTDOWN) == 0) {
			free();
			return 0;
		}

		else {
			cout << ERROR << endl;
		}

	}
}