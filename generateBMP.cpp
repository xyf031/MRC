
// 3.0 -- Almost Perfect!
// Version: 2016-12-05-22:00
// By Xiao Yifan

#include <stdio.h>
#include <stdlib.h>

#include "readMrcStar.h"
void mode1 (const char* fileRoot);  // Modify *.mrc files, and get *.bmp after it.
void mode2 (const char* fileRoot);  // Non-modify. Get *.bmp with gt-boxes.
void mode3 (const char* fileRoot);  // Non-modify. Get *.bmp from origional mrc.


int main(int argc, char* argv[]) {
	printf("./bmp.out /home/name/mrcfiles       <--- Modify all *.mrc and draw *.bmp\n");
	printf("./bmp.out /home/name/mrcfiles 0     <--- Draw *.bmp of all *.mrc, but no-modify.\n");
	printf("./bmp.out /home/name/mrcfiles box   <--- Draw boxes to *.bmp. Need star.txt + mrc.txt\n");
	printf("DO NOT end with / in the file_path!\n");

	switch (argc) {
		case 1: mode1("."); break;
		case 2: mode1(argv[1]); break;  // Only 1 para, modify *.mrc and generate *.bmp

		case 3: 
			if (*argv[2] == '0') {  // The second para begins with '0', generate *.bmp without modify.
				mode3(argv[1]);
			} else {
				mode2(argv[1]);  // The second para can be 'box', draw boxes to *.bmp. There must be star.txt.
			}
			break;
		default: mode2(argv[1]); break;  // No less than 3 paras, draw boxes to *.bmp. 
	}
	return 0;
}


void mode1 (const char* fileRoot) {
	// ls ROOT/*.mrc and modify all of them. Generate *.bmp

	char shell[FILE_NAME_LENGTH];
	sprintf(shell, "ls %s/*.mrc > %s/all_mrc.tmp", fileRoot, fileRoot);
	system(shell);

	sprintf(shell, "%s/all_mrc.tmp", fileRoot);
	FILE *f = fopen(shell, "r");

	char mrcPath[FILE_NAME_LENGTH];
	while (fscanf(f, "%s", mrcPath) > 0) {
		// int read_MRC_And_Star(fileNameMrc, bool modifyMRC = false, bool getBmp = false, fileNameStar = NULL)
		read_MRC_And_Star(mrcPath, true, true);
	}
	fclose(f);

	sprintf(shell, "rm %s/all_mrc.tmp", fileRoot);
	system(shell);
}


void mode2 (const char* fileRoot) {
	// Draw boxes.
	// You can use mrc.txt + star.txt to draw on origional image.
	// Or you can use mrctmp.txt + star.txt to draw on modified images.

	printf("MODE 2 begins! Run the shell below FIRST!\n");
	printf("python /home/name/mrcfiles mrc\n");
	printf("OR python /home/name/mrcfiles mrctmp\n");

	char mrcTxtPath[FILE_NAME_LENGTH], starTxtPath[FILE_NAME_LENGTH];
	sprintf(mrcTxtPath, "%s/mrc.txt", fileRoot);
	sprintf(starTxtPath, "%s/star.txt", fileRoot);
	FILE *fmrc = fopen(mrcTxtPath, "r");
	FILE *fstar = fopen(starTxtPath, "r");

	if (fstar == NULL)
	{
		printf("No star.txt!\n");
		return;
	}
	if (fmrc == NULL)
	{
		sprintf(mrcTxtPath, "%s/mrctmp.txt", fileRoot);
		fmrc = fopen(mrcTxtPath, "r");
	}
	if (fmrc == NULL) {
		printf("No mrc.txt or mrctmp.txt!\n");
		return;
	}

	char mrcPath[FILE_NAME_LENGTH], starPath[FILE_NAME_LENGTH];
	while (fscanf(fmrc, "%s", mrcPath) > 0) {
		fscanf(fstar, "%s", starPath);
		// int read_MRC_And_Star(fileNameMrc, bool modifyMRC = false, bool getBmp = false, fileNameStar = NULL)
		read_MRC_And_Star(mrcPath, false, true, starPath);  // Here!
	}
	fclose(fmrc);
	fclose(fstar);
}


void mode3 (const char* fileRoot) {
	// Draw *.bmp for origional image.

	char shell[FILE_NAME_LENGTH];
	sprintf(shell, "ls %s/*.mrc > %s/all_mrc.tmp", fileRoot, fileRoot);
	system(shell);

	sprintf(shell, "%s/all_mrc.tmp", fileRoot);
	FILE *f = fopen(shell, "r");

	char mrcPath[FILE_NAME_LENGTH];
	while (fscanf(f, "%s", mrcPath) > 0) {
		// int read_MRC_And_Star(fileNameMrc, bool modifyMRC = false, bool getBmp = false, fileNameStar = NULL)
		read_MRC_And_Star(mrcPath, false, true);
	}
	fclose(f);

	sprintf(shell, "rm %s/all_mrc.tmp", fileRoot);
	system(shell);
}
