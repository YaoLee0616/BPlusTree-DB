#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <sstream>
#include <fstream>
#include <map>

/** �궨�� **/

// �����趨
#define PAGE_MAX_LENGTH 4*1024 // ҳ��С��Ϊ4KB
#define BT_PAGE_KEY_MAX 500 // ����ҳ�����500��key
#define DATA_PAGE_KEY_MAX 7 // Ҷ��ҳ�����7��key
#define BUFFER_POOL_MAX_LENGTH 100 // bufferpool���������Դ�100��ҳ

// ҳ������
#define BT_PAGE_TYPE 1
#define DATA_PAGE_TYPE 2

// �˵���
#define INSERT "insert"
#define SELECT "select"
#define DELETE "delete"
#define RESET ".reset"
#define HELP ".help"
#define SHUTDOWN "shutdown"

// ��ʾ��Ϣ
#define ERROR "the input command is wrong, please re-enter it!" 
#define RESET_SUCCESSFUL "RESET SUCCESSFUL!"

// �ļ���
#define FILE_NAME "data.ibd"

// ���벢��ʼ��ҳ
#define MALLOC_PAGE(p,type) type* p = (type*)malloc(sizeof(type)); memset(p,0,sizeof(type));

// �ͷ�ҳ
#define FREE_PAGE(p) free(p);

// ��mode��ʽ���ļ�
#define OPEN_FILE(file_name,mode) fopen(file_name,mode);

// �ļ�ĩβ��ʼд
#define FSEEK_END_WRITE(fp,buf,size,page_count) fseek(fp, page_count*PAGE_MAX_LENGTH, SEEK_SET); fwrite(buf,size,1,fp);

// ָ��λ�ÿ�ʼ��
#define FSEEK_FIXED_READ(fp,page_id,buf,size) fseek(fp,page_id*PAGE_MAX_LENGTH,SEEK_SET); fread(buf,size,1,fp);  

// ָ��λ�ÿ�ʼд
#define FSEEK_FIXED_WRITE(fp,page_id,buf,size) fseek(fp,page_id*PAGE_MAX_LENGTH,SEEK_SET); fwrite(buf,size,1,fp); 

// �ر��ļ�
#define CLOSE_FILE(fp) fclose(fp);

using namespace std;

/* �ؼ��������� */
typedef unsigned int uint;

/* �ļ�ͷ�ṹ�� */
typedef struct {
    int order; // B+���Ľ���
    uint root_page_id;
    uint page_size; // ҳ��С
    uint page_count; // �Ѵ�ҳ������
}FileHead;

/* ����ҳ�ṹ�� */
typedef struct {
    int page_type; // ҳ������, �����ж�������ҳ, ��������ҳ
    int key_count; // �Ѵ�key������

    uint page_id; 
    uint parent_page_id;

    uint key[BT_PAGE_KEY_MAX + 1]; // ����ֵ
    uint child_page_id[BT_PAGE_KEY_MAX + 1]; // ��ҳ������λ��
}BTPage;

/* ���ݽṹ�� */
typedef struct {
    char c1[129];
    char c2[129];
    char c3[129];
    char c4[81];
}Data;

/* ����ҳ�ṹ�� */
typedef struct {
    int page_type; // ҳ������, �����ж�������ҳ, ��������ҳ
    int key_count; // �Ѵ�key������
    bool is_inc_insert; // �Ƿ��������
    bool is_dec_insert; // �Ƿ�ݼ�����

    uint page_id;
    uint parent_page_id;
    uint prev_page_id;
    uint next_page_id;

    uint key[DATA_PAGE_KEY_MAX + 1]; // һ�����ݵ�keyֵ
    Data data[DATA_PAGE_KEY_MAX + 1]; // һ�����ݵ�dataֵ
}DataPage;

/* ���ҽ���ṹ�� */
typedef struct {
    uint page_id; // ҳ������λ��
    int index; // ����key��λ�û���ҵ�key��λ��
    bool is_found; // �Ƿ��ҵ�
} Result;

/* BufferPool */
typedef struct BTListItem{
    BTPage btPage;
    struct BTListItem* next;
}BTPageListItem;

typedef struct DataListItem {
    DataPage dataPage;
    struct DataListItem* next;
}DataPageListItem;


/* ȫ�ֱ��� */
extern FILE* fp; // ȫ�ֱ��� fp

// file_head
extern FileHead buffer_pool_file_head;

// all_list
extern BTPageListItem bt_page_all_list_head;
extern DataPageListItem data_page_all_list_head;
extern int bt_page_all_list_count;
extern int data_page_all_list_count;

// flush_list
extern BTPageListItem bt_page_flush_list_head;
extern DataPageListItem data_page_flush_list_head;
extern int bt_page_flush_list_count;
extern int data_page_flush_list_count;

// all_list_hash_map
extern map<int, BTPage*> btPageMap;
extern map<int, DataPage*> dataPageMap;

// flush_list_hash_map
extern map<int, BTPage*> flushListBTPageMap;
extern map<int, DataPage*> flushListDataPageMap;


/* �ӿ� */
bool insertData(Data* data, uint key); // ���뵥������
int loadCsvData(string csv_file_name); // ���ļ�����ķ�ʽ�����������

int searchBTIndex(BTPage* btPageBuf, uint key); // ����key������ҳ�е�λ��
void searchDataPage(uint page_id, uint key, Result* result); // ��������ҳ
bool selectData(uint key, vector<bool> field); // ������ֵ��ѯ����
void selectRangeData(uint left, uint right, vector<bool> field); // ����������Χɨ���ѯ����
int outputCsvData(uint left, uint right, string csv_file_name); // ����������Χɨ��Ĳ�ѯ��������

void deleteData();

void btPageInsertBufferPool(BTPage* btPage); // ������ҳ����BufferPool 
void dataPageInsertBufferPool(DataPage* dataPage); // ������ҳ����BufferPool 

void free(); // �ͷ�BufferPool
void flush(); // ��flush_listˢ�����

void btPageInsertFlushPool(BTPage* btPage); // ������ҳ����FlushList 
void dataPageInsertFlushPool(DataPage* dataPage); // ������ҳ����FlushList 