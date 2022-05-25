#define _CRT_SECURE_NO_WARNINGS

#include<iostream>
#include <string>
#include <cstring>
#include <vector>
#include<sstream>
#include<fstream>

extern FILE* fp; // ȫ�ֱ��� fp

/** �궨�� **/
#define PAGE_MAX_LENGTH 4*1024 // ҳ��С��Ϊ4KB
#define BT_NODE_MAX 500 // �����ڵ������500��key
#define DATA_NODE_MAX 7 // Ҷ�ӽڵ������7��key

#define INSERT "insert"
#define SELECT "select"
#define DELETE "delete"
#define RESET "reset"
#define SHUTDOWN "shutdown"

#define ERROR "���������������������" 

#define FILE_NAME "data"

// ��ʼ�����
#define MALLOC_NODE(p,type) type* p = (type*)malloc(sizeof(type)); memset(p,0,sizeof(type));

// �ͷŽ��
#define FREE_NODE(p) free(p);

// ��mode��ʽ���ļ�
#define OPEN_FILE(file_name,mode) fopen(file_name,mode)

// ��mode��ʽ���ļ�,�������ļ�ͷ���
#define OPEN_FILE_READ(file_name,mode,buf,size) fp = OPEN_FILE(file_name,mode); fread(buf,size,1,fp);

// �ļ�ĩβ��ʼд
#define FSEEK_END_WRITE(fp,pos,buf,size,pageCount) fseek(fp, pageCount*PAGE_MAX_LENGTH, SEEK_SET);pos=ftell(fp); fwrite(buf,size,1,fp);

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
    int orderNum; // B+���Ľ���
    uint pageSize; // ҳ��С
    uint pageCount; // B+�����ܽ������
    uint rootPos; // ��������ڵ����ݿ��λ�ã�����ֵ��������ļ�ͷ����ƫ��ֵ��
}FileHead;

/* �������ṹ�� */
typedef struct {
    int nodeType; // ������ͣ������ж���������㣬�������ݽ��
    int Count; // һ������ҳ���Ѵ��˶���key
    uint parent; // �����
    uint key[BT_NODE_MAX + 1]; // ����ֵ
    uint ptr[BT_NODE_MAX + 1]; // �ӽ��λ��
}BTNode;

/* ���ݽṹ�� */
typedef struct {
    char c1[129];
    char c2[129];
    char c3[129];
    char c4[81];
}Data;

/* ���ݽ��ṹ�� */
typedef struct {
    int nodeType; // ������ͣ������ж���������㣬�������ݽ��
    int Count; // һ������ҳ���Ѵ��˶���key
    bool Is_inc_insert; // �Ƿ��������
    bool Is_dec_insert; // �Ƿ�ݼ�����
    uint parent; // �����
    uint prevPtr; // ǰ�����ݽ��
    uint nextPtr; // ������ݽ��
    uint key[DATA_NODE_MAX + 1]; // ���ݶԵ�keyֵ
    Data data[DATA_NODE_MAX + 1]; // ���ݶԵ�dataֵ
}DataNode;

/* ���ҽ���ṹ�� */
typedef struct {
    uint ptr; // ָ�����λ��
    int index; // �ڽ���еĹؼ������
    bool found; // �Ƿ��ҵ�
} Result;

/* �ӿ� */
bool insertData(Data data, uint key); // ���뵥������
int loadCsvData(string csv_file_name); // ���ļ�����ķ�ʽ�����������

int searchBTIndex(BTNode* btNodeBuf, uint key); // ����key����������е�λ��
void searchDataNode(uint node_pos, uint key, Result* result); // �������ݽ��
bool selectData(uint key, vector<bool> field); // ������ֵ��ѯ����
void selectRangeData(uint left, uint right, vector<bool> field); // ����������Χɨ���ѯ����
int outputCsvData(uint left,uint right,string csv_file_name); // ����������Χɨ��Ĳ�ѯ��������

void deleteData();