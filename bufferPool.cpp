#include "functions.h"

/* ɾ��bt_page_list��βҳ�� */
void btPageDeleteTailPage(BTPageListItem* head, bool is_flush) {
	BTPageListItem* pre = head;
	BTPageListItem* cur = pre;

	while (cur->next != NULL) {
		pre = cur;
		cur = cur->next;
	}
	if (is_flush) {

		//hash_mapɾ��
		flushListBTPageMap.erase(cur->btPage.page_id);

		//flush_listɾ��
		pre->next = NULL;
		bt_page_flush_list_count--;

		// д�����
		FSEEK_FIXED_WRITE(fp, cur->btPage.page_id, &( cur->btPage), sizeof(BTPage));

		// free_page
		FREE_PAGE(cur);
	}
	else {
		if (flushListBTPageMap.count(cur->btPage.page_id)) {
			//hash_mapɾ��
			flushListBTPageMap.erase(cur->btPage.page_id);
			BTPageListItem* p = &bt_page_flush_list_head;
			BTPageListItem* c = p;
			while (c->btPage.page_id != cur->btPage.page_id) {
				p = c;
				c = c->next;
			}
			//flush_listɾ��
			p->next = c->next;
			bt_page_flush_list_count--;
			cout << "111" << endl;
			// д�����
			FSEEK_FIXED_WRITE(fp, cur->btPage.page_id, &(cur->btPage), sizeof(BTPage));
			// free_page
			FREE_PAGE(c);
		}
		//hash_mapɾ��
		btPageMap.erase(cur->btPage.page_id);

		//all_listɾ��
		pre->next = NULL;
		bt_page_all_list_count--;

		// free_page
		FREE_PAGE(cur);
	}
}

/* ������ҳ����FlushList */
void btPageInsertFlushPool(BTPage* btPage) {

	if (flushListBTPageMap.count(btPage->page_id)) {
		*flushListBTPageMap[btPage->page_id] = *btPage;
	}
	else {
		// flush_list����
		MALLOC_PAGE(item, BTPageListItem);
		item->btPage = *btPage;
		item->next = bt_page_flush_list_head.next;
		bt_page_flush_list_head.next = item;
		bt_page_flush_list_count++;

		// hash_map����
		flushListBTPageMap[btPage->page_id] = &(item->btPage);
	}
	if (bt_page_flush_list_count == BUFFER_POOL_MAX_LENGTH) {
		btPageDeleteTailPage(&bt_page_flush_list_head, true);
	}
}

/* ������ҳ����BufferPool */
void btPageInsertBufferPool(BTPage* btPage) {

	if (btPageMap.count(btPage->page_id)) {
		btPageInsertFlushPool(btPage);
		*btPageMap[btPage->page_id] = *btPage;
	}
	else {
		// all_list����
		MALLOC_PAGE(item, BTPageListItem);
		item->btPage = *btPage;
		item->next = bt_page_all_list_head.next;
		bt_page_all_list_head.next = item;
		bt_page_all_list_count++;

		// hash_map����
		btPageMap[btPage->page_id] = &(item->btPage);
	}
	if (bt_page_all_list_count == BUFFER_POOL_MAX_LENGTH) {
		btPageDeleteTailPage(&bt_page_all_list_head, false);
	}
}


/* ɾ��data_page_list��βҳ�� */
void dataPageDeleteTailPage(DataPageListItem* head, bool is_flush) {
	DataPageListItem* pre = head;
	DataPageListItem* cur = pre;

	while (cur->next != NULL) {
		pre = cur;
		cur = cur->next;
	}
	if (is_flush) {

		//hash_mapɾ��
		flushListDataPageMap.erase(cur->dataPage.page_id);

		//flush_listɾ��
		pre->next = NULL;
		data_page_flush_list_count--;

		// д�����
		FSEEK_FIXED_WRITE(fp, cur->dataPage.page_id, &(cur->dataPage), sizeof(DataPage));

		// free_page
		FREE_PAGE(cur);
	}
	else {
		if (flushListDataPageMap.count(cur->dataPage.page_id)) {
			//hash_mapɾ��
			flushListDataPageMap.erase(cur->dataPage.page_id);
			DataPageListItem* p = &data_page_flush_list_head;
			DataPageListItem* c = p;
			while (c->dataPage.page_id != cur->dataPage.page_id) {
				p = c;
				c = c->next;
			}
			//flush_listɾ��
			p->next = c->next;
			data_page_flush_list_count--;

			// д�����
			FSEEK_FIXED_WRITE(fp, cur->dataPage.page_id, &(cur->dataPage), sizeof(DataPage));
			// free_page
			FREE_PAGE(c);
		}
		//hash_mapɾ��
		dataPageMap.erase(cur->dataPage.page_id);

		//all_listɾ��
		pre->next = NULL;
		data_page_all_list_count--;

		// free_page
		FREE_PAGE(cur);
	}
}

