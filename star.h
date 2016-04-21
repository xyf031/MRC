// Version: 2016-03-20-21:05
// By: Xiao Yi Fan

#pragma once
#include <stdio.h>

#define COLUMN_NUM 5
#define HEADER_LENGTH 50
#define STAR_DATA_LINE_LENGTH 200

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

