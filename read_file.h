// Version: 2016-05-10-15:50
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

#define BUCKET_NUM 1000
#define CUT_LOW 0
// #define CUT_UP 1
#define CUT_UP 0.99999

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


int modify_mrc(const char* fileNameMrc)
{
	MRC mrcData(fileNameMrc, "rb+");
	int imWidth = mrcData.getNx();
	int imHeight = mrcData.getNy();
	long imSize = imWidth * imHeight;
	int imWordLength = mrcData.getWordLength();
	float imMin = mrcData.getMin();
	float imMax = mrcData.getMax();
	float* imLine = (float*)malloc((size_t)((int)(imWidth * 1.1) * sizeof(float)));
	int lineLen = 0;

	
	// Find Quantiles
	// 第一轮读取，统计各桶像素数
	int bucketCount[BUCKET_NUM];  // 桶的编号从0开始
	memset((void *)bucketCount, 0, sizeof(bucketCount));
	float bucketLen = (imMax - imMin) / BUCKET_NUM;

	int bucketId;
	for (int i = 0; i < imHeight; i++)
	{
		lineLen = mrcData.readLine(imLine, 0, i);  // When (lineLen > imWidth), the RAM will be confused.
		lineLen = lineLen / imWordLength;
		if (lineLen != imWidth) {
			printf("********** MRC data format error! **********\n");
			printf("Line: %d: Pixel-numbers = %d,\timWidth(getNx) = %d\n", i, lineLen, imWidth);
		}

		for (int j = 0; j < lineLen; j++)
		{
			bucketId = (int)((imLine[j] - imMin) / bucketLen);
			if (imLine[j] <= imMin)
			{
				printf("##### MinGet #####\n");
			}
			if (imLine[j] >= imMax)
			{
				printf("##### MaxGet #####\n");
				bucketId = BUCKET_NUM - 1;  // 其他区间都是左闭右开，这里把最后一个桶改成全闭区间
			}
			bucketCount[bucketId] += 1;
		}
	}

	// Search for the BucketCutPosition

	// i-th.leftBound = imMin + i * bucketLen
	// i-th.rightBound = imMin + (i+1) * bucketLen
	// i = 0, 1, ..., (BUCKET_NUM - 1)

	// i-th.leftBound <= i-th.BucketPixelValue < i-th.rightBound
	// ---In other word: i-th.rightBound = i-th.leftBound + bucketLen = (i+1)-th.leftBound

	int cutMinId = BUCKET_NUM, cutMaxId = -1;  // cutMinId.leftBound <= **AllPixel** <= cutMaxId.rightBount
	bool minGot = false;
	long tmpPixelCount = 0;
	for (int i = 0; i < BUCKET_NUM; i++)
	{
		tmpPixelCount += bucketCount[i];
		if (!minGot && imSize * CUT_LOW <= (double)tmpPixelCount)
		{
			// cutMinId = min(cutMinId, i);
			cutMinId = i;
			minGot = true;
		}
		if ((double)tmpPixelCount >= imSize * CUT_UP)
		{
			cutMaxId = i;
			break;
		}
	}

	printf("\nBuckets between the two ***-marked buckets(included) will be saved:\n");
	for (int i = 0; i < BUCKET_NUM; i++)
	{
		if (i == cutMinId or i == cutMaxId)
		{
			printf("********** %d: \t%d\n", i, bucketCount[i]);
		} else {
			printf("%d: \t%d\n", i, bucketCount[i]);
		}
	}
	printf("\n\n");


	// 第二轮读取，计算像素值
	long cutLowerCount = 0, cutUpperCount = 0;
	for (int i = 0; i < imHeight; i++)
	{
		lineLen = mrcData.readLine(imLine, 0, i);  // When (lineLen > imWidth), the RAM will be confused.
		lineLen = lineLen / imWordLength;

		for (int j = 0; j < lineLen; j++)
		{
			bucketId = (int)((imLine[j] - imMin) / bucketLen);
			if (bucketId < cutMinId)
			{
				cutLowerCount += 1;
				imLine[j] = imMin + cutMinId * bucketLen;
			}
			else if (bucketId > cutMaxId)
			{
				cutUpperCount += 1;
				imLine[j] = imMin + (cutMaxId + 1) * bucketLen;
			}
		}
		mrcData.writeLine(imLine, 0, i);
	}
	printf("CutMinId = \t%d\n", cutMinId);
	printf("CutMaxId = \t%d\n", cutMaxId);
	printf("Pixel-Total:\t%ld\n", imSize);
	printf("Pixel-Cut-Lower:\t%ld\n", cutLowerCount);
	printf("Pixel-Cut-Upper:\t%ld\n", cutUpperCount);

	mrcData.m_header.dmin = imMin + cutMinId * bucketLen;
	mrcData.m_header.dmax = imMin + (cutMaxId + 1) * bucketLen;
	mrcData.m_header.dmean = -3.1415;
	mrcData.m_header.nlabels += 1;
	strcpy(mrcData.m_header.label[mrcData.m_header.nlabels - 1], "Modified by XYF.");
	mrcData.updateHeader();
	mrcData.close();

	return 0;
}


