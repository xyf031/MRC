// Version: 2016-03-24-15:09
// By Xiao Yifan

#pragma once

#include <iostream>
#include <fstream>
#include <algorithm>
#include <time.h>

#include "mrc.h"
#include "star.h"
extern "C" {
#include "qdbmp.h"
}

#define FILE_NAME_LENGTH 100
#define BMP_DEPTH 24
#define BOX_SIDE_LENGTH 100
#define BUCKET_NUM 100

using namespace std;


void drawBox(BMP* mrc, int center_x, int center_y, int max_x, int max_y, int sideLength = BOX_SIDE_LENGTH, int r = 0, int g = 0, int b = 255)
{
	if (max_x <= 0 || max_y <= 0)
	{
		printf("drawBox Error!---max_x = %d\tmax_y = %d\n", max_x, max_y);
		return;
	}

	// xMin <= pixel-i <= xMax
	int xMin = max(0, center_x - sideLength / 2);
	int xMax = min(max_x - 1, center_x + sideLength / 2);
	// yMin <= pixel-i <= yMax
	int yMin = max(0, center_y - sideLength / 2);
	int yMax = min(max_y - 1, center_y + sideLength / 2);

	for (int i = xMin; i <= xMax; i++)
	{
		BMP_SetPixelRGB(mrc, i, yMin, r, g, b);
		BMP_SetPixelRGB(mrc, i, yMax, r, g, b);
	}
	for (int i = yMin; i <= yMax; i++)
	{
		BMP_SetPixelRGB(mrc, xMin, i, r, g, b);
		BMP_SetPixelRGB(mrc, xMax, i, r, g, b);
	}

}


int copyBoxData()
{

	return 1;
}


int fprintfSVM(MRC m, FILE* f, int center_x, int center_y, int max_x, int max_y, int sideLength = BOX_SIDE_LENGTH, int class_id = 1)
{
	if (max_x <= 0 || max_y <= 0)
	{
		printf("fprintfSVM Error!---max_x = %d\tmax_y = %d\n", max_x, max_y);
		return -1;
	}

	// xMin <= pixel-i <= xMax
	int xMin = max(0, center_x - sideLength / 2);
	int xMax = min(max_x - 1, center_x + sideLength / 2);
	if (xMin == 0)
	{
		xMax = sideLength;
	}
	else if (xMax == max_x - 1){
		xMin = xMax - sideLength;
	}
	int nCol = xMax - xMin + 1;
	if (nCol != sideLength + 1)
	{
		printf("fprintfSVM Error!---xMin = %d\txMax = %d\t\n", xMin, xMax);
		return -2;
	}

	// yMin <= pixel-i <= yMax
	int yMin = max(0, center_y - sideLength / 2);
	int yMax = min(max_y - 1, center_y + sideLength / 2);

	if (yMin == 0)
	{
		yMax = sideLength;
	}
	else if (yMax == max_y - 1){
		yMin = yMax - sideLength;
	}

	if (yMax - yMin != sideLength)
	{
		printf("fprintfSVM Error!---yMin = %d\tyMax = %d\t\n", yMin, yMax);
		return -3;
	}

	int row = 0, col = 0;
	float* tmp = (float*)malloc(sizeof(float));

	fprintf(f, "%d ", class_id);
	for (int i = yMin; i <= yMax; i++)
	{
		col = 0;	
		for (int j = xMin; j <= xMax; j++)
		{
			m.readPixel(tmp, 0, row, col);
			fprintf(f, "%d:%f ", row * nCol + col + 1, *tmp);
			col++;
		}
		row++;
	}
	fprintf(f, "\n");

	return 1;
}


