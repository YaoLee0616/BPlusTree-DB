#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <sstream>
#include <fstream>

extern FILE* fp; // 全局变量 fp

/** 宏定义 **/
#define PAGE_MAX_LENGTH 4*1024 // 页大小设为4KB
#define BT_PAGE_KEY_MAX 500 // 索引页最多有500个key
#define DATA_PAGE_KEY_MAX 7 // 叶子页最多有7个key

#define INSERT "insert"
#define SELECT "select"
#define DELETE "delete"
#define RESET "reset"
#define SHUTDOWN "shutdown"

#define ERROR "输入命令错误，请重新输入" 

#define FILE_NAME "data.ibd"

// 申请并初始化页
#define MALLOC_PAGE(p,type) type* p = (type*)malloc(sizeof(type)); memset(p,0,sizeof(type));

// 释放页
#define FREE_PAGE(p) free(p);

// 以mode形式打开文件
#define OPEN_FILE(file_name,mode) fopen(file_name,mode)

// 以mode形式打开文件,并读出文件头页面
#define OPEN_FILE_READ(file_name,mode,buf,size) fp = OPEN_FILE(file_name,mode); fread(buf,size,1,fp);

// 文件末尾开始写
#define FSEEK_END_WRITE(fp,pos,buf,size,page_count) fseek(fp, page_count*PAGE_MAX_LENGTH, SEEK_SET);pos=ftell(fp); fwrite(buf,size,1,fp);

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
    int order; // B+树的阶数
    uint page_size; // 页大小
    uint page_count; // 已存页的数量
    uint root_page_pos; // b+树根页面所在位置，它的值是相对于文件头部的偏移值。
}FileHead;

/* 索引页结构体 */
typedef struct {
    int page_type; // 页面类型，用于判断是索引页，还是数据页
    int key_count; // 已存key的数量
    uint parent_page_pos; // 父页面所在位置
    uint key[BT_PAGE_KEY_MAX + 1]; // 索引值
    uint child_page_pos[BT_PAGE_KEY_MAX + 1]; // 子页面所在位置
}BTPage;

/* 数据结构体 */
typedef struct {
    char c1[129];
    char c2[129];
    char c3[129];
    char c4[81];
}Data;

/* 数据页结构体 */
typedef struct {
    int page_type; // 页面类型，用于判断是索引页，还是数据页
    int key_count; // 已存key的数量
    bool is_inc_insert; // 是否递增插入
    bool is_dec_insert; // 是否递减插入
    uint parent_page_pos; // 父页面所在位置
    uint prev_page_pos; // 前一个页面所在位置
    uint next_page_pos; // 后一个页面所在位置
    uint key[DATA_PAGE_KEY_MAX + 1]; // 一条数据的key值
    Data data[DATA_PAGE_KEY_MAX + 1]; // 一条数据的data值
}DataPage;

/* 查找结果结构体 */
typedef struct {
    uint pos; // 指向的位置
    int index; // 在页中的关键字序号
    bool is_found; // 是否找到
} Result;


/* 接口 */
bool insertData(Data data, uint key); // 插入单条数据
int loadCsvData(string csv_file_name); // 以文件导入的方式插入大量数据

int searchBTIndex(BTPage* btPageBuf, uint key); // 查找key在索引页中的位置
void searchDataPage(uint page__pos, uint key, Result* result); // 查找数据页
bool selectData(uint key, vector<bool> field); // 主键等值查询数据
void selectRangeData(uint left, uint right, vector<bool> field); // 主键索引范围扫描查询数据
int outputCsvData(uint left, uint right, string csv_file_name); // 主键索引范围扫描的查询导出数据

void deleteData();