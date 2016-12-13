
// 3.0 -- Almost Perfect!
// Version: 2016-12-06-10:00
// By Xiao Yifan

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "readMrcStar.h"
void mode1 (const char* fileRoot);  // Ignore *.star, just need *.mrc
void mode2 (const char* fileRoot);  // Include *.star into the *.boxes

int boxesForSingleImage(const char* mrcPath, int roiN = 40000, const char* starPath = NULL);


int main(int argc, char* argv[]) {
	printf("./box.out /home/name/mrcfiles       <--- Generate boxes for all *.mrc and make *.boxes\n");
	printf("./box.out /home/name/mrcfiles box   <--- Include *.star into boxes. Need star.txt + mrc.txt\n");
	printf("DO NOT end with / in the file_path (the 2nd parameter)!\n");

	switch (argc) {
		case 1: mode1("."); break;
		case 2: mode1(argv[1]); break;  // ./box.out /home/name/mrcfiles
		case 3: mode2(argv[1]); break;  // ./box.out /home/name/mrcfiles box
		default: mode2(argv[1]); break;
	}
	return 0;
}


void mode1 (const char* fileRoot) {
	// Use ls /home/*.mrc

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
	// Read *.star.
	// Read *.star, include in *.boxes. And convert into *.gtroi
	// Need *.mrc, *.star, mrc.txt, star.txt

	printf("\nMODE 2 begins! Run the shell below FIRST!\n");
	printf("python /home/name/mrcfiles mrc\n\n");

	char mrcTxtPath[FILE_NAME_LENGTH], starTxtPath[FILE_NAME_LENGTH];
	sprintf(mrcTxtPath, "%s/mrc.txt", fileRoot);
	sprintf(starTxtPath, "%s/star.txt", fileRoot);
	// char shell[FILE_NAME_LENGTH];
	// bool isBmp = false;
	// int mrcPathLen = 0;

	FILE *fmrc = fopen(mrcTxtPath, "r");
	FILE *fstar = fopen(starTxtPath, "r");

	// if (fstar == NULL)
	// {
	// 	sprintf(shell, "python analyse_dataset.py %s mrc", fileRoot);
	// 	system(shell);
	// 	// sleep(1);
	// 	fmrc = fopen(mrcTxtPath, "r");
	// 	fstar = fopen(starTxtPath, "r");
	// }
	// if (fmrc == NULL)
	// {
	// 	sprintf(shell, "python analyse_dataset.py %s bmp", fileRoot);
	// 	system(shell);
	// 	// sleep(1);
	// 	sprintf(mrcTxtPath, "%s/bmp.txt", fileRoot);
	// 	isBmp = true;
	// 	fmrc = fopen(mrcTxtPath, "r");
	// 	fstar = fopen(starTxtPath, "r");
	// }

	if (fmrc == NULL || fstar == NULL)
	{
		printf("No mrc.txt or star.txt!\n");
		return;
	}
	char mrcPath[FILE_NAME_LENGTH], starPath[FILE_NAME_LENGTH];
	while (fscanf(fmrc, "%s", mrcPath) > 0) {
		fscanf(fstar, "%s", starPath);
		// if (isBmp)
		// {
		// 	mrcPathLen = strlen(mrcPath);
		// 	mrcPath[mrcPathLen - 3] = 'm';
		// 	mrcPath[mrcPathLen - 2] = 'r';
		// 	mrcPath[mrcPathLen - 1] = 'c';
		// }
		boxesForSingleImage(mrcPath, 40000, starPath);
	}
	fclose(fmrc);
	fclose(fstar);
}


