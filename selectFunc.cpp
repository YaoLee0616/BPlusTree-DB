#include "functions.h"

/* 查找key在索引页中的位置 */
int searchBTIndex(BTPage* btPageBuf, uint key) {
	int left = 0;
	int right = btPageBuf->key_count - 1;
	while (left <= right) {
		int mid = left + (right - left) / 2;
		if (btPageBuf->key[mid] == key) return mid;
		else if (btPageBuf->key[mid] < key) left = mid + 1;
		else right = mid - 1;
	}
	return left;
}

/* 查找key在数据页中的位置 */
int searchDataIndex(DataPage* dataPageBuf, uint key) {
	int left = 0;
	int right = dataPageBuf->key_count - 1;
	while (left <= right) {
		int mid = left + (right - left) / 2;
		if (dataPageBuf->key[mid] == key) return mid;
		else if (dataPageBuf->key[mid] < key) left = mid + 1;
		else right = mid - 1;
	}
	return left;
}

/* 打印字段信息 */
void printHeader(vector<bool> field) {
	string set[5] = { "a","c1","c2","c3","c4" };
	bool has = false;
	for (int i = 0; i < 5; i++) {
		if (field[i]) {
			if (!has) {
				cout << '|';
			}
			if (i != 0) cout << ' ';
			cout << set[i] << '|';
			has = true;
		}
	}
	cout << endl;
}

/* 打印数据信息 */
void printData(DataPage* dataPageBuf, vector<bool> field, int index) {
	bool has = false;
	for (int i = 0; i < 5; i++) {
		if (field[i]) {
			if (!has) cout << '|';
			if (i == 0)
				cout << dataPageBuf->key[index] << '|';
			else if (i == 1)
				cout << ' ' << dataPageBuf->data[index].c1 << '|';
			else if (i == 2)
				cout << ' ' << dataPageBuf->data[index].c2 << '|';
			else if (i == 3)
				cout << ' ' << dataPageBuf->data[index].c3 << '|';
			else
				cout << ' ' << dataPageBuf->data[index].c4 << '|';
			has = true;
		}
	}
	cout << endl;
}

// 查找数据页
void searchDataPage(uint page_id, uint key, Result* result) {

	int i = 0;
	bool is_found = false;
	
	MALLOC_PAGE(dataPageBuf,DataPage);
	MALLOC_PAGE(btPageBuf, BTPage);

	while (page_id != 0 && !is_found) {
		
		if (btPageMap.count(page_id))
			*btPageBuf = *btPageMap[page_id];
		else {
			 FSEEK_FIXED_READ(fp, page_id, btPageBuf, sizeof(BTPage));
		}

		if (btPageBuf->page_type == DATA_PAGE_TYPE) {
			if (dataPageMap.count(page_id))
				*dataPageBuf = *dataPageMap[page_id];
			else {
				FSEEK_FIXED_READ(fp, page_id, dataPageBuf, sizeof(DataPage));
				dataPageInsertBufferPool(dataPageBuf);
			}
			i = searchDataIndex(dataPageBuf, key);
			if (i < dataPageBuf->key_count && dataPageBuf->key[i] == key)
				is_found = true;
			break;
		}
		else {
			btPageInsertBufferPool(btPageBuf);
			i = searchBTIndex(btPageBuf, key);
			page_id = btPageBuf->child_page_id[i - 1];
		}

	}
	result->page_id = page_id;
	result->index = i;
	result->is_found = is_found;

	FREE_PAGE(dataPageBuf);
	FREE_PAGE(btPageBuf);
}

