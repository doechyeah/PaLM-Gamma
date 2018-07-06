/*
 * Parking lot detection image processing code.
 */
#ifndef ImageAnalyzer_H
#define ImageAnalyzer_H

#define MAX_STALLS 50
#define FLIP_AXIS 2

#include <cv.h>
#include <opencv2/opencv.hpp>
#include <highgui.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>

using namespace std;
using namespace cv;

bool loadImage(String imagePath, short ***&pixels, int &rows, int &cols);
bool closeImage(short ***&pixels, int &rows, int &cols);
bool updateImage(String imagePath, short ***&pixels, int &rows, int &cols);
void flipImage(Mat &image, int rows, int cols, int axis);
bool createMap(Mat currentMapImg, short ***currentMap, int mapRows, int mapCols, int outStalls[], int stallPoints[][2][2], int numStalls, short fullColour[3], short emptyColour[3]);
bool readPointsFile(int outPoints[MAX_STALLS][2], int &numStalls, ifstream &inFile);
void createStallBounds(int stallBounds[][2][2], short ***pixels, int rows, int cols, int stallPoints[][2], int numPoints);
void checkStalls(int outStalls[], short ***pixels1, int rows1, int cols1, short ***pixels2, int rows2, int cols2, int stallBounds[][2][2], int numStalls);
double findAngle();
void findEdgePoints(int **edges, int numEdges, short ***pixels, int rows, int cols, short targetRGB[], short maxIntensityDiff, double maxRGBRatioDiff);
void findStallPixels(short ***stallPixels, short ***pixels, int rows, int cols, short targetRGB[], short maxIntensityDiff, double maxRGBRatioDiff);
void avgRGB(short *avgRGB, short **pixels, int startRow, int endRow);
void avgRGB(short *avgRGB, short ***pixels, int startRow, int startCol, int endRow, int endCol);
bool similarColour(short *pixel1, short *pixel2, double maxRGBRatioDiff, const short *maxRGBDiff);
bool similarIntensity(short *pixel1, short *pixel2, short maxIntensityDiff);

#endif
