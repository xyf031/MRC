
// Version: 2016-12-06-10:00
// By: Xiao Yi Fan

#include "star.h"


int preGetTotalNum(const char* filename)
{
	FILE* starFile = fopen(filename, "r");
	if (starFile == NULL)
	{
		printf("********** ERROR **********\nStar File <%s> can not open.\n", filename);
		return -1;
	}

	char tmpChar[STAR_DATA_LINE_LENGTH];
	int tmpCount = 0;
	float tmpNum = 0;
	while (!feof(starFile))
	{
		fscanf(starFile, "%s", tmpChar);
		if (sscanf(tmpChar, "%f", &tmpNum) != 0)
		{
			tmpCount += 1;
		}
	}

	fclose(starFile);

	tmpCount = tmpCount / COLUMN_NUM;
	printf("%s \tStar Point Total Number is: %d\n", filename, tmpCount);
	return tmpCount;
}


MRCStar::MRCStar(const char *filename, int totalnum)
{
	totalNum = totalnum;
	x = (int*)malloc(sizeof(int) * totalNum);
	y = (int*)malloc(sizeof(int) * totalNum);
	psi = (float*)malloc(sizeof(float) * totalNum);
	classNum = (int*)malloc(sizeof(int)* totalNum);
	merit = (float*)malloc(sizeof(float)* totalNum);

	FILE* starFile = fopen(filename, "r");
	char tmpChar[STAR_DATA_LINE_LENGTH];
	float tmpNum = 0;

	int headerCount = 0;
	int dataCount = 0;
	int colCount = 0;

	while (!feof(starFile))
	{
		fscanf(starFile, "%s", tmpChar);
		if (sscanf(tmpChar, "%f", &tmpNum) != 0)
		{
			switch (colCount)
			{
			case 0:
				x[dataCount] = (int)tmpNum;
				break;
			case 1:
				y[dataCount] = (int)tmpNum;
				break;
			case 2:
				psi[dataCount] = tmpNum;
				break;
			case 3:
				classNum[dataCount] = (int)tmpNum;
				break;
			case 4:
				merit[dataCount] = tmpNum;
				break;
			default:
				break;
			}
			colCount++;
			if (colCount == COLUMN_NUM)
			{
				colCount = 0;
				dataCount++;
			}
		}
		else {
			strcpy(header[headerCount ++], tmpChar);
		}
	}
	fclose(starFile);

	headerLines = headerCount;
}


void MRCStar::printInfo()
{
	printf("Star Header:\n-----------------------\n");
	for (int i = 0; i < headerLines; i++)
	{
		printf("\t%s\n", header[i]);
	}
	printf("--------------------------\n");
}


int MRCStar::getX(int id)
{
	if (id >= totalNum)
	{
		printf("ERROR---getX\tid(%d) >= totalNum(%d)\n", id, totalNum);
		return -1;
	}
	return x[id];
}


int MRCStar::getY(int id)
{
	if (id >= totalNum)
	{
		printf("ERROR---getY\tid(%d) >= totalNum(%d)\n", id, totalNum);
		return -1;
	}
	return y[id];
}


int MRCStar::getTotalNum()
{
	return totalNum;
}
