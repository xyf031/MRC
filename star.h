// Version: 2016-05-10-15:40
// By: Xiao Yi Fan

#pragma once
#include <stdio.h>

#define COLUMN_NUM 5
#define HEADER_LENGTH 50
#define STAR_DATA_LINE_LENGTH 200

/*
COLUMN_NUM: 			star文件中，每行数据有多少列？一般都是固定的 5行，不需要修改。
HEADER_LENGTH: 			star文件中，数据之前有header信息，用 char[][] 正方形数组存储，预留的边长长度。最短 30 够用
STAR_DATA_LINE_LENGTH: 	star文件中，每行字符用 char[] 存储，预留的长度。最短 66 够用
*/

int preGetTotalNum(const char* filename);


class MRCStar
{
public:
	MRCStar(const char *filename, int totalnum);

	void printInfo();

	int getX(int id);
	int getY(int id);
	int getTotalNum();

private:
	int headerLines;  // Number of lines before the data.
	char header[HEADER_LENGTH][HEADER_LENGTH];  // Mind here! [20][20] may be not enough.

	int totalNum;  // Total lines of data.

	// We use "malloc(sizeof(int) * totalNum)" to initialize them:
	int* x;
	int* y;
	float* psi;
	int* classNum;
	float* merit;
};

