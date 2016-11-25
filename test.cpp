
#include <iostream>
// #include <stdio.h>
// #include <stdlib.h>

using namespace std;

int main() {
	FILE *f = fopen("all_mrc.txt", "r");
	char a[100];
	fscanf(f, "%s", a);
	cout << a << endl;
	fclose(f);
	return 0;
}

