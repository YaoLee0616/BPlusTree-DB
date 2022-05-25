#include "functions.h"

/* 获取文件中页的数量 */
uint getPageCount() {
	MALLOC_PAGE(fileHeadBuf, FileHead);
	FSEEK_FIXED_READ(fp, 0, fileHeadBuf, sizeof(FileHead));
	uint page_count = fileHeadBuf->page_count;
	FREE_PAGE(fileHeadBuf);
	return page_count;
}

/* 更新文件头信息 */
void updateFileHead(uint new_root_page_pos) {
	MALLOC_PAGE(fileHeadBuf, FileHead);
	FSEEK_FIXED_READ(fp, 0, fileHeadBuf, sizeof(FileHead));
	if (new_root_page_pos > 0) {
		fileHeadBuf->root_page_pos = new_root_page_pos;
	}
	fileHeadBuf->page_count++;
	FSEEK_FIXED_WRITE(fp, 0, fileHeadBuf, sizeof(FileHead));
	FREE_PAGE(fileHeadBuf);
}

/* 插入索引页 */
BTPage insertBTPage(uint page_pos, int index, uint key, uint ap) {

	BTPage buf;
	BTPage* btPageBuf = &buf;

	FSEEK_FIXED_READ(fp, page_pos, btPageBuf, sizeof(BTPage));

	int j;
	for (j = btPageBuf->key_count - 1; j >= index; j--) {
		btPageBuf->key[j + 1] = btPageBuf->key[j];
		btPageBuf->child_page_pos[j + 1] = btPageBuf->child_page_pos[j];
	}
	btPageBuf->key[index] = key;
	btPageBuf->child_page_pos[index] = ap;

	btPageBuf->key_count++;
	FSEEK_FIXED_WRITE(fp, page_pos, btPageBuf, sizeof(BTPage));
	return (*btPageBuf);
}

/* 插入数据页 */
DataPage insertDataPage(uint page_pos, int index, uint key, Data data) {
	DataPage buf;
	DataPage* dataPageBuf = &buf;

	FSEEK_FIXED_READ(fp, page_pos, dataPageBuf, sizeof(DataPage));
	if (index != 0) dataPageBuf->is_dec_insert = false;
	if (index != dataPageBuf->key_count) dataPageBuf->is_inc_insert = false;

	int j;
	if (dataPageBuf->key_count != 0) {
		for (j = dataPageBuf->key_count - 1; j >= index; j--) {
			dataPageBuf->key[j + 1] = dataPageBuf->key[j];
			dataPageBuf->data[j + 1] = dataPageBuf->data[j];
		}
	}

	dataPageBuf->key[index] = key;
	dataPageBuf->data[index] = data;

	dataPageBuf->key_count++;
	FSEEK_FIXED_WRITE(fp, page_pos, dataPageBuf, sizeof(DataPage));
	return (*dataPageBuf);
}

/* 分裂数据页 */
uint splitDataPage(DataPage* dataPageBuf, uint page_pos, int s) {

	int i, j;
	int n = dataPageBuf->key_count;
	uint ap;

	MALLOC_PAGE(apDataPageBuf, DataPage);

	for (i = s + 1, j = 0; i < n; i++, j++) {
		apDataPageBuf->key[j] = dataPageBuf->key[i];
		apDataPageBuf->data[j] = dataPageBuf->data[i];
	}
	apDataPageBuf->page_type = 2;
	apDataPageBuf->key_count = n - s - 1;
	apDataPageBuf->is_inc_insert = true;
	apDataPageBuf->is_dec_insert = true;
	apDataPageBuf->parent_page_pos = dataPageBuf->parent_page_pos;
	apDataPageBuf->prev_page_pos = page_pos;
	apDataPageBuf->next_page_pos = dataPageBuf->next_page_pos;

	uint page_count = getPageCount();
	FSEEK_END_WRITE(fp, ap, apDataPageBuf, sizeof(DataPage), page_count);

	updateFileHead(0);

	dataPageBuf->key_count = s + 1;
	dataPageBuf->next_page_pos = ap;
	FSEEK_FIXED_WRITE(fp, page_pos, dataPageBuf, sizeof(DataPage));
	FREE_PAGE(apDataPageBuf);
	return ap;
}

/* 分裂索引页 */
uint splitBTPage(BTPage* btPageBuf, uint page_pos, int s) {

	int i, j;
	int n = btPageBuf->key_count;
	uint ap;

	MALLOC_PAGE(apBTPageBuf, BTPage);
	MALLOC_PAGE(apBTPageBufChild, BTPage);
	MALLOC_PAGE(apDataPageBufChild, DataPage);

	for (i = s, j = 0; i < n; i++, j++) {
		apBTPageBuf->key[j] = btPageBuf->key[i];
		apBTPageBuf->child_page_pos[j] = btPageBuf->child_page_pos[i];
	}
	apBTPageBuf->page_type = 1;
	apBTPageBuf->key_count = n - s;
	apBTPageBuf->parent_page_pos = btPageBuf->parent_page_pos;

	uint page_count = getPageCount();
	FSEEK_END_WRITE(fp, ap, apBTPageBuf, sizeof(BTPage), page_count);

	updateFileHead(0);

	for (int i = 0; i < n - s; i++) {
		if (apBTPageBuf->child_page_pos[i] != 0) {
			FSEEK_FIXED_READ(fp, apBTPageBuf->child_page_pos[i], apBTPageBufChild, sizeof(BTPage));
			if (apBTPageBufChild->page_type == 2) {
				FSEEK_FIXED_READ(fp, apBTPageBuf->child_page_pos[i], apDataPageBufChild, sizeof(DataPage));
				apDataPageBufChild->parent_page_pos = ap;
				FSEEK_FIXED_WRITE(fp, apBTPageBuf->child_page_pos[i], apDataPageBufChild, sizeof(DataPage));
			}
			else {
				apBTPageBufChild->parent_page_pos = ap;
				FSEEK_FIXED_WRITE(fp, apBTPageBuf->child_page_pos[i], apBTPageBufChild, sizeof(BTPage));
			}
		}
	}

	btPageBuf->key_count = s;

	FSEEK_FIXED_WRITE(fp, page_pos, btPageBuf, sizeof(BTPage));
	FREE_PAGE(apBTPageBuf);
	FREE_PAGE(apBTPageBufChild);
	FREE_PAGE(apDataPageBufChild);
	return (ap);
}

