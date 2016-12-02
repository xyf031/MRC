
// Version: 2016-12-01-22:00
// By Xiao Yifan

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "readMrcStar.h"
void mode1 (const char* fileRoot);
void mode2 (const char* fileRoot);
int boxesForSingleImage(const char* mrcPath, int roiN = 3000, const char* starPath = NULL);


int main(int argc, char* argv[]) {
	switch (argc) {
		case 1: mode1("."); break;
		case 2: mode1(argv[1]); break;  // No care about .star files. Use "ls *.mrc"
		case 3: mode2(argv[1]); break;  // Need mrc.txt + star.txt
		default: mode2(argv[1]); break;
	}
	return 0;
}


void mode1 (const char* fileRoot) {
	char shell[FILE_NAME_LENGTH];
	sprintf(shell, "ls %s/*.mrc > %s/all_mrc.tmp", fileRoot, fileRoot);
	system(shell);

	sprintf(shell, "%s/all_mrc.tmp", fileRoot);
	FILE *f = fopen(shell, "r");
	char mrcPath[FILE_NAME_LENGTH];
	while (fscanf(f, "%s", mrcPath) > 0) {
		boxesForSingleImage(mrcPath);
	}
	fclose(f);
	sprintf(shell, "rm %s/all_mrc.tmp", fileRoot);
	system(shell);
}


void mode2 (const char* fileRoot) {
	char mrcTxtPath[FILE_NAME_LENGTH], starTxtPath[FILE_NAME_LENGTH], shell[FILE_NAME_LENGTH];
	bool isBmp = false;
	sprintf(mrcTxtPath, "%s/mrc.txt", fileRoot);
	sprintf(starTxtPath, "%s/star.txt", fileRoot);

	char mrcPath[FILE_NAME_LENGTH], starPath[FILE_NAME_LENGTH];
	int mrcPathLen = 0;
	FILE *fmrc = fopen(mrcTxtPath, "r");
	FILE *fstar = fopen(starTxtPath, "r");

	if (fstar == NULL)
	{
		sprintf(shell, "python analyse_dataset.py %s mrc", fileRoot);
		system(shell);
		// sleep(1);
		FILE *fmrc = fopen(mrcTxtPath, "r");
		FILE *fstar = fopen(starTxtPath, "r");
	}
	if (fmrc == NULL)
	{
		sprintf(shell, "python analyse_dataset.py %s bmp", fileRoot);
		system(shell);
		// sleep(1);
		sprintf(mrcTxtPath, "%s/bmp.txt", fileRoot);
		isBmp = true;
		FILE *fmrc = fopen(mrcTxtPath, "r");
		FILE *fstar = fopen(starTxtPath, "r");
	}

	while (fscanf(fmrc, "%s", mrcPath) > 0) {
		fscanf(fstar, "%s", starPath);
		if (isBmp)
		{
			mrcPathLen = strlen(mrcPath);
			mrcPath[mrcPathLen - 4] = '\0';
		}
		boxesForSingleImage(mrcPath, 3000, starPath);
	}
	fclose(fmrc);
	fclose(fstar);
}


int boxesForSingleImage(const char* mrcPath, int roiN, const char* starPath) {
	// int boxesForSingleImage(const char* mrcPath, int roiN = 3000, const char* starPath = NULL);
	if (mrcPath == NULL) return -1;
	int *X1 = new int[roiN];
	int *Y1 = new int[roiN];
	char boxPath[FILE_NAME_LENGTH];
	int roiCount = 0;

	// Read MRC file, and get imWidth imHeight.
	MRC mrcData(mrcPath, "rb");
	if (mrcData.m_header.nx == -31415 || mrcData.m_header.nx <= 0)
	{
		// 'nx = -31415' marks whether this .mrc file could be read.
		printf("\n*** File < %s > cannot be read. ***\n", mrcPath);
		return -1;
	}
	int imWidth = mrcData.getNx();
	int imHeight = mrcData.getNy();
	mrcData.close();

	// Include Ground-Truth boxes.
	if (starPath != NULL)
	{
		int starPointNum = preGetTotalNum(starPath);
		if (starPointNum < 0)
		{
			return -2;
		}
		MRCStar starData = MRCStar(starPath, starPointNum);

		int getX, getY;
		for (int i = 0; i < starData.getTotalNum(); i++)
		{
			if (roiCount >= roiN) break;
			getX = starData.getX(i) - BOX_SIDE_LENGTH/2;
			getY = starData.getY(i) - BOX_SIDE_LENGTH/2;
			X1[roiCount] = min(imWidth - 1 - BOX_SIDE_LENGTH, max(0, getX));  // [0, imWidth-1-BOX_SIDE_LENGTH]
			Y1[roiCount] = min(imHeight - 1 - BOX_SIDE_LENGTH, max(0, getY));  // [0, imHeight-1-BOX_SIDE_LENGTH]
			++roiCount;

			if (getX > imWidth-1-BOX_SIDE_LENGTH || getX < 0 || getY > imHeight-1-BOX_SIDE_LENGTH || getY < 0)
			{
				printf("@@@@@@@@@@@@@@@@@@@ %s \t:xCenter= %d, yCenter= %d \t@@@@@@@@@@@@@@@@@@@@\n", 
					starPath, getX+BOX_SIDE_LENGTH/2, getY+BOX_SIDE_LENGTH/2);
			}
		}
	}

	srand((unsigned)time(0));
	// Slide begins.
	int xBegin = rand() % BOX_SIDE_LENGTH;
	int yBegin = rand() % BOX_SIDE_LENGTH;
	int xStep = BOX_SIDE_LENGTH;
	int yStep = BOX_SIDE_LENGTH;
	for (int x1 = xBegin; x1 < imWidth - BOX_SIDE_LENGTH; x1 += xStep)
	{
		for (int y1 = yBegin; y1 < imHeight - BOX_SIDE_LENGTH; y1 += yStep)
		{
			if (roiCount >= roiN) break;
			X1[roiCount] = x1;
			Y1[roiCount] = y1;
			++roiCount;
		}
	}

	// Random begins.
	while (roiCount < roiN) {
		xBegin = rand() % (imWidth - BOX_SIDE_LENGTH);
		yBegin = rand() % (imHeight - BOX_SIDE_LENGTH);
		X1[roiCount] = xBegin;
		Y1[roiCount] = yBegin;
		++roiCount;
	}

	// Write to file.
	sprintf(boxPath, "%s.boxes", mrcPath);
	FILE *fBox = fopen(boxPath, "w+");
	if (fBox == NULL) {
		delete[] X1;
		delete[] Y1;
		return -3;
	}
	for (int i = 0; i < roiN; ++i)
	{
		fprintf(fBox, "%d %d %d %d\r\n", X1[i], Y1[i], X1[i] + BOX_SIDE_LENGTH, Y1[i] + BOX_SIDE_LENGTH);
	}
	fclose(fBox);

	if (false)
	{
		sprintf(boxPath, "%s-my.star", mrcPath);
		FILE *fStar = fopen(boxPath, "w+");
		if (fStar == NULL) {
			delete[] X1;
			delete[] Y1;
			return -3;
		}
		for (int i = 0; i < roiN; ++i)
		{
			fprintf(fStar, "%d %d 31 31 31\r\n", X1[i] + BOX_SIDE_LENGTH/2, Y1[i] + BOX_SIDE_LENGTH/2);
		}
		fclose(fStar);
	}

	delete[] X1;
	delete[] Y1;

	return 0;
}


