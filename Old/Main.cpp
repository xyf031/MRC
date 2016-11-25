
#include <stdio.h>
#include <stdlib.h>


#include "read_file.h"

using namespace std;

int main() {
	char fileRoot[] = "/Users/XYF/Desktop/Image";
	char shell[FILE_NAME_LENGTH];
	sprintf(shell, "ls %s > Data/ProteinList.tmp", fileRoot);
	system(shell);

	FILE *dirName = fopen("Data/ProteinList.tmp", "r");
	FILE *fname;
	char proteinName[FILE_NAME_LENGTH];
	char mrcName[FILE_NAME_LENGTH];
	char starName[FILE_NAME_LENGTH];
	char a[FILE_NAME_LENGTH];
	char b[FILE_NAME_LENGTH];
	while(fscanf(dirName, "%s", proteinName) > 0) {
		printf("\n--- %s/%s ---\n", fileRoot, proteinName);
		sprintf(shell, "ls %s/%s > Data/FileList.tmp", fileRoot, proteinName);
		system(shell);

		fname = fopen("Data/FileList.tmp", "r");
		while(fscanf(fname, "%s", mrcName) > 0) {
			fscanf(fname, "%s", starName);
			sprintf(a, "%s/%s/%s", fileRoot, proteinName, mrcName);
			sprintf(b, "%s/%s/%s", fileRoot, proteinName, starName);
			printf("%s\n", b);
			read_MRC_And_Star(a, b, true);
		}
		fclose(fname);
	}
	fclose(dirName);

	return 0;
}

