#define _CRT_SECURE_NO_WARNINGS
/** ͷ�ļ� **/
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

extern FILE* fp;

/** �궨�� **/
#define PAGE_MAX_LENGTH 4*1024 // ҳ��С��Ϊ4KB
#define M 5
#define INSERT "insert"
#define SELECT "select"
#define DELETE "delete"
#define SHUTDOWN "shutdown"

#define FILE_NAME "data"

#define MALLOC_NODE(p,type) type* p = (type*)malloc(sizeof(type)); memset(p,0,sizeof(type));

#define FREE_NODE(p) free(p);
// ��mode��ʽ���ļ�
#define OPEN_FILE(file_name,mode) fopen(file_name,mode)

#define OPEN_FILE_READ(file_name,mode,buf,size) fp = OPEN_FILE(file_name,mode); fread(buf,size,1,fp);
// �ļ�ĩβ��ʼд
#define FSEEK_END_WRITE(fp,pageCount,pageSize,pos,buf,size) fseek(fp, pageCount*pageSize, SEEK_SET); pageCount++;pos=ftell(fp); fwrite(buf,size,1,fp);

// �ļ�ͷ��ʼд
#define FSEEK_HED_WRITE(fp,pos,buf,size) fseek(fp, 0, SEEK_SET); pos=ftell(fp); fwrite(buf,size,1,fp);

#define FSEEK_FIXED_READ(fp,pos,buf,size) fseek(fp,pos,SEEK_SET); fread(buf,size,1,fp);

#define FSEEK_FIXED_WRITE(fp,pos,buf,size) fseek(fp,pos,SEEK_SET); fwrite(buf,size,1,fp);
// �ر��ļ�
#define CLOSE_FILE(fp) fclose(fp)
#define UINT_MAX_VALUE 4294967295
using namespace std;

/** �ؼ��������� **/
typedef unsigned int uint;

/** �ļ�ͷ�ṹ�� **/
typedef struct {
    uint pageSize; // ҳ��С
    // sint keySize; // ������С
    // sint valueSize; // ������С
    uint orderNum; // B+���Ľ���
    uint pageCount; // B+�����ܽ������
    uint rootPos; // ��������ڵ����ݿ��λ�ã�����ֵ��������ļ�ͷ����ƫ��ֵ��
}FileHead;

/** �������ṹ�� **/
typedef struct {
    uint nodeType; // ������ͣ������ж���������㣬�������ݽ��
    uint Count; // һ������ҳ���Ѵ��˶���key
    uint key[M + 1]; // ����ֵ
    uint ptr[M + 1]; // �ӽ��λ��
}BTNode;

/** ���ݽṹ�� **/
typedef struct {
    char c1[128];
    char c2[128];
    char c3[128];
    char c4[80];
}Data;

/** ���ݽ��ṹ�� **/
typedef struct {
    uint nodeType; // ������ͣ������ж���������㣬�������ݽ��
    uint Count; // һ������ҳ���Ѵ��˶���key
    uint prevPtr; // ǰ�����ݽ��
    uint nextPtr; // ������ݽ��
    uint key[M + 1]; // ���ݶԵ�keyֵ
    Data data[M + 1]; // ���ݶԵ�valueֵ
}DataNode;

typedef struct {
    uint ptr; // ָ�����λ��
    uint index; // �ڽ���еĹؼ������
    bool found;
} Result;
/** �ӿ� **/
void insertData(Data data, uint key);

void selectData(uint key);
void searchBTree(uint node_pos, uint key, Result* result);

void deleteData();