int read_MRC_And_Star(const char* fileNameMrc, const char* fileNameStar, FILE* svmFile) {
	/* Return: -1 -> MRC File Not Exist;  -2 -> MRC readLine() Fails;  -3 -> Star File Not Exist;  -4*/


	/* Read MRC and Print Info */
	MRC mrcData(fileNameMrc, "rb");
	if (mrcData.m_header.nx == -31415)
	{
		return -1;
	}

	printf("File < %s > has been read:\n", fileNameMrc);
	printf("MRC Header:\n-----------------------\n");
	mrcData.printInfo();
	printf("--------------------------\n");

	int imWidth = mrcData.getNx();
	int imHeight = mrcData.getNy();
	int imSize = imWidth * imHeight;
	int imWordLength = mrcData.getWordLength();
	printf("Pixel numbers (Width * Height): \t%d \t= %d \t* %d\n", imSize, imWidth, imHeight);

	if (imWordLength != 4 || sizeof(float) != 4)
	{
		printf("********** Vital ERROR **********\n");
		printf("The MRC WorldLength:\t%d. The sizeof float is:\t%lu.\nBoth of they must be 4, otherwise the mrc.cpp has to be changed.\n", imWordLength, sizeof(float));
		return -2;
	}


	/* Read Star File */
	int starPointNum = preGetTotalNum(fileNameStar);  // Test .star file and printf the star-point num.
	if (starPointNum < 0)
	{
		return -3;
	}
	MRCStar starData = MRCStar(fileNameStar, starPointNum);


	/* Create BMP */
	BMP* bmpOutput = BMP_Create(imWidth, imHeight, BMP_DEPTH);
	float* imLine = (float*)malloc((size_t)((int)(imWidth * 1.1) * sizeof(float)));
	int lineLen = 0;

	//***************************************
	//lineLen = mrcData.readLine(imLine, 0, 2);
	//float *imPexel = (float*)malloc(sizeof(float));
	//for (int i = 0; i < lineLen/imWordLength; i++)
	//{
	//	mrcData.readPixel(imPexel, 0, 2, i);
	//	if (*imPexel != imLine[i])
	//	{
	//		printf("************* i = %d,  imPexel = %f,  imLine = %f\n", i, *imPexel, imLine[i]);
	//	}
	//}
	//************************************

	float imMin = mrcData.getMin();
	float imMax = mrcData.getMax();

	// Find Quantiles
	int bucketCount[BUCKET_NUM];
	memset((void *)bucketCount, 0, sizeof(bucketCount));





	int tmpPixel;
	UCHAR tmpGrey;

	for (int i = 0; i < imHeight; i++)
	{
		lineLen = mrcData.readLine(imLine, 0, i);  // When (lineLen > imWidth), the RAM will be confused.
		lineLen = lineLen / imWordLength;
		if (lineLen != imWidth) {
			printf("********** MRC data format error! Line: %d: Pixel-numbers = %d\timWidth(getNx) = %d\n", i, lineLen, imWidth);
		}

		for (int j = 0; j < lineLen; j++)
		{
			tmpPixel = (int)((imLine[j] - imMin) / (imMax - imMin) * 255);  // ----------
			tmpGrey = (UCHAR)tmpPixel;
			BMP_SetPixelRGB(bmpOutput, j, i, tmpGrey, tmpGrey, tmpGrey);
		}
	}


	/* Draw Box */
	for (int i = 0; i < starData.getTotalNum(); i++)
	{
		drawBox(bmpOutput, starData.getX(i), starData.getY(i), imWidth, imHeight);
		fprintfSVM(mrcData, svmFile, starData.getX(i), starData.getY(i), imWidth, imHeight);
	}


	/* Rand() */
	srand((unsigned int)time(0));
	double negNum1 = rand() % 100;
	double negNum2 = 0.5 + negNum1 / 100.0;
	int negNum = (int)(negNum2 * starData.getTotalNum());

	int negX, negY;
	for (int i = 0; i < negNum; i++)
	{
		negX = rand() % imWidth;
		negY = rand() % imHeight;
		drawBox(bmpOutput, negX, negY, imWidth, imHeight, BOX_SIDE_LENGTH, 255, 0, 0);
		fprintfSVM(mrcData, svmFile, negX, negY, imWidth, imHeight, BOX_SIDE_LENGTH, -1);
	}

	
	/* Save Result */
	char bmpFileName[FILE_NAME_LENGTH] = "";
	strcat(bmpFileName, fileNameMrc);
	strcat(bmpFileName, ".bmp");
	BMP_WriteFile(bmpOutput, bmpFileName);
	BMP_CHECK_ERROR(stdout, -2);
	BMP_Free(bmpOutput);

	
	return 0;
}
