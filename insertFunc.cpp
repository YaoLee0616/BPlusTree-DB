#include "functions.h"

/* 获取文件中页的数量 */
uint getPageCount() {
	MALLOC_NODE(fileHeadBuf, FileHead);
	FSEEK_FIXED_READ(fp, 0, fileHeadBuf, sizeof(FileHead));
	uint pageCount = fileHeadBuf->pageCount;
	FREE_NODE(fileHeadBuf);
	return pageCount;
}

/* 更新页头结点信息 */
void updateFileHead(uint new_head_pos) {
	MALLOC_NODE(fileHeadBuf, FileHead);
	FSEEK_FIXED_READ(fp, 0, fileHeadBuf, sizeof(FileHead));
	if (new_head_pos > 0) {
		fileHeadBuf->rootPos = new_head_pos;
	}
	fileHeadBuf->pageCount++;
	FSEEK_FIXED_WRITE(fp, 0, fileHeadBuf, sizeof(FileHead));
	FREE_NODE(fileHeadBuf);
}

/* 插入索引结点 */
BTNode insertBTNode(uint node_pos, int index, uint key, uint ap) {

	BTNode buf;
	BTNode* btNodeBuf = &buf;

	FSEEK_FIXED_READ(fp, node_pos, btNodeBuf, sizeof(BTNode));

	int j;
	for (j = btNodeBuf->Count - 1; j >= index; j--) {
		btNodeBuf->key[j + 1] = btNodeBuf->key[j];
		btNodeBuf->ptr[j + 1] = btNodeBuf->ptr[j];
	}
	btNodeBuf->key[index] = key;
	btNodeBuf->ptr[index] = ap;

	btNodeBuf->Count++;
	FSEEK_FIXED_WRITE(fp, node_pos, btNodeBuf, sizeof(BTNode));
	return (*btNodeBuf);
}

/* 插入数据结点 */
DataNode insertDataNode(uint node_pos, int index, uint key, Data data) {
	DataNode buf;
	DataNode* dataNodeBuf = &buf;

	FSEEK_FIXED_READ(fp, node_pos, dataNodeBuf, sizeof(DataNode));
	if (index != 0) dataNodeBuf->Is_dec_insert = false;
	if (index != dataNodeBuf->Count) dataNodeBuf->Is_inc_insert = false;

	int j;
	if (dataNodeBuf->Count != 0) {
		for (j = dataNodeBuf->Count - 1; j >= index; j--) {
			dataNodeBuf->key[j + 1] = dataNodeBuf->key[j];
			dataNodeBuf->data[j + 1] = dataNodeBuf->data[j];
		}
	}

	dataNodeBuf->key[index] = key;
	dataNodeBuf->data[index] = data;

	dataNodeBuf->Count++;
	FSEEK_FIXED_WRITE(fp, node_pos, dataNodeBuf, sizeof(DataNode));
	return (*dataNodeBuf);
}

/* 分裂数据结点 */
uint splitDataNode(DataNode* dataNodeBuf, uint node_pos, int s) {

	int i, j;
	int n = dataNodeBuf->Count;
	uint ap;

	MALLOC_NODE(apDataNodeBuf, DataNode);

	for (i = s + 1, j = 0; i < n; i++, j++) {
		apDataNodeBuf->key[j] = dataNodeBuf->key[i];
		apDataNodeBuf->data[j] = dataNodeBuf->data[i];
	}
	apDataNodeBuf->nodeType = 2;
	apDataNodeBuf->Count = n - s - 1;
	apDataNodeBuf->Is_inc_insert = true;
	apDataNodeBuf->Is_dec_insert = true;
	apDataNodeBuf->parent = dataNodeBuf->parent;
	apDataNodeBuf->prevPtr = node_pos;
	apDataNodeBuf->nextPtr = dataNodeBuf->nextPtr;

	uint pageCount = getPageCount();
	FSEEK_END_WRITE(fp, ap, apDataNodeBuf, sizeof(DataNode), pageCount);

	updateFileHead(0);

	dataNodeBuf->Count = s + 1;
	dataNodeBuf->nextPtr = ap;
	FSEEK_FIXED_WRITE(fp, node_pos, dataNodeBuf, sizeof(DataNode));
	FREE_NODE(apDataNodeBuf);
	return ap;
}

/* 分裂索引结点 */
uint splitBTNode(BTNode* btNodeBuf, uint node_pos, int s) {

	int i, j;
	int n = btNodeBuf->Count;
	uint ap;

	MALLOC_NODE(apBTNodeBuf, BTNode);
	MALLOC_NODE(apBTNodeBufChild, BTNode);
	MALLOC_NODE(apDataNodeBufChild, DataNode);

	for (i = s, j = 0; i < n; i++, j++) {
		apBTNodeBuf->key[j] = btNodeBuf->key[i];
		apBTNodeBuf->ptr[j] = btNodeBuf->ptr[i];
	}
	apBTNodeBuf->nodeType = 1;
	apBTNodeBuf->Count = n - s;
	apBTNodeBuf->parent = btNodeBuf->parent;

	uint pageCount = getPageCount();
	FSEEK_END_WRITE(fp, ap, apBTNodeBuf, sizeof(BTNode), pageCount);

	updateFileHead(0);

	for (int i = 0; i < n - s; i++) {
		if (apBTNodeBuf->ptr[i] != 0) {
			FSEEK_FIXED_READ(fp, apBTNodeBuf->ptr[i], apBTNodeBufChild, sizeof(BTNode));
			if (apBTNodeBufChild->nodeType == 2) {
				FSEEK_FIXED_READ(fp, apBTNodeBuf->ptr[i], apDataNodeBufChild, sizeof(DataNode));
				apDataNodeBufChild->parent = ap;
				FSEEK_FIXED_WRITE(fp, apBTNodeBuf->ptr[i], apDataNodeBufChild, sizeof(DataNode));
			}
			else {
				apBTNodeBufChild->parent = ap;
				FSEEK_FIXED_WRITE(fp, apBTNodeBuf->ptr[i], apBTNodeBufChild, sizeof(BTNode));
			}
		}
	}

	btNodeBuf->Count = s;

	FSEEK_FIXED_WRITE(fp, node_pos, btNodeBuf, sizeof(BTNode));
	FREE_NODE(apBTNodeBuf);
	FREE_NODE(apBTNodeBufChild);
	FREE_NODE(apDataNodeBufChild);
	return (ap);
}

