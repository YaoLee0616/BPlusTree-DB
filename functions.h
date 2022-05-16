/** ͷ�ļ� **/
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

/** �궨�� **/
#define M 5
#define INSERT "insert"
#define SELECT "select"
#define DELETE "delete"
#define SHUTDOWN "shutdown"

using namespace std;

/** �ؼ��������� **/
typedef unsigned int uint;
typedef signed int sint;

/** �ļ�ͷ�ṹ�� **/
typedef struct {
    sint orderNum; // B+���Ľ���
    sint nodeSum; // B+�����ܽ������
    uint rootPos; // ��������ڵ����ݿ��λ�ã�����ֵ��������ļ�ͷ����ƫ��ֵ��
}FileHead;

/** �������ṹ�� **/
typedef struct {
    sint nodeType; // ������ͣ������ж���������㣬�������ݽ��
    uint key[M + 1]; // ����ֵ
    uint ptr[M + 1]; // �ӽ��λ��
}BTNode;

/** ���ݽṹ�� **/
typedef struct {
    int a;
    char c1[128];
    char c2[128];
    char c3[128];
    char c4[80];
}Data;

/** ���ݽ��ṹ�� **/
typedef struct {
    sint nodeType; // ������ͣ������ж���������㣬�������ݽ��
    uint prevPtr; // ǰ�����ݽ��
    uint nextPtr; // ������ݽ��
    uint key[M + 1]; // ���ݶԵ�keyֵ
    uint value[M + 1]; // ���ݶԵ�valueֵ
}DataNode;

/** �ӿ� **/
void insertValue();

void selectValue();

void deleteValue();