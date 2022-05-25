#define _CRT_SECURE_NO_WARNINGS

#include<iostream>
#include <string>
#include <cstring>
#include <vector>
#include<sstream>
#include<fstream>

extern FILE* fp; // 全局变量 fp

/** 宏定义 **/
#define PAGE_MAX_LENGTH 4*1024 // 页大小设为4KB
#define BT_NODE_MAX 500 // 索引节点最多有500个key
#define DATA_NODE_MAX 7 // 叶子节点最多有7个key

#define INSERT "insert"
#define SELECT "select"
#define DELETE "delete"
#define RESET "reset"
#define SHUTDOWN "shutdown"

#define ERROR "输入命令错误，请重新输入" 

#define FILE_NAME "data"

// 初始化结点
#define MALLOC_NODE(p,type) type* p = (type*)malloc(sizeof(type)); memset(p,0,sizeof(type));

// 释放结点
#define FREE_NODE(p) free(p);

// 以mode形式打开文件
#define OPEN_FILE(file_name,mode) fopen(file_name,mode)

// 以mode形式打开文件,并读出文件头结点
#define OPEN_FILE_READ(file_name,mode,buf,size) fp = OPEN_FILE(file_name,mode); fread(buf,size,1,fp);

// 文件末尾开始写
#define FSEEK_END_WRITE(fp,pos,buf,size,pageCount) fseek(fp, pageCount*PAGE_MAX_LENGTH, SEEK_SET);pos=ftell(fp); fwrite(buf,size,1,fp);

// 指定位置开始读
#define FSEEK_FIXED_READ(fp,pos,buf,size) fseek(fp,pos,SEEK_SET); fread(buf,size,1,fp);

// 指定位置开始写
#define FSEEK_FIXED_WRITE(fp,pos,buf,size) fseek(fp,pos,SEEK_SET); fwrite(buf,size,1,fp);

// 关闭文件
#define CLOSE_FILE(fp) fclose(fp)

using namespace std;

/* 关键字重命名 */
typedef unsigned int uint;

/* 文件头结构体 */
typedef struct {
    int orderNum; // B+树的阶数
    uint pageSize; // 页大小
    uint pageCount; // B+树的总结点数量
    uint rootPos; // 根结点所在的数据块的位置，它的值是相对于文件头部的偏移值。
}FileHead;

/* 索引结点结构体 */
typedef struct {
    int nodeType; // 结点类型，用于判断是索引结点，还是数据结点
    int Count; // 一个索引页里已存了多少key
    uint parent; // 父结点
    uint key[BT_NODE_MAX + 1]; // 索引值
    uint ptr[BT_NODE_MAX + 1]; // 子结点位置
}BTNode;

/* 数据结构体 */
typedef struct {
    char c1[129];
    char c2[129];
    char c3[129];
    char c4[81];
}Data;

/* 数据结点结构体 */
typedef struct {
    int nodeType; // 结点类型，用于判断是索引结点，还是数据结点
    int Count; // 一个数据页里已存了多少key
    bool Is_inc_insert; // 是否递增插入
    bool Is_dec_insert; // 是否递减插入
    uint parent; // 父结点
    uint prevPtr; // 前驱数据结点
    uint nextPtr; // 后继数据结点
    uint key[DATA_NODE_MAX + 1]; // 数据对的key值
    Data data[DATA_NODE_MAX + 1]; // 数据对的data值
}DataNode;

/* 查找结果结构体 */
typedef struct {
    uint ptr; // 指向结点的位置
    int index; // 在结点中的关键字序号
    bool found; // 是否找到
} Result;

/* 接口 */
bool insertData(Data data, uint key); // 插入单条数据
int loadCsvData(string csv_file_name); // 以文件导入的方式插入大量数据

int searchBTIndex(BTNode* btNodeBuf, uint key); // 查找key在索引结点中的位置
void searchDataNode(uint node_pos, uint key, Result* result); // 查找数据结点
bool selectData(uint key, vector<bool> field); // 主键等值查询数据
void selectRangeData(uint left, uint right, vector<bool> field); // 主键索引范围扫描查询数据
int outputCsvData(uint left,uint right,string csv_file_name); // 主键索引范围扫描的查询导出数据

void deleteData();