
#include <stdio.h>
#include <stdlib.h>


#include "read_file.h"

using namespace std;

int main() {

	read_MRC_And_Star("Data/1.mrc", "Data/1.star", true);

	return 0;
}

