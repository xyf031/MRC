#include <iostream>
#include <fstream>
#include "mrc.h"
#include "star.h"
extern "C" {
#include "qdbmp.h"
}

#include <stdlib.h>
#include <algorithm>
#include <string.h>
#define BUCKET_NUM 100
#define CUT_LOW 0
#define CUT_UP 1

#include "read_file.h"

using namespace std;

int main3()
{
	char fileNameMrc[] = "Data/1.mrc";
	MRC mrcData(fileNameMrc, "rb");
	if (mrcData.m_header.nx == -31415)
	{
		return -1;
	}

	// printf("File < %s > has been read:\n", fileNameMrc);
	// printf("MRC Header:\n-----------------------\n");
	// // mrcData.printInfo();
	// printf("--------------------------\n");

	FILE* f = fopen("Data/svm.ddd", "w+");
	read_MRC_And_Star(fileNameMrc, "Data/1.star", f);

	return 0;
}

int main()
{
	char fileNameMrc[] = "Data/1.mrc";
	MRC mrcData(fileNameMrc, "rb");
	if (mrcData.m_header.nx == -31415)
	{
		// 'nx = -31415' marks whether the .mrc file has been read.
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

	BMP* bmpOutput = BMP_Create(imWidth, imHeight, 24);
	float* imLine = (float*)malloc((size_t)((int)(imWidth * 1.1) * sizeof(float)));  // 临时存储 MRC 的一行像素，预留10%的冗余空间
	int lineLen = 0;  // 
	float imMin = mrcData.getMin();
	float imMax = mrcData.getMax();



	int bucketCount[BUCKET_NUM];
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
	for (int i = 0; i < BUCKET_NUM; i++)
	{
		printf("%d: \t%d\n", i, bucketCount[i]);
	}
	printf("\n\n");

	// Search for the BucketCutPosition
	int cutMinId = BUCKET_NUM, cutMaxId = 0;  // cutMinId.leftBound <= ?? <= cutMaxId.rightBount
	long tmpPixelCount = 0;
	for (int i = 0; i < BUCKET_NUM; i++)
	{
		tmpPixelCount += bucketCount[i];
		if (imSize * CUT_LOW < (double)tmpPixelCount)
		{
			cutMinId = min(cutMinId, i);
		}
		if ((double)tmpPixelCount >= imSize * CUT_UP)
		{
			cutMaxId = i;
			break;
		}
	}





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
			if (imLine[j] < cutMinId * bucketLen + imMin)
			{
				tmpPixel = 0;
			}
			else if (imLine[j] > (cutMaxId + 1) * bucketLen + imMin)
			{
				tmpPixel = 255;
			}
			else
			{
				tmpPixel = (int)((imLine[j] - cutMinId * bucketLen - imMin) / (bucketLen * (cutMaxId + 1 - cutMinId)) * 255);
			}
			//tmpPixel = (int)((imLine[j] - imMin) / (imMax - imMin) * 255);  // ----------
			tmpGrey = (UCHAR)0;
			BMP_SetPixelRGB(bmpOutput, j, i, tmpGrey, tmpGrey, tmpGrey);
		}
	}

	/* Save Result */
	char bmpFileName[100] = "";
	strcat(bmpFileName, fileNameMrc);
	strcat(bmpFileName, ".bmp");
	BMP_WriteFile(bmpOutput, bmpFileName);
	BMP_CHECK_ERROR(stdout, -2);
	BMP_Free(bmpOutput);



	printf("\n\n\n");
	return 0;
}

