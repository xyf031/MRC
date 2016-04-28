// UTF-8

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

#define CUT_LOW 0.00001
#define CUT_UP 0.99999

#include "read_file.h"

using namespace std;

int main()
{
	char fileNameMrc[] = "Data/1.mrc";
	MRC mrcData(fileNameMrc, "rb");
	if (mrcData.m_header.nx == -31415)
	{
		// 'nx = -31415' marks whether this .mrc file has been read.
		return -1;
	}

	printf("File < %s > has been read:\n", fileNameMrc);
	printf("MRC Header:\n-----------------------\n");
	mrcData.printInfo();
	printf("--------------------------\n");

	int imWidth = mrcData.getNx();
	int imHeight = mrcData.getNy();
	long imSize = imWidth * imHeight;
	int imWordLength = mrcData.getWordLength();

	BMP* bmpOutput = BMP_Create(imWidth, imHeight, 24);
	float* imLine = (float*)malloc((size_t)((int)(imWidth * 1.1) * sizeof(float)));  // 临时存储 MRC 的一行像素，预留10%的冗余空间
	int lineLen = 0;  // 
	float imMin = mrcData.getMin();
	float imMax = mrcData.getMax();



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
	int cutMinId = BUCKET_NUM, cutMaxId = 0;  // cutMinId.leftBound <= *** <= cutMaxId.rightBount
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
	int tmpPixel;
	UCHAR tmpGrey;
	long cutLowerCount = 0, cutUpperCount = 0;
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
			if (bucketId < cutMinId)
			{
				cutLowerCount += 1;
				tmpPixel = 0;
			}
			else if (bucketId > cutMaxId)
			{
				cutUpperCount += 1;
				tmpPixel = 255;
			}
			else
			{
				// [1, 254]
				tmpPixel = 1 + (int)(253 * (bucketId - cutMinId) / (cutMaxId - cutMinId));
			}
			
			tmpGrey = (UCHAR)tmpPixel;
			BMP_SetPixelRGB(bmpOutput, j, i, tmpGrey, tmpGrey, tmpGrey);
		}
	}
	printf("CutMinId = \t%d\n", cutMinId);
	printf("CutMaxId = \t%d\n", cutMaxId);
	printf("Pixel-Total:\t%ld\n", imSize);
	printf("Pixel-Cut-Lower:\t%ld\n", cutLowerCount);
	printf("Pixel-Cut-Upper:\t%ld\n", cutUpperCount);

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

