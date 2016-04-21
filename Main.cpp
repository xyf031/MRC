#include <stdio.h>
#include <io.h>
#include <string.h>

#include "read_file.h"


int main0()
{
	printf("\n");
	// http://blog.csdn.net/onlyonename/article/details/8521492
	// http://jacoxu.com/?p=118
	// svm-train [options] training_set_file [model_file]
	// svm-predict [options] test_file model_file output_file
	// svm-train.exe 1.data output.model
	// svm-predict.exe 1.data output.model output.file


	char root[] = "D:\\#CS\\0324SVM\\#\\";
	char mrcRoot[FILE_NAME_LENGTH] = "";
	//strcat(mrcRoot, root);
	//strcat(mrcRoot, "*.mrc");

	char starRoot[FILE_NAME_LENGTH] = "";
	strcat(starRoot, root);
	strcat(starRoot, "*.star");

	char fileNameSvm[FILE_NAME_LENGTH] = "";
	strcat(fileNameSvm, root);
	strcat(fileNameSvm, "#.svm.data");

	FILE* svmFile = fopen(fileNameSvm, "w");

	char fileNameMrc[FILE_NAME_LENGTH] = "";
	char fileNameStar[FILE_NAME_LENGTH] = "";

	_finddata_t starList;
	long tmpResult;

	printf("%s \t %s\n", mrcRoot, starRoot);
	if ((tmpResult = _findfirst(starRoot, &starList)) == -1l)
	{
		printf("No Star file in %s\n", root);
		return 0;
	}
	else {
		do {
			strcpy(fileNameStar, root);
			strcat(fileNameStar, starList.name);			

			strcpy(fileNameMrc, "");
			strncat(fileNameMrc, fileNameStar, strlen(fileNameStar) - strlen("_manual_lgc.star"));
			strcat(fileNameMrc, ".mrc");
			printf(":::::%s \t %s\n", fileNameMrc, fileNameStar);
			
			read_MRC_And_Star(fileNameMrc, fileNameStar, svmFile);

		} while (_findnext(tmpResult, &starList) == 0);
	}

	fclose(svmFile);

	printf("\n\n");
	return 0;
}
