#include"functions.h"

FILE* fp; // ȫ�ֱ���fp

void Stringsplit(string str, const char split, vector<string>& res) // �ָ��ַ���
{
	istringstream iss(str);	// ������
	string token;			// ���ջ�����
	while (getline(iss, token, split))	// ��splitΪ�ָ���
	{
		res.push_back(token);
	}
}

/* ���������ļ� */
void createDataFile() {
	fp = OPEN_FILE(FILE_NAME, "rb");
	if (fp == NULL) {

		uint page_pos;

		MALLOC_PAGE(fileHead, FileHead);
		MALLOC_PAGE(btPage, BTPage);
		MALLOC_PAGE(dataPage, DataPage);


		fp = OPEN_FILE(FILE_NAME, "wb+");

		/* ��ʼ��fileHead */
		fileHead->order = DATA_PAGE_KEY_MAX;
		fileHead->page_size = PAGE_MAX_LENGTH;
		fileHead->page_count = 3;
		fileHead->root_page_pos = 0;

		FSEEK_END_WRITE(fp, page_pos, fileHead, sizeof(FileHead), 0);

		/** ��ʼ��fileHeadָ��ĵ�һ��btPage **/
		btPage->page_type = 1;
		btPage->key_count = 1;
		btPage->key[0] = 0;
		FSEEK_END_WRITE(fp, page_pos, btPage, sizeof(BTPage), 1);
		fileHead->root_page_pos = page_pos;

		/** ��ʼ��btPageָ��ĵ�һ��dataPage **/

		dataPage->page_type = 2;
		dataPage->key_count = 0;
		dataPage->is_inc_insert = true;
		dataPage->is_dec_insert = true;
		dataPage->parent_page_pos = page_pos;
		FSEEK_END_WRITE(fp, page_pos, dataPage, sizeof(DataPage), 2);
		btPage->child_page_pos[0] = page_pos;

		FSEEK_FIXED_WRITE(fp, 0, fileHead, sizeof(FileHead));
		FSEEK_FIXED_WRITE(fp, fileHead->root_page_pos, btPage, sizeof(BTPage));

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

int main() {
	createDataFile(); // û�������ļ�ʱ����Ҫ���������ļ�
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

				if (insertData(data, key)) {
					end = clock();
					cout << "executed insert index:" << key << "��time��" << float(end - start) / CLOCKS_PER_SEC << " seconds" << endl;
				}
			}
			else if (strList.size() == 2) { // ���ļ�����ķ�ʽ�����������
				clock_t start, end;
				start = clock();

				cout << "load data begin. . ." << endl;
				int cnt = loadCsvData(strList[1]);

				end = clock();
				cout << "load data end��" << cnt << " rows are inserted��time��" << float(end - start) / CLOCKS_PER_SEC << " seconds" << endl;
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
					cout << "executed search��time��" << float(end - start) / CLOCKS_PER_SEC << " seconds" << endl;
				}
				else {
					end = clock();
					cout << "index:" << stoi(strList[1]) << " doesn't exist��time��" << float(end - start) / CLOCKS_PER_SEC << " seconds" << endl;
				}
			}
			else if (strList.size() == 8) { // ����������Χɨ���ѯ����
				vector<bool> field = fieldChoice(strList[1]);

				clock_t start, end;
				start = clock();

				selectRangeData(stoi(strList[5]), stoi(strList[7]), field);

				end = clock();
				cout << "executed search��time��" << float(end - start) / CLOCKS_PER_SEC << " seconds" << endl;
			}

			else if (strList.size() == 9) { // ����������Χɨ��Ĳ�ѯ��������
				cout << "export data start..." << endl;
				clock_t start, end;
				start = clock();
				outputCsvData(stoi(strList[6]), stoi(strList[8]), strList[2]);
				end = clock();
				cout << "export data end��time��" << float(end - start) / CLOCKS_PER_SEC << " seconds" << endl;  //���ʱ�䣨��λ����
			}
			else {
				cout << ERROR << endl;
			}

		}

		/* ɾ������TODO */
		else if (menu.compare(DELETE) == 0) {
			deleteData(); // TODO
		}

		else if (menu.compare(RESET) == 0) {
			remove(FILE_NAME);
			createDataFile();
		}

		/* �رճ��� */
		else if (menu.compare(SHUTDOWN) == 0) {
			return 0;
		}

		else {
			cout << ERROR << endl;
		}
	}
}