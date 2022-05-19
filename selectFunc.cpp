#include "functions.h"

/* 查找key在索引结点中的位置 */
int searchBTIndex(BTNode* btNodeBuf, uint key) {
	int left = 0;
	int right = btNodeBuf->Count - 1;
	while (left <= right) {
		int mid = left + (right - left) / 2;
		if (btNodeBuf->key[mid] == key) return mid;
		else if (btNodeBuf->key[mid] < key) left = mid + 1;
		else right = mid - 1;
	}
	return left;
}

/* 查找key在数据结点中的位置 */
int searchDataIndex(DataNode* dataNodeBuf, uint key) {
	int left = 0;
	int right = dataNodeBuf->Count - 1;
	while (left <= right) {
		int mid = left + (right - left) / 2;
		if (dataNodeBuf->key[mid] == key) return mid;
		else if (dataNodeBuf->key[mid] < key) left = mid + 1;
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
			cout << set[i]<<'|';
			has = true;
		}
	}
	cout << endl;
}

/* 打印数据信息 */
void printData(DataNode* dataNodeBuf, vector<bool> field,int index) {
	bool has = false;
	for (int i = 0; i < 5; i++) {
		if (field[i]) {
			if (!has) cout << '|';
			if (i == 0)
				cout << dataNodeBuf->key[index]<<'|';
			else if (i == 1)
				cout <<' '<< dataNodeBuf->data[index].c1<<'|';
			else if (i == 2)
				cout <<' '<< dataNodeBuf->data[index].c2 <<'|';
			else if (i == 3)
				cout << ' '<<dataNodeBuf->data[index].c3<<'|';
			else
				cout <<' ' <<dataNodeBuf->data[index].c4<<'|';
			has = true;
		}
	}
	cout << endl;
}

// 查找数据结点
void searchDataNode(uint node_pos, uint key, Result* result) {

	int i = 0;
	bool found = false;

	MALLOC_NODE(btNodeBuf, BTNode);
	MALLOC_NODE(dataNodeBuf, DataNode);

	while (node_pos != 0 && !found) {
		FSEEK_FIXED_READ(fp, node_pos, btNodeBuf, sizeof(BTNode));

		if (btNodeBuf->nodeType == 2) {
			FSEEK_FIXED_READ(fp, node_pos, dataNodeBuf, sizeof(DataNode));
			i = searchDataIndex(dataNodeBuf, key);
			if (i < dataNodeBuf->Count && dataNodeBuf->key[i] == key)
				found = true;
			break;
		}
		else {
			i = searchBTIndex(btNodeBuf, key);
			node_pos = btNodeBuf->ptr[i - 1];
		}

	}
	result->ptr = node_pos;
	result->index = i;
	result->found = found;

	FREE_NODE(btNodeBuf);
	FREE_NODE(dataNodeBuf);
}

/* 主键等值查询数据 */
bool selectData(uint key, vector<bool> field) {
	MALLOC_NODE(fileHeadBuf, FileHead);
	MALLOC_NODE(dataNodeBuf, DataNode);

	OPEN_FILE_READ(FILE_NAME, "rb+", fileHeadBuf, sizeof(FileHead));
	Result result;
	searchDataNode(fileHeadBuf->rootPos, key, &result);
	if (!result.found) {
		FREE_NODE(fileHeadBuf);
		FREE_NODE(dataNodeBuf);
		CLOSE_FILE(fp);
		return false;
	}
	else {
		FSEEK_FIXED_READ(fp, result.ptr, dataNodeBuf, sizeof(DataNode));
		printHeader(field);
		printData(dataNodeBuf, field, result.index);
		FREE_NODE(fileHeadBuf);
		FREE_NODE(dataNodeBuf);
		CLOSE_FILE(fp);
		return true;
	}
}

/* 主键索引范围扫描查询数据 */
void selectRangeData(uint left, uint right,vector<bool> field) {
	MALLOC_NODE(fileHeadBuf, FileHead);
	MALLOC_NODE(dataNodeBuf, DataNode);

	OPEN_FILE_READ(FILE_NAME, "rb+", fileHeadBuf, sizeof(FileHead));
	Result result;
	searchDataNode(fileHeadBuf->rootPos, left, &result);

	FSEEK_FIXED_READ(fp, result.ptr, dataNodeBuf, sizeof(DataNode));


	printHeader(field);
	uint cur = result.index;
	while (dataNodeBuf->key[cur] <= right) {

		printData(dataNodeBuf, field, cur);
		if (cur < dataNodeBuf->Count - 1) {
			cur++;
		}
		else {
			if (dataNodeBuf->nextPtr != 0) {
				FSEEK_FIXED_READ(fp, dataNodeBuf->nextPtr, dataNodeBuf, sizeof(DataNode));
				cur = 0;
			}
			else {
				break;
			}

		}
	}
	FREE_NODE(fileHeadBuf);
	FREE_NODE(dataNodeBuf);
	CLOSE_FILE(fp);
}

/* 主键索引范围扫描的查询导出数据 */
int outputCsvData(uint left, uint right, string csv_file_name) {
	ofstream outFile(csv_file_name, ios::out);
	MALLOC_NODE(fileHeadBuf, FileHead);
	MALLOC_NODE(dataNodeBuf, DataNode);

	OPEN_FILE_READ(FILE_NAME, "rb+", fileHeadBuf, sizeof(FileHead));
	Result result;
	searchDataNode(fileHeadBuf->rootPos, left, &result);

	FSEEK_FIXED_READ(fp, result.ptr, dataNodeBuf, sizeof(DataNode));

	int cnt = 0;

	uint cur = result.index;
	while (dataNodeBuf->key[cur] <= right) {

		outFile << dataNodeBuf->key[cur];
		outFile << (char)0x09 << dataNodeBuf->data[cur].c1;
		outFile << (char)0x09 << dataNodeBuf->data[cur].c2;
		outFile << (char)0x09 << dataNodeBuf->data[cur].c3;
		outFile << (char)0x09 << dataNodeBuf->data[cur].c4;
		outFile << endl;

		cnt++;

		if (cur < dataNodeBuf->Count - 1) {
			cur++;
		}
		else {
			if (dataNodeBuf->nextPtr != 0) {
				FSEEK_FIXED_READ(fp, dataNodeBuf->nextPtr, dataNodeBuf, sizeof(DataNode));
				cur = 0;
			}
			else {
				break;
			}
		}
	}
	outFile.close();
	FREE_NODE(fileHeadBuf);
	FREE_NODE(dataNodeBuf);
	CLOSE_FILE(fp);
	return cnt;
}