/* 主键等值查询数据 */
bool selectData(uint key, vector<bool> field) {

	fp = OPEN_FILE(FILE_NAME,"rb+");

	Result result;
	searchDataPage(buffer_pool_file_head.root_page_id, key, &result);

	if (!result.is_found) {
		CLOSE_FILE(fp);
		return false;
	}

	else {
		MALLOC_PAGE(dataPageBuf, DataPage);
		if(dataPageMap.count(result.page_id))
			*dataPageBuf= *dataPageMap[result.page_id];
		else {
			FSEEK_FIXED_READ(fp, result.page_id, dataPageBuf, sizeof(DataPage));
			dataPageInsertBufferPool(dataPageBuf);
		}
		printHeader(field);
		printData(dataPageBuf, field, result.index);
		CLOSE_FILE(fp);
		FREE_PAGE(dataPageBuf);
		return true;
	}
}

/* 主键索引范围扫描查询数据 */
void selectRangeData(uint left, uint right, vector<bool> field) {

	fp= OPEN_FILE(FILE_NAME, "rb+");

	Result result;
	if (left < 1) left = 1;
	searchDataPage(buffer_pool_file_head.root_page_id, left, &result);
	MALLOC_PAGE(dataPageBuf, DataPage);
	if (dataPageMap.count(result.page_id))
		*dataPageBuf = *dataPageMap[result.page_id];
	else {
		FSEEK_FIXED_READ(fp, result.page_id, dataPageBuf, sizeof(DataPage));
		dataPageInsertBufferPool(dataPageBuf);
	}

	printHeader(field);
	uint cur = result.index;
	while (dataPageBuf->key[cur] != 0 && dataPageBuf->key[cur] <= right) {

		printData(dataPageBuf, field, cur);
		if (cur < dataPageBuf->key_count - 1) {
			cur++;
		}
		else {
			if (dataPageBuf->next_page_id != 0) {
				uint page_id = dataPageBuf->next_page_id;
				if (dataPageMap.count(page_id))
					*dataPageBuf = *dataPageMap[page_id];
				else {
					FSEEK_FIXED_READ(fp, page_id, dataPageBuf, sizeof(DataPage));
					dataPageInsertBufferPool(dataPageBuf);
				}
				cur = 0;
			}
			else {
				break;
			}

		}
	}
	CLOSE_FILE(fp);
	FREE_PAGE(dataPageBuf);
}

/* 主键索引范围扫描的查询导出数据 */
int outputCsvData(uint left, uint right, string csv_file_name) {
	fp = OPEN_FILE(FILE_NAME, "rb+");
	ofstream outFile(csv_file_name, ios::out);

	Result result;
	if (left < 1) left = 1;
	searchDataPage(buffer_pool_file_head.root_page_id, left, &result);

	int cnt = 0;
	MALLOC_PAGE(dataPageBuf, DataPage);
	if (dataPageMap.count(result.page_id))
		*dataPageBuf = *dataPageMap[result.page_id];
	else {
		FSEEK_FIXED_READ(fp, result.page_id, dataPageBuf, sizeof(DataPage));
		dataPageInsertBufferPool(dataPageBuf);
	}
	uint cur = result.index;
	while (dataPageBuf->key[cur] <= right) {

		outFile << dataPageBuf->key[cur];
		outFile << (char)0x09 << dataPageBuf->data[cur].c1;
		outFile << (char)0x09 << dataPageBuf->data[cur].c2;
		outFile << (char)0x09 << dataPageBuf->data[cur].c3;
		outFile << (char)0x09 << dataPageBuf->data[cur].c4;
		outFile << endl;

		cnt++;

		if (cur < dataPageBuf->key_count - 1) {
			cur++;
		}
		else {
			if (dataPageBuf->next_page_id != 0) {
				uint page_id = dataPageBuf->next_page_id;
				if (dataPageMap.count(page_id))
					*dataPageBuf = *dataPageMap[page_id];
				else {
					FSEEK_FIXED_READ(fp, page_id, dataPageBuf, sizeof(DataPage));
					dataPageInsertBufferPool(dataPageBuf);
				}
				cur = 0;
			}
			else {
				break;
			}
		}
	}
	outFile.close();

	CLOSE_FILE(fp);
	return cnt;
}