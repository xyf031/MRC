
// Version: 2016-12-01-22:00
// By Xiao Yifan

#include <stdio.h>
#include <stdlib.h>

#include "readMrcStar.h"
void mode1 (const char* fileRoot);
void mode2 (const char* fileRoot);
void mode3 (const char* fileRoot);


int main(int argc, char* argv[]) {
	switch (argc) {
		case 1: mode1("."); break;
		case 2: mode1(argv[1]); break;
		case 3: 
			if (*argv[2] == '0') {
				mode3(argv[1]);
			} else {
				mode2(argv[1]);
			}
			break;
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
		read_MRC_And_Star(mrcPath, true, true);
		printf("MRC modified:\t\t\t%s\n", mrcPath);
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
		read_MRC_And_Star(mrcPath, false, true, starPath);
	}
	fclose(fmrc);
	fclose(fstar);
}


void mode3 (const char* fileRoot) {
	char shell[FILE_NAME_LENGTH];
	sprintf(shell, "ls %s/*.mrc > %s/all_mrc.tmp", fileRoot, fileRoot);
	system(shell);

	sprintf(shell, "%s/all_mrc.tmp", fileRoot);
	FILE *f = fopen(shell, "r");
	char mrcPath[FILE_NAME_LENGTH];
	while (fscanf(f, "%s", mrcPath) > 0) {
		read_MRC_And_Star(mrcPath, false, true);
		// printf("MRC modified:\t\t\t%s\n", mrcPath);
	}
	fclose(f);
	sprintf(shell, "rm %s/all_mrc.tmp", fileRoot);
	system(shell);
}