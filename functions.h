/** 头文件 **/
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

/** 宏定义 **/
#define M 5
#define INSERT "insert"
#define SELECT "select"
#define DELETE "delete"
#define SHUTDOWN "shutdown"

using namespace std;

/** 关键字重命名 **/
typedef unsigned int uint;
typedef signed int sint;

/** 文件头结构体 **/
typedef struct {
    sint orderNum; // B+树的阶数
    sint nodeSum; // B+树的总结点数量
    uint rootPos; // 根结点所在的数据块的位置，它的值是相对于文件头部的偏移值。
}FileHead;

/** 索引结点结构体 **/
typedef struct {
    sint nodeType; // 结点类型，用于判断是索引结点，还是数据结点
    uint key[M + 1]; // 索引值
    uint ptr[M + 1]; // 子结点位置
}BTNode;

/** 数据结构体 **/
typedef struct {
    int a;
    char c1[128];
    char c2[128];
    char c3[128];
    char c4[80];
}Data;

/** 数据结点结构体 **/
typedef struct {
    sint nodeType; // 结点类型，用于判断是索引结点，还是数据结点
    uint prevPtr; // 前驱数据结点
    uint nextPtr; // 后继数据结点
    uint key[M + 1]; // 数据对的key值
    uint value[M + 1]; // 数据对的value值
}DataNode;

/** 接口 **/
void insertValue();

void selectValue();

void deleteValue();