int read_MRC_And_Star(const char* fileNameMrc, const char* fileNameStar = NULL, bool modifyMRC = false, FILE* svmFile = NULL) {
	/* Return: -1 -> MRC File Not Exist;  -2 -> MRC readLine() Fails;  -3 -> Star File Not Exist;  -4*/


	/* Read MRC and Print Info */
	MRC mrcData(fileNameMrc, "rb");
	if (mrcData.m_header.nx == -31415)
	{
		// 'nx = -31415' marks whether this .mrc file could be read.
		printf("\n*** File < %s > cannot be read. ***\n", fileNameMrc);
		return -1;
	}

	printf("\n~~~~~~~~~~~~File < %s > has been read:~~~~~~~~~~~~\n", fileNameMrc);
	int imWidth = mrcData.getNx();
	int imHeight = mrcData.getNy();
	long imSize = imWidth * imHeight;
	printf("Pixel numbers (Width * Height): \t%ld \t= %d \t* %d\n", imSize, imWidth, imHeight);

	int imWordLength = mrcData.getWordLength();
	if (imWordLength != 4 || sizeof(float) != 4)
	{
		printf("********** Vital ERROR **********\n");
		printf("The MRC WorldLength:\t%d. The sizeof float is:\t%lu.\n", imWordLength, sizeof(float));
		printf("Both of they must be 4, otherwise the mrc.cpp has to be changed.\n");
		return -2;
	}
	printf("MRC Header:\n-----------------------\n");
	mrcData.printInfo();
	printf("--------------------------\n");


	/* Cut Pixel */
	if (modifyMRC && mrcData.getMean() > 0)
	{
		mrcData.close();
		modify_mrc(fileNameMrc);
		mrcData.open(fileNameMrc, "rb");

		printf("\n************************ After Modify MRC Header:\n");
		mrcData.printInfo();
		printf("************************\n");
	}


	/* Draw BMP */
	BMP* bmpOutput = BMP_Create(imWidth, imHeight, BMP_DEPTH);
	float* imLine = (float*)malloc((size_t)((int)(imWidth * 1.1) * sizeof(float)));
	int lineLen = 0;
	float imMin = mrcData.getMin();
	float imMax = mrcData.getMax();

	int tmpPixel;
	UCHAR tmpGrey;
	for (int i = 0; i < imHeight; i++)
	{
		lineLen = mrcData.readLine(imLine, 0, i);  // When (lineLen > imWidth), the RAM will be confused.
		lineLen = lineLen / imWordLength;

		for (int j = 0; j < lineLen; j++)
		{
			if (imLine[j] <= imMin)
			{
				tmpPixel = 0;
			}
			else if (imLine[j] >= imMax)
			{
				tmpPixel = 255;
			}
			else
			{
				// [1, 254]
				tmpPixel = 1 + (int)(253 * (imLine[j] - imMin) / (imMax - imMin));
			}
			
			tmpGrey = (UCHAR)tmpPixel;
			BMP_SetPixelRGB(bmpOutput, j, i, tmpGrey, tmpGrey, tmpGrey);
		}
	}


	/* Draw Box And Analysis */
	if (fileNameStar != NULL)
	{
		// Read Star File
		int starPointNum = preGetTotalNum(fileNameStar);  // Test .star file and printf the star-point num.
		if (starPointNum < 0)
		{
			return -3;
		}
		MRCStar starData = MRCStar(fileNameStar, starPointNum);

		// Draw Box
		for (int i = 0; i < starData.getTotalNum(); i++)
		{
			drawBox(bmpOutput, starData.getX(i), starData.getY(i), imWidth, imHeight);
			if (svmFile != NULL) fprintfSVM(mrcData, svmFile, starData.getX(i), starData.getY(i), imWidth, imHeight);
		}
	

		// Analysis --- SVM
		if (svmFile != NULL)
		{
			// Rand()
			srand((unsigned int)time(0));
			double negNum1 = rand() % 100;
			double negNum2 = 0.5 + negNum1 / 100.0;  // [0.5, 1.5)
			int negNum = (int)(negNum2 * starData.getTotalNum());

			int negX, negY;
			for (int i = 0; i < negNum; i++)
			{
				negX = rand() % imWidth;
				negY = rand() % imHeight;
				drawBox(bmpOutput, negX, negY, imWidth, imHeight, BOX_SIDE_LENGTH, 255, 0, 0);
				fprintfSVM(mrcData, svmFile, negX, negY, imWidth, imHeight, BOX_SIDE_LENGTH, -1);
			}
		}
	}

	
	/* Save BMP */
	char bmpFileName[FILE_NAME_LENGTH] = "";
	strcat(bmpFileName, fileNameMrc);
	strcat(bmpFileName, ".bmp");
	BMP_WriteFile(bmpOutput, bmpFileName);
	BMP_CHECK_ERROR(stdout, -2);
	BMP_Free(bmpOutput);


	mrcData.close();
	return 0;
}
