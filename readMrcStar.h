
// Version: 2016-11-24-21:00
// By Xiao Yifan

#pragma once

#include <iostream>
#include <fstream>
#include <algorithm>
#include <time.h>

#include "mrc.h"
#include "star.h"
extern "C" {
#include "qdbmp.h"
}

#define FILE_NAME_LENGTH 300
#define BMP_DEPTH 24
#define BOX_SIDE_LENGTH 100

#define BUCKET_NUM 1024
#define CUT_LOW 0
#define CUT_UP 0.99999

using namespace std;

int read_MRC_And_Star(const char* fileNameMrc, bool modifyMRC = false, bool getBmp = false, const char* fileNameStar = NULL);

int modify_mrc_cut(const char* fileNameMrc);
int modify_mrc_histeq(const char* fileNameMrc);

void drawBox(BMP* mrc, int center_x, int center_y, int max_x, int max_y, int sideLength = BOX_SIDE_LENGTH, int r = 0, int g = 0, int b = 255);

