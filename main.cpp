#include"functions.h"

/* ȫ�ֱ��� */
FILE* fp; // ȫ�ֱ���fp

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


void Stringsplit(string str, const char split, vector<string>& res) // �ָ��ַ���
{
	istringstream iss(str);	// ������
	string token;			// ���ջ�����
	while (getline(iss, token, split))	// ��splitΪ�ָ���
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

/* ���������ļ� */
void createDataFile() {
	fp = OPEN_FILE(FILE_NAME, "rb");
	if (fp == NULL) {
		fp = OPEN_FILE(FILE_NAME, "wb+");

		MALLOC_PAGE(fileHead, FileHead);
		MALLOC_PAGE(btPage, BTPage);
		MALLOC_PAGE(dataPage, DataPage);

		/* ��ʼ��fileHead */
		fileHead->order = DATA_PAGE_KEY_MAX;
		fileHead->page_size = PAGE_MAX_LENGTH;
		fileHead->page_count = 3;
		fileHead->root_page_id = 1;

		FSEEK_END_WRITE(fp, fileHead, sizeof(FileHead), 0);

		/** ��ʼ��fileHeadָ��ĵ�һ��btPage **/
		btPage->page_type = BT_PAGE_TYPE;
		btPage->key_count = 1;

		btPage->page_id = 1;

		btPage->key[0] = 0;
		btPage->child_page_id[0] = 2;
		FSEEK_END_WRITE(fp, btPage, sizeof(BTPage), 1);

		/** ��ʼ��btPageָ��ĵ�һ��dataPage **/

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

/* �ж���Ҫ��Щ�ֶ� */
vector<bool> fieldChoice(string str) {
	string set[5] = { "a","c1","c2","c3","c4" };
	vector<bool> field(5);
	if (str == "*") {
		for (int i = 0; i < 5; i++) field[i] = true;
	}
	vector<string> fieldList;
	Stringsplit(str, ',', fieldList);	// ���Ӵ���ŵ�fieldList��
	for (int i = 0; i < fieldList.size(); i++) {
		for (int j = 0; j < 5; j++) {
			if (fieldList[i] == set[j])
				field[j] = true;
		}
	}
	return field;
}

/* �����ĵ� */
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

/* ��ӭ���� */
void welcome()
{
	cout << "******************************************************************************************************************" << endl;
	cout << "                               Welcome to the bplus_tree_db                           " << endl;
	cout << "                               db file locates in \"./data.ibd\"                 " << endl;
	help();
}

int main() {
	welcome();
	createDataFile(); // û�������ļ�ʱ, ��Ҫ���������ļ�
	initialBufferPool();
	while (true)
	{
		cout << '>';
		string str;
		getline(cin, str);
		vector<string> strList;
		Stringsplit(str, ' ', strList);
		string menu = strList[0];

		/* �������� */
		if (menu.compare(INSERT) == 0) {

			if (strList.size() == 6) { // ���뵥������
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
			else if (strList.size() == 2) { // ���ļ�����ķ�ʽ�����������
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

		/* ��ѯ���� */
		else if (menu.compare(SELECT) == 0) {
			if (strList.size() == 4) { // ������ֵ��ѯ����
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
			else if (strList.size() == 8) { // ����������Χɨ���ѯ����
				vector<bool> field = fieldChoice(strList[1]);

				clock_t start, end;
				start = clock();

				selectRangeData(stoi(strList[5]), stoi(strList[7]), field);

				end = clock();
				cout << "executed search, time:" << float(end - start) / CLOCKS_PER_SEC << " seconds" << endl;
			}

			else if (strList.size() == 9) { // ����������Χɨ��Ĳ�ѯ��������
				cout << "export data start..." << endl;
				clock_t start, end;
				start = clock();
				outputCsvData(stoi(strList[6]), stoi(strList[8]), strList[2]);
				end = clock();
				cout << "export data end, time:" << float(end - start) / CLOCKS_PER_SEC << " seconds" << endl;  //���ʱ�䣨��λ:��
			}
			else {
				cout << ERROR << endl;
			}

		}

		/* ɾ������TODO */
		else if (menu.compare(DELETE) == 0) {
			deleteData(); // TODO
		}

		/* ���� */
		else if (menu.compare(RESET) == 0) {
			remove(FILE_NAME);
			createDataFile();
			initialBufferPool();
			cout << RESET_SUCCESSFUL << endl;
		}

		/* �����ĵ� */
		else if (menu.compare(HELP) == 0) {
			help();
		}

		/* �رճ��� */
		else if (menu.compare(SHUTDOWN) == 0) {
			free();
			return 0;
		}

		else {
			cout << ERROR << endl;
		}

	}
}