#include "functions.h"

/* 更新文件头信息 */
void updateFileHead(uint new_root_page_id) {
	if (new_root_page_id != 1) {
		buffer_pool_file_head.root_page_id = new_root_page_id;
	}
	buffer_pool_file_head.page_count++;
}

/* 插入索引页 */
BTPage insertBTPage(uint page_id, int index, uint key, uint ap) {

	BTPage buf;
	BTPage* btPageBuf = &buf;

	// BufferPool
	if (btPageMap.count(page_id))
		*btPageBuf = *btPageMap[page_id];

	// 磁盘读取
	else{
		FSEEK_FIXED_READ(fp, page_id, btPageBuf, sizeof(BTPage));
		btPageInsertBufferPool(btPageBuf);
	}

	int j;
	for (j = btPageBuf->key_count - 1; j >= index; j--) {
		btPageBuf->key[j + 1] = btPageBuf->key[j];
		btPageBuf->child_page_id[j + 1] = btPageBuf->child_page_id[j];
	}
	btPageBuf->key[index] = key;
	btPageBuf->child_page_id[index] = ap;

	btPageBuf->key_count++;

	// 写入bufferpool
	btPageInsertBufferPool(btPageBuf);

	return (* btPageBuf);
}

/* 插入数据页 */
DataPage insertDataPage(uint page_id, int index, uint key, Data* data) {

	DataPage buf;
	DataPage* dataPageBuf = &buf;

	// BufferPool
	if (dataPageMap.count(page_id))
		*dataPageBuf = *dataPageMap[page_id];

	// 磁盘读取
	else {
		FSEEK_FIXED_READ(fp, page_id, dataPageBuf, sizeof(DataPage));
		dataPageInsertBufferPool(dataPageBuf);
	}
	
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
	dataPageBuf->data[index] = *data;

	dataPageBuf->key_count++;

	// 写入bufferpool
	dataPageInsertBufferPool(dataPageBuf);

	return (*dataPageBuf);
}

/* 分裂数据页 */
uint splitDataPage(DataPage* dataPageBuf, uint page_id, int s) {

	int i, j;
	int n = dataPageBuf->key_count;
	uint ap;

	MALLOC_PAGE(apDataPageBuf, DataPage);

	for (i = s + 1, j = 0; i < n; i++, j++) {
		apDataPageBuf->key[j] = dataPageBuf->key[i];
		apDataPageBuf->data[j] = dataPageBuf->data[i];
	}
	uint page_count = buffer_pool_file_head.page_count;

	apDataPageBuf->page_type = DATA_PAGE_TYPE;
	apDataPageBuf->key_count = n - s - 1;
	apDataPageBuf->is_inc_insert = true;
	apDataPageBuf->is_dec_insert = true;
	apDataPageBuf->page_id = page_count;
	apDataPageBuf->parent_page_id = dataPageBuf->parent_page_id;
	apDataPageBuf->prev_page_id = page_id;
	apDataPageBuf->next_page_id = dataPageBuf->next_page_id;

	// 写入磁盘文件
	FSEEK_END_WRITE(fp, apDataPageBuf, sizeof(DataPage), page_count);
	// 写入bufferpool
	dataPageInsertBufferPool(apDataPageBuf);

	ap = page_count;

	updateFileHead(1);

	dataPageInsertBufferPool(dataPageBuf);

	dataPageBuf->key_count = s + 1;
	dataPageBuf->next_page_id = ap;

	// 写入bufferpool
	dataPageInsertBufferPool(dataPageBuf);

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
		apBTPageBuf->child_page_id[j] = btPageBuf->child_page_id[i];
	}
	uint page_count = buffer_pool_file_head.page_count;
	apBTPageBuf->page_type = BT_PAGE_TYPE;
	apBTPageBuf->key_count = n - s;
	apBTPageBuf->page_id = page_count;
	apBTPageBuf->parent_page_id = btPageBuf->parent_page_id;

	FSEEK_END_WRITE(fp, apBTPageBuf, sizeof(BTPage), page_count);
	btPageInsertBufferPool(apBTPageBuf);

	ap = page_count;

	updateFileHead(1);
	
	for (int i = 0; i < n - s; i++) {
		if (apBTPageBuf->child_page_id[i] != 0) {
			uint page_id = apBTPageBuf->child_page_id[i];
			if (btPageMap.count(page_id))
				*apBTPageBufChild = *btPageMap[page_id];
			else {
				FSEEK_FIXED_READ(fp, page_id, apBTPageBufChild, sizeof(BTPage));
			}
			
			if (apBTPageBufChild->page_type == DATA_PAGE_TYPE) {
				if (dataPageMap.count(page_id))
					*apDataPageBufChild = *dataPageMap[page_id];
				else {
					FSEEK_FIXED_READ(fp, page_id, apDataPageBufChild, sizeof(DataPage));	
					dataPageInsertBufferPool(apDataPageBufChild);
				}
				apDataPageBufChild->parent_page_id = ap;
				dataPageInsertBufferPool(apDataPageBufChild);
			}
			else {
				btPageInsertBufferPool(apBTPageBufChild);
				apBTPageBufChild->parent_page_id = ap;
				btPageInsertBufferPool(apBTPageBufChild);
			}
		}
	}

	btPageInsertBufferPool(btPageBuf);
	btPageBuf->key_count = s;
	btPageInsertBufferPool(btPageBuf);
	FREE_PAGE(apBTPageBuf);
	FREE_PAGE(apBTPageBufChild);
	FREE_PAGE(apDataPageBufChild);
	return (ap);
}