/* 新建根页面 */
void newRoot(uint root_page_pos, uint key, uint ap) {
	uint pos;
	MALLOC_PAGE(rootBTPageBuf, BTPage);
	rootBTPageBuf->page_type = 1;
	rootBTPageBuf->key_count = 2;
	rootBTPageBuf->child_page_pos[0] = root_page_pos;
	rootBTPageBuf->child_page_pos[1] = ap;
	rootBTPageBuf->key[0] = 0;
	rootBTPageBuf->key[1] = key;

	uint page_count = getPageCount();
	FSEEK_END_WRITE(fp, pos, rootBTPageBuf, sizeof(BTPage), page_count);

	/* 读取原根页面更新parent_page_pos位置 */
	FSEEK_FIXED_READ(fp, root_page_pos, rootBTPageBuf, sizeof(BTPage));
	rootBTPageBuf->parent_page_pos = pos;
	FSEEK_FIXED_WRITE(fp, root_page_pos, rootBTPageBuf, sizeof(BTPage));

	/* 读取分裂页面更新parent_page_pos位置 */
	FSEEK_FIXED_READ(fp, ap, rootBTPageBuf, sizeof(BTPage));
	rootBTPageBuf->parent_page_pos = pos;
	FSEEK_FIXED_WRITE(fp, ap, rootBTPageBuf, sizeof(BTPage));

	updateFileHead(pos);

	FREE_PAGE(rootBTPageBuf);
}

/* 将数据插入B+树 */
void insertBPlusTree(uint head_pos, uint key, uint page_pos, int i, Data data) {
	int s = 0;
	uint ap = 0;
	bool finished = false;
	bool needNewRoot = false;
	DataPage dataPageBuf;
	dataPageBuf = insertDataPage(page_pos, i, key, data);
	if (dataPageBuf.key_count <= DATA_PAGE_KEY_MAX) finished = true;

	if (!finished) {
		if (dataPageBuf.is_inc_insert) {
			s = DATA_PAGE_KEY_MAX - 1;
		}
		else if (dataPageBuf.is_dec_insert) {
			s = 0;
		}
		else {
			s = (DATA_PAGE_KEY_MAX + 1) / 2 - 1;
		}
		ap = splitDataPage(&dataPageBuf, page_pos, s);
		key = dataPageBuf.key[s];
		page_pos = dataPageBuf.parent_page_pos;
		BTPage btPageBuf;
		FSEEK_FIXED_READ(fp, page_pos, &btPageBuf, sizeof(BTPage));
		i = searchBTIndex(&btPageBuf, key);
	}

	BTPage btPageBuf;
	while (!needNewRoot && !finished) {
		btPageBuf = insertBTPage(page_pos, i, key, ap);
		if (btPageBuf.key_count <= BT_PAGE_KEY_MAX) {
			finished = true;
		}
		else {
			s = (BT_PAGE_KEY_MAX + 1) / 2 - 1;
			ap = splitBTPage(&btPageBuf, page_pos, s);
			key = btPageBuf.key[s];
			if (btPageBuf.parent_page_pos != 0) {
				page_pos = btPageBuf.parent_page_pos;
				FSEEK_FIXED_READ(fp, page_pos, &btPageBuf, sizeof(BTPage));
				i = searchBTIndex(&btPageBuf, key);
			}
			else {
				needNewRoot = true;
			}
		}
	}
	if (needNewRoot) {
		newRoot(head_pos, key, ap);
	}
}
/* 插入单条数据 */
bool insertData(Data data, uint key) {
	MALLOC_PAGE(fileHeadBuf, FileHead);

	OPEN_FILE_READ(FILE_NAME, "rb+", fileHeadBuf, sizeof(FileHead));
	Result result;
	searchDataPage(fileHeadBuf->root_page_pos, key, &result);
	if (result.is_found) {
		cout << "index:" << key << " already exists" << endl;
		FREE_PAGE(fileHeadBuf);
		CLOSE_FILE(fp);
		return false;
	}
	else {
		insertBPlusTree(fileHeadBuf->root_page_pos, key, result.pos, result.index, data);
		FREE_PAGE(fileHeadBuf);
		CLOSE_FILE(fp);
		return true;
	}
}

/* 以文件导入的方式插入大量数据 */
int loadCsvData(string csv_file_name) {
	ifstream inFile(csv_file_name, ios::in);
	string lineStr;
	int cnt = 0;
	Data data;
	while (getline(inFile, lineStr))
	{
		stringstream ss(lineStr);
		string str;
		vector<string> strList;
		while (getline(ss, str, (char)0x09))
			strList.push_back(str);
		uint key = stoi(strList[0]);
		strcpy(data.c1, strList[1].c_str());
		strcpy(data.c2, strList[2].c_str());
		strcpy(data.c3, strList[3].c_str());
		strcpy(data.c4, strList[4].c_str());
		if (insertData(data, key))
			cnt++;
	}

	inFile.close();
	return cnt;
}