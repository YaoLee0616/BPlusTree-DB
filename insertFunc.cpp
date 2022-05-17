#include "functions.h"

DataNode Insert(uint node_pos, uint index, uint key, Data data) {
	DataNode buf;
	DataNode* nodeBuf = &buf;

	FSEEK_FIXED_READ(fp, node_pos, nodeBuf, sizeof(DataNode));

	uint j;
	for (j = nodeBuf->Count; j > index; j--) {
		nodeBuf->key[j ] = nodeBuf->key[j-1];
		nodeBuf->data[j] = nodeBuf->data[j-1];
	}
	nodeBuf->key[index] = key;
	nodeBuf->data[index] = data;

	nodeBuf->Count++;
	FSEEK_FIXED_WRITE(fp, node_pos, nodeBuf, sizeof(DataNode));
	return (*nodeBuf);
}

void insertBTree(uint head_pos, uint key, uint node_pos, uint i, Data data) {
	uint s = 0;
	uint finished = 0;
	uint needNewRoot = 0;
	DataNode nodeBuf;
	while (needNewRoot == 0 && finished == 0) {
		nodeBuf = Insert(node_pos, i, key, data);

		if (nodeBuf.Count < M) {
			finished = 1;
		}
	}
}

void insertData(Data data,uint key) {
	MALLOC_NODE(headBuf, FileHead);

	OPEN_FILE_READ(FILE_NAME, "rb+", headBuf, sizeof(FileHead));
	Result result;
	searchBTree(headBuf->rootPos, key, &result);
	if (result.found) {
		cout << "索引为" << key << "的记录已存在" << endl;
	}
	else {
		insertBTree(headBuf->rootPos, key, result.ptr, result.index, data);
	}

	FREE_NODE(headBuf);
	CLOSE_FILE(fp);
}