/* ������ҳ����FlushList */
void dataPageInsertFlushPool(DataPage* dataPage) {

	if (flushListDataPageMap.count(dataPage->page_id)) {
		*flushListDataPageMap[dataPage->page_id] = *dataPage;
	}
	else {
		// flush_list����
		MALLOC_PAGE(item, DataPageListItem);
		item->dataPage = *dataPage;
		item->next = data_page_flush_list_head.next;
		data_page_flush_list_head.next = item;
		data_page_flush_list_count++;

		// hash_map����
		flushListDataPageMap[dataPage->page_id] = &(item->dataPage);
	}
	if (data_page_flush_list_count == BUFFER_POOL_MAX_LENGTH) {
		dataPageDeleteTailPage(&data_page_flush_list_head, true);
	}
}

/* ������ҳ����BufferPool */
void dataPageInsertBufferPool(DataPage* dataPage) {

	if (dataPageMap.count(dataPage->page_id)) {
		dataPageInsertFlushPool(dataPage);
		*dataPageMap[dataPage->page_id] = *dataPage;
	}
	else {
		// all_list����
		MALLOC_PAGE(item, DataPageListItem);
		item->dataPage = *dataPage;
		item->next = data_page_all_list_head.next;
		data_page_all_list_head.next = item;
		data_page_all_list_count++;

		// hash_map����
		dataPageMap[dataPage->page_id] = &(item->dataPage);
	}
	if (data_page_all_list_count == BUFFER_POOL_MAX_LENGTH) {
		dataPageDeleteTailPage(&data_page_all_list_head, false);
	}
}


void freeBTPageList(BTPageListItem *head) {
	BTPageListItem* p = head;
	BTPageListItem* q = head->next;
	while (q!= NULL) {
		p = q;
		q = q->next;
		FREE_PAGE(p);
	}
}

void freeDataPageList(DataPageListItem* head) {
	DataPageListItem* p = head;
	DataPageListItem* q = head->next;
	while (q != NULL) {
		p = q;
		q = q->next;
		FREE_PAGE(p);
	}
}

/* �ͷ�BufferPool */
void free() {
	freeBTPageList(&bt_page_all_list_head);
	freeBTPageList(&bt_page_flush_list_head);
	freeDataPageList(&data_page_all_list_head);
	freeDataPageList(&data_page_flush_list_head);
}


void flushBTPageList(BTPageListItem* head) {
	fp = OPEN_FILE(FILE_NAME, "rb+");
	BTPageListItem* p = head;
	BTPageListItem* q = head->next;
	while (q != NULL) {
		p = q;
		q = q->next;
		FSEEK_FIXED_WRITE(fp, p->btPage.page_id, &p->btPage, sizeof(BTPage));
		FREE_PAGE(p);
	}
	CLOSE_FILE(fp);
}


void flushDataPageList(DataPageListItem* head) {
	fp = OPEN_FILE(FILE_NAME, "rb+");
	DataPageListItem* p = head;
	DataPageListItem* q = head->next;
	while (q != NULL) {
		p = q;
		q = q->next;
		FSEEK_FIXED_WRITE(fp, p->dataPage.page_id, &p->dataPage, sizeof(DataPage));
		FREE_PAGE(p);
	}
	CLOSE_FILE(fp);

}

/* ��flush_listˢ����� */
void flush() {
	fp = OPEN_FILE(FILE_NAME, "rb+");
	FSEEK_FIXED_WRITE(fp, 0, &buffer_pool_file_head, sizeof(FileHead));
	CLOSE_FILE(fp);
	flushBTPageList(&bt_page_flush_list_head);
	flushDataPageList(&data_page_flush_list_head);

	// flush_list
	bt_page_flush_list_count = 0;
	data_page_flush_list_count = 0;
	bt_page_flush_list_head.next = NULL;
	data_page_flush_list_head.next = NULL;

	// hash_map
	flushListBTPageMap.erase(flushListBTPageMap.begin(), flushListBTPageMap.end());
	flushListDataPageMap.erase(flushListDataPageMap.begin(), flushListDataPageMap.end());
}