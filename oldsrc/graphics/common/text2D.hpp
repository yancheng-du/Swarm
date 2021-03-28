#ifndef TEXT2D_HPP
#define TEXT2D_HPP

#include <stdio.h>
#include <stdlib.h>
using namespace std;

void initText2D(string texturePath, string mdir, float x_sc, float y_sc);
void printText2D(const char * text, float x, float y, float size);
void cleanupText2D();

#endif
