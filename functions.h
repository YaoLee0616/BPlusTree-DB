#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <sstream>
#include <fstream>

extern FILE* fp; // ȫ�ֱ��� fp

/** �궨�� **/
#define PAGE_MAX_LENGTH 4*1024 // ҳ��С��Ϊ4KB
#define BT_PAGE_KEY_MAX 500 // ����ҳ�����500��key
#define DATA_PAGE_KEY_MAX 7 // Ҷ��ҳ�����7��key

#define INSERT "insert"
#define SELECT "select"
#define DELETE "delete"
#define RESET "reset"
#define SHUTDOWN "shutdown"

#define ERROR "���������������������" 

#define FILE_NAME "data.ibd"

// ���벢��ʼ��ҳ
#define MALLOC_PAGE(p,type) type* p = (type*)malloc(sizeof(type)); memset(p,0,sizeof(type));

// �ͷ�ҳ
#define FREE_PAGE(p) free(p);

// ��mode��ʽ���ļ�
#define OPEN_FILE(file_name,mode) fopen(file_name,mode)

// ��mode��ʽ���ļ�,�������ļ�ͷҳ��
#define OPEN_FILE_READ(file_name,mode,buf,size) fp = OPEN_FILE(file_name,mode); fread(buf,size,1,fp);

// �ļ�ĩβ��ʼд
#define FSEEK_END_WRITE(fp,pos,buf,size,page_count) fseek(fp, page_count*PAGE_MAX_LENGTH, SEEK_SET);pos=ftell(fp); fwrite(buf,size,1,fp);

// ָ��λ�ÿ�ʼ��
#define FSEEK_FIXED_READ(fp,pos,buf,size) fseek(fp,pos,SEEK_SET); fread(buf,size,1,fp);

// ָ��λ�ÿ�ʼд
#define FSEEK_FIXED_WRITE(fp,pos,buf,size) fseek(fp,pos,SEEK_SET); fwrite(buf,size,1,fp);

// �ر��ļ�
#define CLOSE_FILE(fp) fclose(fp)

using namespace std;

/* �ؼ��������� */
typedef unsigned int uint;

/* �ļ�ͷ�ṹ�� */
typedef struct {
    int order; // B+���Ľ���
    uint page_size; // ҳ��С
    uint page_count; // �Ѵ�ҳ������
    uint root_page_pos; // b+����ҳ������λ�ã�����ֵ��������ļ�ͷ����ƫ��ֵ��
}FileHead;

/* ����ҳ�ṹ�� */
typedef struct {
    int page_type; // ҳ�����ͣ������ж�������ҳ����������ҳ
    int key_count; // �Ѵ�key������
    uint parent_page_pos; // ��ҳ������λ��
    uint key[BT_PAGE_KEY_MAX + 1]; // ����ֵ
    uint child_page_pos[BT_PAGE_KEY_MAX + 1]; // ��ҳ������λ��
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
    int page_type; // ҳ�����ͣ������ж�������ҳ����������ҳ
    int key_count; // �Ѵ�key������
    bool is_inc_insert; // �Ƿ��������
    bool is_dec_insert; // �Ƿ�ݼ�����
    uint parent_page_pos; // ��ҳ������λ��
    uint prev_page_pos; // ǰһ��ҳ������λ��
    uint next_page_pos; // ��һ��ҳ������λ��
    uint key[DATA_PAGE_KEY_MAX + 1]; // һ�����ݵ�keyֵ
    Data data[DATA_PAGE_KEY_MAX + 1]; // һ�����ݵ�dataֵ
}DataPage;

/* ���ҽ���ṹ�� */
typedef struct {
    uint pos; // ָ���λ��
    int index; // ��ҳ�еĹؼ������
    bool is_found; // �Ƿ��ҵ�
} Result;


/* �ӿ� */
bool insertData(Data data, uint key); // ���뵥������
int loadCsvData(string csv_file_name); // ���ļ�����ķ�ʽ�����������

int searchBTIndex(BTPage* btPageBuf, uint key); // ����key������ҳ�е�λ��
void searchDataPage(uint page__pos, uint key, Result* result); // ��������ҳ
bool selectData(uint key, vector<bool> field); // ������ֵ��ѯ����
void selectRangeData(uint left, uint right, vector<bool> field); // ����������Χɨ���ѯ����
int outputCsvData(uint left, uint right, string csv_file_name); // ����������Χɨ��Ĳ�ѯ��������

void deleteData();