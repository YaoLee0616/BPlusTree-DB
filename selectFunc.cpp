#include "functions.h"


uint searchBTNode(BTNode* nodeBuf, uint key) {
	uint i = 1;
	while (i <= nodeBuf->Count && key > nodeBuf->key[i]) {
		i++;
	}
	return i;
}

uint searchDataNode(DataNode* nodeBuf, uint key) {
	uint i = 0;
	while (i < nodeBuf->Count && key > nodeBuf->key[i]) {
		i++;
	}
	return i;
}

void selectData(uint key) {
	MALLOC_NODE(headBuf, FileHead);
	MALLOC_NODE(nodeBuf, DataNode);

	OPEN_FILE_READ(FILE_NAME, "rb+", headBuf, sizeof(FileHead));
	Result result;
	searchBTree(headBuf->rootPos, key, &result);
	if (!result.found) {
		cout << "未找到索引为" << key << "的记录" << endl;
	}
	else {
		FSEEK_FIXED_READ(fp, result.ptr, nodeBuf, sizeof(DataNode));
		cout << key << '|' << nodeBuf->data[result.index].c1 << '|' << nodeBuf->data[result.index].c2 << '|' << nodeBuf->data[result.index].c3 << '|' << nodeBuf->data[result.index].c4 << endl;
	}
	
	FREE_NODE(headBuf);
	FREE_NODE(nodeBuf);
	CLOSE_FILE(fp);
}
void searchBTree(uint node_pos, uint key, Result* result) {

	uint i = 0;
	bool found = false;

	MALLOC_NODE(btNodeBuf, BTNode);
	MALLOC_NODE(dataNodeBuf, DataNode);

	while (node_pos != 0 && !found) {
		FSEEK_FIXED_READ(fp, node_pos, btNodeBuf, sizeof(BTNode));

		if (btNodeBuf->nodeType == 2) {
			FSEEK_FIXED_READ(fp, node_pos, dataNodeBuf, sizeof(DataNode));
			i = searchDataNode(dataNodeBuf, key);
			if(i<dataNodeBuf->Count&& dataNodeBuf ->key[i] == key)
				found = true;
			break;
		}
		else {
			i = searchBTNode(btNodeBuf, key);
			node_pos = btNodeBuf->ptr[i - 1];
		}

	}
	result->ptr = node_pos;
	result->index = i;
	result->found = found;

	FREE_NODE(btNodeBuf);
	FREE_NODE(dataNodeBuf);
}