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
void searchDataPage(uint page__pos, uint key, Result* result) {

	int i = 0;
	bool is_found = false;

	MALLOC_PAGE(btPageBuf, BTPage);
	MALLOC_PAGE(dataPageBuf, DataPage);

	while (page__pos != 0 && !is_found) {
		FSEEK_FIXED_READ(fp, page__pos, btPageBuf, sizeof(BTPage));

		if (btPageBuf->page_type == 2) {
			FSEEK_FIXED_READ(fp, page__pos, dataPageBuf, sizeof(DataPage));
			i = searchDataIndex(dataPageBuf, key);
			if (i < dataPageBuf->key_count && dataPageBuf->key[i] == key)
				is_found = true;
			break;
		}
		else {
			i = searchBTIndex(btPageBuf, key);
			page__pos = btPageBuf->child_page_pos[i - 1];
		}

	}
	result->pos = page__pos;
	result->index = i;
	result->is_found = is_found;

	FREE_PAGE(btPageBuf);
	FREE_PAGE(dataPageBuf);
}

/* 主键等值查询数据 */
bool selectData(uint key, vector<bool> field) {
	MALLOC_PAGE(fileHeadBuf, FileHead);
	MALLOC_PAGE(dataPageBuf, DataPage);

	OPEN_FILE_READ(FILE_NAME, "rb+", fileHeadBuf, sizeof(FileHead));
	Result result;
	searchDataPage(fileHeadBuf->root_page_pos, key, &result);
	if (!result.is_found) {
		FREE_PAGE(fileHeadBuf);
		FREE_PAGE(dataPageBuf);
		CLOSE_FILE(fp);
		return false;
	}
	else {
		FSEEK_FIXED_READ(fp, result.pos, dataPageBuf, sizeof(DataPage));
		printHeader(field);
		printData(dataPageBuf, field, result.index);
		FREE_PAGE(fileHeadBuf);
		FREE_PAGE(dataPageBuf);
		CLOSE_FILE(fp);
		return true;
	}
}

/* 主键索引范围扫描查询数据 */
void selectRangeData(uint left, uint right, vector<bool> field) {
	MALLOC_PAGE(fileHeadBuf, FileHead);
	MALLOC_PAGE(dataPageBuf, DataPage);

	OPEN_FILE_READ(FILE_NAME, "rb+", fileHeadBuf, sizeof(FileHead));
	Result result;
	searchDataPage(fileHeadBuf->root_page_pos, left, &result);

	FSEEK_FIXED_READ(fp, result.pos, dataPageBuf, sizeof(DataPage));


	printHeader(field);
	uint cur = result.index;
	while (dataPageBuf->key[cur] != 0 && dataPageBuf->key[cur] <= right) {

		printData(dataPageBuf, field, cur);
		if (cur < dataPageBuf->key_count - 1) {
			cur++;
		}
		else {
			if (dataPageBuf->next_page_pos != 0) {
				FSEEK_FIXED_READ(fp, dataPageBuf->next_page_pos, dataPageBuf, sizeof(DataPage));
				cur = 0;
			}
			else {
				break;
			}

		}
	}
	FREE_PAGE(fileHeadBuf);
	FREE_PAGE(dataPageBuf);
	CLOSE_FILE(fp);
}

/* 主键索引范围扫描的查询导出数据 */
int outputCsvData(uint left, uint right, string csv_file_name) {
	ofstream outFile(csv_file_name, ios::out);
	MALLOC_PAGE(fileHeadBuf, FileHead);
	MALLOC_PAGE(dataPageBuf, DataPage);

	OPEN_FILE_READ(FILE_NAME, "rb+", fileHeadBuf, sizeof(FileHead));
	Result result;
	searchDataPage(fileHeadBuf->root_page_pos, left, &result);

	FSEEK_FIXED_READ(fp, result.pos, dataPageBuf, sizeof(DataPage));

	int cnt = 0;

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
			if (dataPageBuf->next_page_pos != 0) {
				FSEEK_FIXED_READ(fp, dataPageBuf->next_page_pos, dataPageBuf, sizeof(DataPage));
				cur = 0;
			}
			else {
				break;
			}
		}
	}
	outFile.close();
	FREE_PAGE(fileHeadBuf);
	FREE_PAGE(dataPageBuf);
	CLOSE_FILE(fp);
	return cnt;
}