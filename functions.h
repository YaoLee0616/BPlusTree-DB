#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <sstream>
#include <fstream>
#include <map>

/** 宏定义 **/

// 长度设定
#define PAGE_MAX_LENGTH 4*1024 // 页大小设为4KB
#define BT_PAGE_KEY_MAX 500 // 索引页最多有500个key
#define DATA_PAGE_KEY_MAX 7 // 叶子页最多有7个key
#define BUFFER_POOL_MAX_LENGTH 100 // bufferpool里面最多可以存100个页

// 页面类型
#define BT_PAGE_TYPE 1
#define DATA_PAGE_TYPE 2

// 菜单栏
#define INSERT "insert"
#define SELECT "select"
#define DELETE "delete"
#define RESET ".reset"
#define HELP ".help"
#define SHUTDOWN "shutdown"

// 提示信息
#define ERROR "the input command is wrong, please re-enter it!" 
#define RESET_SUCCESSFUL "RESET SUCCESSFUL!"

// 文件名
#define FILE_NAME "data.ibd"

// 申请并初始化页
#define MALLOC_PAGE(p,type) type* p = (type*)malloc(sizeof(type)); memset(p,0,sizeof(type));

// 释放页
#define FREE_PAGE(p) free(p);

// 以mode形式打开文件
#define OPEN_FILE(file_name,mode) fopen(file_name,mode);

// 文件末尾开始写
#define FSEEK_END_WRITE(fp,buf,size,page_count) fseek(fp, page_count*PAGE_MAX_LENGTH, SEEK_SET); fwrite(buf,size,1,fp);

// 指定位置开始读
#define FSEEK_FIXED_READ(fp,page_id,buf,size) fseek(fp,page_id*PAGE_MAX_LENGTH,SEEK_SET); fread(buf,size,1,fp);  

// 指定位置开始写
#define FSEEK_FIXED_WRITE(fp,page_id,buf,size) fseek(fp,page_id*PAGE_MAX_LENGTH,SEEK_SET); fwrite(buf,size,1,fp); 

// 关闭文件
#define CLOSE_FILE(fp) fclose(fp);

using namespace std;

/* 关键字重命名 */
typedef unsigned int uint;

/* 文件头结构体 */
typedef struct {
    int order; // B+树的阶数
    uint root_page_id;
    uint page_size; // 页大小
    uint page_count; // 已存页的数量
}FileHead;

/* 索引页结构体 */
typedef struct {
    int page_type; // 页面类型, 用于判断是索引页, 还是数据页
    int key_count; // 已存key的数量

    uint page_id; 
    uint parent_page_id;

    uint key[BT_PAGE_KEY_MAX + 1]; // 索引值
    uint child_page_id[BT_PAGE_KEY_MAX + 1]; // 子页面所在位置
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
    int page_type; // 页面类型, 用于判断是索引页, 还是数据页
    int key_count; // 已存key的数量
    bool is_inc_insert; // 是否递增插入
    bool is_dec_insert; // 是否递减插入

    uint page_id;
    uint parent_page_id;
    uint prev_page_id;
    uint next_page_id;

    uint key[DATA_PAGE_KEY_MAX + 1]; // 一条数据的key值
    Data data[DATA_PAGE_KEY_MAX + 1]; // 一条数据的data值
}DataPage;

/* 查找结果结构体 */
typedef struct {
    uint page_id; // 页面所在位置
    int index; // 插入key的位置或查找到key的位置
    bool is_found; // 是否找到
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


/* 全局变量 */
extern FILE* fp; // 全局变量 fp

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


/* 接口 */
bool insertData(Data* data, uint key); // 插入单条数据
int loadCsvData(string csv_file_name); // 以文件导入的方式插入大量数据

int searchBTIndex(BTPage* btPageBuf, uint key); // 查找key在索引页中的位置
void searchDataPage(uint page_id, uint key, Result* result); // 查找数据页
bool selectData(uint key, vector<bool> field); // 主键等值查询数据
void selectRangeData(uint left, uint right, vector<bool> field); // 主键索引范围扫描查询数据
int outputCsvData(uint left, uint right, string csv_file_name); // 主键索引范围扫描的查询导出数据

void deleteData();

void btPageInsertBufferPool(BTPage* btPage); // 将索引页插入BufferPool 
void dataPageInsertBufferPool(DataPage* dataPage); // 将数据页插入BufferPool 

void free(); // 释放BufferPool
void flush(); // 将flush_list刷入磁盘

void btPageInsertFlushPool(BTPage* btPage); // 将索引页插入FlushList 
void dataPageInsertFlushPool(DataPage* dataPage); // 将数据页插入FlushList 