int boxesForSingleImage(const char* mrcPathIn, int roiN, const char* starPath) {
	// int boxesForSingleImage(const char* mrcPath, int roiN = 3000, const char* starPath = NULL);
	// Use sliding-window and random methods to generate rois for a particular image.

	if (mrcPathIn == NULL) return -1;
	char mrcPath[FILE_NAME_LENGTH];
	sprintf(mrcPath, "%s", mrcPathIn);
	
	int *X1 = new int[roiN];
	int *Y1 = new int[roiN];
	char boxPath[FILE_NAME_LENGTH];  // Path of *.boxes, containing the results of sliding window.
	char gtroiPath[FILE_NAME_LENGTH];  // Path of *.gtroi. This is the 4-int version of *.star
	int mrcPathLen = strlen(mrcPath);
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
	mrcPath[mrcPathLen - 4] = '\0';  // Remove ".mrc"

	// Include Ground-Truth boxes.
	if (starPath != NULL)
	{
		int starPointNum = preGetTotalNum(starPath);
		if (starPointNum < 0)
		{
			return -2;
		}
		MRCStar starData = MRCStar(starPath, starPointNum);

		sprintf(gtroiPath, "%s.gtroi", mrcPath);  // Keep *.gtroi filename the same as *.mrc, not *.star
		FILE *fGT = fopen(gtroiPath, "w+");

		int getX, getY;
		for (int i = 0; i < starData.getTotalNum(); i++)
		{
			// if (roiCount >= roiN) break;
			getX = starData.getX(i) - BOX_SIDE_LENGTH/2;
			getY = starData.getY(i) - BOX_SIDE_LENGTH/2;
			X1[roiCount] = min(imWidth - 1 - BOX_SIDE_LENGTH, max(0, getX));  // [0, imWidth-1-BOX_SIDE_LENGTH]
			Y1[roiCount] = min(imHeight - 1 - BOX_SIDE_LENGTH, max(0, getY));  // [0, imHeight-1-BOX_SIDE_LENGTH]
			fprintf(fGT, "%d %d %d %d\r\n", X1[roiCount], Y1[roiCount], 
				X1[roiCount]+BOX_SIDE_LENGTH, Y1[roiCount]+BOX_SIDE_LENGTH);
			++roiCount;

			if (getX > imWidth-1-BOX_SIDE_LENGTH || getX < 0 || getY > imHeight-1-BOX_SIDE_LENGTH || getY < 0)
			{
				printf("@@@@@@@@@@@@@@@@@@@ %s \t:xCenter= %d, yCenter= %d \t@@@\n", 
					starPath, getX+BOX_SIDE_LENGTH/2, getY+BOX_SIDE_LENGTH/2);
			}
		}
		fclose(fGT);
	}

	roiCount = 0;  // --------- Here! ---------- Do NOT include gt-rois.
	srand((unsigned)time(0));
	// Slide begins.
	// int xBegin = rand() % BOX_SIDE_LENGTH;  // The begining of sliding-window is random.
	// int yBegin = rand() % BOX_SIDE_LENGTH;
	int xBegin = 0;
	int yBegin = 0;
	int xStep = BOX_SIDE_LENGTH / 5;  // This can be set to BOX_SIDE_LENGTH/10, BOX_SIDE_LENGTH/2, ...
	int yStep = BOX_SIDE_LENGTH / 5;
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
	// while (roiCount < roiN) {
	// 	xBegin = rand() % (imWidth - BOX_SIDE_LENGTH);
	// 	yBegin = rand() % (imHeight - BOX_SIDE_LENGTH);
	// 	X1[roiCount] = xBegin;
	// 	Y1[roiCount] = yBegin;
	// 	++roiCount;
	// }

	// Write to file.
	sprintf(boxPath, "%s.boxes", mrcPath);
	FILE *fBox = fopen(boxPath, "w+");
	if (fBox == NULL) {
		delete[] X1;
		delete[] Y1;
		return -3;
	}
	for (int i = 0; i < roiCount; ++i)
	{
		fprintf(fBox, "%d %d %d %d\r\n", X1[i], Y1[i], X1[i] + BOX_SIDE_LENGTH, Y1[i] + BOX_SIDE_LENGTH);
	}
	fclose(fBox);

	// Print-to-file: Generated-boxes in star-file type.
	// This is for draw_box() to see what has been generated.
	if (false)
	{
		sprintf(boxPath, "%s.boxstar", mrcPath);
		FILE *fStar = fopen(boxPath, "w+");
		if (fStar == NULL) {
			delete[] X1;
			delete[] Y1;
			return -3;
		}
		for (int i = 0; i < roiCount; ++i)
		{
			fprintf(fStar, "%d %d 31 31 31\r\n", X1[i] + BOX_SIDE_LENGTH/2, Y1[i] + BOX_SIDE_LENGTH/2);
		}
		fclose(fStar);
	}

	delete[] X1;
	delete[] Y1;

	return 0;
}