/* 新建根页面 */
void newRoot(uint root_page_id, uint key, uint ap) {
	uint id;

	MALLOC_PAGE(rootBTPageBuf, BTPage);

	uint page_count = buffer_pool_file_head.page_count;

	rootBTPageBuf->page_type = BT_PAGE_TYPE;
	rootBTPageBuf->key_count = 2;
	rootBTPageBuf->page_id = page_count;
	rootBTPageBuf->child_page_id[0] = root_page_id;
	rootBTPageBuf->child_page_id[1] = ap;
	rootBTPageBuf->key[0] = 0;
	rootBTPageBuf->key[1] = key;


	btPageInsertBufferPool(rootBTPageBuf);

	FSEEK_END_WRITE(fp, rootBTPageBuf, sizeof(BTPage), page_count);

	id = page_count;

	/* 读取原根页面更新parent_page_id位置 */
	if (btPageMap.count(root_page_id))
		*rootBTPageBuf = *btPageMap[root_page_id];
	else {
		FSEEK_FIXED_READ(fp, root_page_id, rootBTPageBuf, sizeof(BTPage));
		btPageInsertBufferPool(rootBTPageBuf);
	}
	rootBTPageBuf->parent_page_id = id;
	btPageInsertBufferPool(rootBTPageBuf);


	/* 读取分裂页面更新parent_page_id位置 */
	if (btPageMap.count(ap))
		*rootBTPageBuf = *btPageMap[ap];
	else {
		FSEEK_FIXED_READ(fp, ap, rootBTPageBuf, sizeof(BTPage));
		btPageInsertBufferPool(rootBTPageBuf);
	}
	rootBTPageBuf->parent_page_id = id;
	btPageInsertBufferPool(rootBTPageBuf);

	updateFileHead(id);
	FREE_PAGE(rootBTPageBuf);
}

/* 将数据插入B+树 */
void insertBPlusTree(uint root_page_id, uint key, uint page_id, int i, Data* data) {
	int s = 0;
	uint ap = 0;
	bool finished = false;
	bool needNewRoot = false;
	DataPage dataPageBuf;
	BTPage btPageBuf;
	dataPageBuf = insertDataPage(page_id, i, key, data);
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
		ap = splitDataPage(&dataPageBuf, page_id, s);
		key = dataPageBuf.key[s];
		page_id = dataPageBuf.parent_page_id;
		if (btPageMap.count(page_id))
			btPageBuf = *btPageMap[page_id];
		else {
			FSEEK_FIXED_READ(fp, page_id, &btPageBuf, sizeof(BTPage));
			btPageInsertBufferPool(&btPageBuf);
		}
		i = searchBTIndex(&btPageBuf, key);
	}

	while (!needNewRoot && !finished) {
		btPageBuf = insertBTPage(page_id, i, key, ap);
		if (btPageBuf.key_count <= BT_PAGE_KEY_MAX) {
			finished = true;
		}
		else {
			s = (BT_PAGE_KEY_MAX + 1) / 2-1;
			ap = splitBTPage(&btPageBuf, page_id, s);
			key = btPageBuf.key[s];
			if (btPageBuf.parent_page_id != 0) {
				page_id = btPageBuf.parent_page_id;
				if (btPageMap.count(page_id))
					btPageBuf = *btPageMap[page_id];
				else {
					FSEEK_FIXED_READ(fp, page_id, &btPageBuf, sizeof(BTPage));
					btPageInsertBufferPool(&btPageBuf);
				}
				i = searchBTIndex(&btPageBuf, key);
			}
			else {
				needNewRoot = true;
			}
		}
	}
	if (needNewRoot) {
		newRoot(root_page_id, key, ap);
	}
}

/* 插入单条数据 */
bool insertData(Data* data, uint key) {
	fp = OPEN_FILE(FILE_NAME, "rb+");
	Result result;
	searchDataPage(buffer_pool_file_head.root_page_id, key, &result);
	if (result.is_found) {
		cout << "index:" << key << " already exists" << endl;
		CLOSE_FILE(fp);
		return false;
	}
	else {
		insertBPlusTree(buffer_pool_file_head.root_page_id, key, result.page_id, result.index, data);
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
		if (insertData(&data, key)) {
			cnt++; 
		}

	}

	inFile.close();
	return cnt;
}