/* 新建根结点 */
void newRoot(uint head_pos, uint key, uint ap) {
	uint pos;
	MALLOC_NODE(rootBTNodeBuf, BTNode);
	rootBTNodeBuf->nodeType = 1;
	rootBTNodeBuf->Count = 2;
	rootBTNodeBuf->ptr[0] = head_pos;
	rootBTNodeBuf->ptr[1] = ap;
	rootBTNodeBuf->key[0] = 0;
	rootBTNodeBuf->key[1] = key;

	uint pageCount = getPageCount();
	FSEEK_END_WRITE(fp, pos, rootBTNodeBuf, sizeof(BTNode), pageCount);

	/* 读取原根结点更新parent位置 */
	FSEEK_FIXED_READ(fp, head_pos, rootBTNodeBuf, sizeof(BTNode));
	rootBTNodeBuf->parent = pos;
	FSEEK_FIXED_WRITE(fp, head_pos, rootBTNodeBuf, sizeof(BTNode));

	/* 读取分裂结点更新parent位置 */
	FSEEK_FIXED_READ(fp, ap, rootBTNodeBuf, sizeof(BTNode));
	rootBTNodeBuf->parent = pos;
	FSEEK_FIXED_WRITE(fp, ap, rootBTNodeBuf, sizeof(BTNode));

	updateFileHead(pos);

	FREE_NODE(rootBTNodeBuf);
}

/* 将数据插入树 */
void insertBTree(uint head_pos, uint key, uint node_pos, int i, Data data) {
	int s = 0;
	uint ap = 0;
	bool finished = false;
	bool needNewRoot = false;
	DataNode dataNodeBuf;
	dataNodeBuf = insertDataNode(node_pos, i, key, data);
	if (dataNodeBuf.Count <= DATA_NODE_MAX) finished = true;

	if (!finished) {
		if (dataNodeBuf.Is_inc_insert) {
			s = DATA_NODE_MAX - 1;
		}
		else if (dataNodeBuf.Is_dec_insert) {
			s = 0;
		}
		else {
			s = (DATA_NODE_MAX + 1) / 2 - 1;
		}
		ap = splitDataNode(&dataNodeBuf, node_pos, s);
		key = dataNodeBuf.key[s];
		node_pos = dataNodeBuf.parent;
		BTNode btNodeBuf;
		FSEEK_FIXED_READ(fp, node_pos, &btNodeBuf, sizeof(BTNode));
		i = searchBTIndex(&btNodeBuf, key);
	}

	BTNode btNodeBuf;
	while (!needNewRoot && !finished) {
		btNodeBuf = insertBTNode(node_pos, i, key, ap);
		if (btNodeBuf.Count <= BT_NODE_MAX) {
			finished = true;
		}
		else {
			s = (BT_NODE_MAX + 1) / 2 - 1;
			ap = splitBTNode(&btNodeBuf, node_pos, s);
			key = btNodeBuf.key[s];
			if (btNodeBuf.parent != 0) {
				node_pos = btNodeBuf.parent;
				FSEEK_FIXED_READ(fp, node_pos, &btNodeBuf, sizeof(BTNode));
				i = searchBTIndex(&btNodeBuf, key);
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
	MALLOC_NODE(fileHeadBuf, FileHead);

	OPEN_FILE_READ(FILE_NAME, "rb+", fileHeadBuf, sizeof(FileHead));
	Result result;
	searchDataNode(fileHeadBuf->rootPos, key, &result);
	if (result.found) {
		cout << "index:" << key << " already exists" << endl;
		FREE_NODE(fileHeadBuf);
		CLOSE_FILE(fp);
		return false;
	}
	else {
		insertBTree(fileHeadBuf->rootPos, key, result.ptr, result.index, data);
		FREE_NODE(fileHeadBuf);
		CLOSE_FILE(fp);
		return true;
	}
}

/* 以文件导入的方式插入大量数据 */
int loadCsvData(string csv_file_name) {
	ifstream inFile(csv_file_name, ios::in);
	string lineStr;
	vector<vector<string>> strArray;
	int cnt = 0;
	Data data;
	while (getline(inFile, lineStr))
	{
		stringstream ss(lineStr);
		string str;
		vector<string> strList;
		while (getline(ss, str, (char)0x09))
			strList.push_back(str);
		strArray.push_back(strList);
		uint key = stoi(strList[0]);
		strcpy(data.c1, strList[1].c_str());
		strcpy(data.c2, strList[2].c_str());
		strcpy(data.c3, strList[3].c_str());
		strcpy(data.c4, strList[4].c_str());
		if(insertData(data, key))
			cnt++;
	}

	inFile.close();
	return cnt;
}