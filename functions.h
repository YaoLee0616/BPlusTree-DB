#define _CRT_SECURE_NO_WARNINGS
/** 头文件 **/
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

extern FILE* fp;

/** 宏定义 **/
#define PAGE_MAX_LENGTH 4*1024 // 页大小设为4KB
#define M 5
#define INSERT "insert"
#define SELECT "select"
#define DELETE "delete"
#define SHUTDOWN "shutdown"

#define FILE_NAME "data"

#define MALLOC_NODE(p,type) type* p = (type*)malloc(sizeof(type)); memset(p,0,sizeof(type));

#define FREE_NODE(p) free(p);
// 以mode形式打开文件
#define OPEN_FILE(file_name,mode) fopen(file_name,mode)

#define OPEN_FILE_READ(file_name,mode,buf,size) fp = OPEN_FILE(file_name,mode); fread(buf,size,1,fp);
// 文件末尾开始写
#define FSEEK_END_WRITE(fp,pageCount,pageSize,pos,buf,size) fseek(fp, pageCount*pageSize, SEEK_SET); pageCount++;pos=ftell(fp); fwrite(buf,size,1,fp);

// 文件头开始写
#define FSEEK_HED_WRITE(fp,pos,buf,size) fseek(fp, 0, SEEK_SET); pos=ftell(fp); fwrite(buf,size,1,fp);

#define FSEEK_FIXED_READ(fp,pos,buf,size) fseek(fp,pos,SEEK_SET); fread(buf,size,1,fp);

#define FSEEK_FIXED_WRITE(fp,pos,buf,size) fseek(fp,pos,SEEK_SET); fwrite(buf,size,1,fp);
// 关闭文件
#define CLOSE_FILE(fp) fclose(fp)
#define UINT_MAX_VALUE 4294967295
using namespace std;

/** 关键字重命名 **/
typedef unsigned int uint;

/** 文件头结构体 **/
typedef struct {
    uint pageSize; // 页大小
    // sint keySize; // 索引大小
    // sint valueSize; // 索引大小
    uint orderNum; // B+树的阶数
    uint pageCount; // B+树的总结点数量
    uint rootPos; // 根结点所在的数据块的位置，它的值是相对于文件头部的偏移值。
}FileHead;

/** 索引结点结构体 **/
typedef struct {
    uint nodeType; // 结点类型，用于判断是索引结点，还是数据结点
    uint Count; // 一个索引页里已存了多少key
    uint key[M + 1]; // 索引值
    uint ptr[M + 1]; // 子结点位置
}BTNode;

/** 数据结构体 **/
typedef struct {
    char c1[128];
    char c2[128];
    char c3[128];
    char c4[80];
}Data;

/** 数据结点结构体 **/
typedef struct {
    uint nodeType; // 结点类型，用于判断是索引结点，还是数据结点
    uint Count; // 一个数据页里已存了多少key
    uint prevPtr; // 前驱数据结点
    uint nextPtr; // 后继数据结点
    uint key[M + 1]; // 数据对的key值
    Data data[M + 1]; // 数据对的value值
}DataNode;

typedef struct {
    uint ptr; // 指向结点的位置
    uint index; // 在结点中的关键字序号
    bool found;
} Result;
/** 接口 **/
void insertData(Data data, uint key);

void selectData(uint key);
void searchBTree(uint node_pos, uint key, Result* result);

void deleteData();