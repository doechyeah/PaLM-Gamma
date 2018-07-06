/*
 * Parking lot detection image processing code.
 */
#include "ImageAnalyzer.h"

const double PAVEMENT_RATIO_DIFF = 0.01;
const short PAVEMENT_VALUE_DIFF[3] = {15, 15, 15};
const short PAVEMENT_INTENSITY_DIFF = 60;
const short PAVEMENT_INTENSITY_DIFF_NIGHT = 100;

bool loadImage(String imagePath, short ***&pixels, int &rows, int &cols) {
	cout << "Opening image \"" << imagePath << "\".\n";
	Mat image = imread(imagePath);
	if(!image.data) {
		cerr << "ERROR: No image data in image at \"" << imagePath << "\".\n";
		return 0;
	}
	rows = image.rows;
	cols = image.cols;
	flipImage(image, rows, cols, FLIP_AXIS);
	pixels = new short **[rows];
	for(int r = 0; r < rows; r++) {
		pixels[r] = new short*[cols];
		for(int c = 0; c < cols; c++) {
			pixels[r][c] = new short[3];
			for(int i = 0; i < 3; i++) {
				pixels[r][c][i] = (short)image.at<Vec3b>(r, c)[2 - i];
			}
		}
	}
	cout << "Opened image \"" << imagePath << "\".\n";
	return 1;
}

bool closeImage(short ***&pixels, int &rows, int &cols) {
	cout << "Closing image.\n";
	for(int r = 0; r < rows; r++) {
		for(int c = 0; c < cols; c++) {
			delete [] pixels[r][c];
		}
		delete [] pixels[r];
	}
	delete [] pixels;
	pixels = NULL;
	cout << "Closed image.\n";
	return 1;
}

bool updateImage(String imagePath, short ***&pixels, int &rows, int &cols) {
	return closeImage(pixels, rows, cols) && loadImage(imagePath, pixels, rows, cols);
}

/*
 * Axis is 0 for vertical axis, 1 for horizontal axis, and 2 is for both axis.
 */
void flipImage(Mat &image, int rows, int cols, int axis) {
	Mat flippedImage(rows, cols, CV_8UC3, Scalar(0,0,0));
	if(axis == 0) {
		for(int r = rows - 1; r >= 0; r--) {
			for(int c = 0; c < cols; c++) {
				for(int i = 0; i < 3; i++) {
					flippedImage.at<Vec3b>(rows - 1 - r, c).val[i] = image.at<Vec3b>(r, c).val[i];
				}
			}
		}
	}
	else if(axis == 1) {
		for(int r = 0; r < rows; r++) {
			for(int c = cols - 1; c >= 0; c--) {
				for(int i = 0; i < 3; i++) {
					flippedImage.at<Vec3b>(r, cols - 1 - c).val[i] = image.at<Vec3b>(r, c).val[i];
				}
			}
		}
	}
	else {
		for(int r = rows - 1; r >= 0; r--) {
			for(int c = cols - 1; c >= 0; c--) {
				for(int i = 0; i < 3; i++) {
					flippedImage.at<Vec3b>(rows - 1 - r, cols - 1 - c).val[i] = image.at<Vec3b>(r, c).val[i];
				}
			}
		}
	}
	image = flippedImage;
	return;
}

/*
 * Full and empty colour are taken as RGB.
 */
bool createMap(Mat currentMapImg, short ***currentMap, int mapRows, int mapCols, int outStalls[], int stallBounds[][2][2], int numStalls, short fullColour[3], short emptyColour[3]) {
	for(int r = 0; r < mapRows; r++) {
		for(int c = 0; c < mapCols; c++) {
			// Swap RGB of image to BGR
			currentMapImg.at<Vec3b>(r, c).val[2] = (int)currentMap[r][c][0];
			currentMapImg.at<Vec3b>(r, c).val[1] = (int)currentMap[r][c][1];
			currentMapImg.at<Vec3b>(r, c).val[0] = (int)currentMap[r][c][2];
		}
	}
	for(int i = 0; i < numStalls; i++) {
		short stallRGB[3];
		if(outStalls[i] == 1) {
			stallRGB[0] = fullColour[0];
			stallRGB[1] = fullColour[1];
			stallRGB[2] = fullColour[2];
		}
		else if(outStalls[i] == 2) {
			stallRGB[0] = (fullColour[0] + emptyColour[0]) / 2;
			stallRGB[1] = (fullColour[1] + emptyColour[1]) / 2;
			stallRGB[2] = (fullColour[2] + emptyColour[2]) / 2;
		}
		else {
			stallRGB[0] = emptyColour[0];
			stallRGB[1] = emptyColour[1];
			stallRGB[2] = emptyColour[2];
		}
		for(int r = stallBounds[i][0][0]; r < stallBounds[i][1][0]; r++) {
			for(int c = stallBounds[i][0][1]; c < stallBounds[i][1][1]; c++) {
				// Change pixels in stalls to a colour to indicate if they are full.
				currentMapImg.at<Vec3b>(r, c).val[2] = (int)stallRGB[0];
				currentMapImg.at<Vec3b>(r, c).val[1] = (int)stallRGB[1];
				currentMapImg.at<Vec3b>(r, c).val[0] = (int)stallRGB[2];
			}
		}
	}
	return 0;
}

/*
 * Reads the inFile of points up to the MAX_STALLS number and converts into an array of int arrays of length 2 each.
 * The number of points found and is placed in numStalls.
 * Returns 0 if one of the pieces of data in the inFile is not an int. Otherwise 1. Warning if the entire file is not read.
 */
bool readPointsFile(int outPoints[MAX_STALLS][2], int &numStalls, ifstream &inFile) {
	numStalls = 0;
	for(int i = 0; i < MAX_STALLS; i++) {
		int completePoint = 0;
		string input;
		for(int j = 0; j < 2; j++) {
			if(inFile.eof()) {
				break;
			}
			inFile >> input;
			if(input == ";") {
				break;
			}
			stringstream sstream;
			sstream << input;
			if(!(sstream >> outPoints[i][j])) {
				cerr << "ERROR: Bad data in stall points file at line " << i + 1 << ".\n";
				return 0;
			}
			if(j == 1) {
				completePoint = 1;
			}
		}
		if(completePoint) {
			numStalls++;
		}
		if(inFile.eof()) {
			break;
		}
		if(input == ";") {
			break;
		}
	}
	if(!inFile.eof()) {
		cerr << "WARNING: Stall points file has extra data.\n";
	}
	return 1;
}

/*
 * Creates a rectangle for the bounds of the pixels that are being compared for a stall.
 * Center of bounds is at each point in stallPoints and sides of bounds are each 2 * radius unless
 * that would make the bounds go off image in which case the bounds will be cropped.
 */
void createStallBounds(int stallBounds[][2][2], short ***pixels, int rows, int cols, int stallPoints[][2], int numPoints) {
	//int radius = rows / (numPoints * 2);
	if(numPoints != 0) { // Use manual stall selection
		int radius = rows / (3 * numPoints);
		for(int i = 0; i < numPoints; i++) {
			// Rows
			// Start
			if(stallPoints[i][0] - radius < 0) {
				stallBounds[i][0][0] = 0;
			}
			else {
				stallBounds[i][0][0] = stallPoints[i][0] - radius;
			}
			// End
			if(stallPoints[i][0] + radius >= rows) {
				stallBounds[i][1][0] = rows - 1;
			}
			else {
				stallBounds[i][1][0] = stallPoints[i][0] + radius;
			}
			// Columns
			// Start
			if(stallPoints[i][1] - radius < 0) {
				stallBounds[i][0][1] = 0;
			}
			else {
				stallBounds[i][0][1] = stallPoints[i][1] - radius;
			}
			// End
			if(stallPoints[i][1] + radius >= cols) {
				stallBounds[i][1][1] = cols - 1;
			}
			else {
				stallBounds[i][1][1] = stallPoints[i][1] + radius;
			}
		}
	}
	else { // Use automatic stall detection
		/*
		 * EDGE DETECTION
		 */
	}
	return;
}

/*
 * outStalls is the output array of the states of stalls. 1 for full, 0 for empty (, and -1 is for the end of a row of stalls)?.
 * pixels1 is the original image used for calibration.
 * pixels2 is the current image of the parking lot.
 * stallBounds is the input array of the bounds for each stall in the form (startRow, startCol, rows, cols). Should be same length as outStalls.
 */
void checkStalls(int outStalls[], short ***pixels1, int rows1, int cols1, short ***pixels2, int rows2, int cols2, int stallBounds[][2][2], int numStalls) {
	for(int i = 0; i < numStalls; i++) {
		short originalAvg[3] = {};
		short currentAvg[3] = {};
		
		//countEdges();
		
		avgRGB(originalAvg, pixels1, stallBounds[i][0][0], stallBounds[i][0][1], stallBounds[i][1][0], stallBounds[i][1][1]);
		avgRGB(currentAvg, pixels2, stallBounds[i][0][0], stallBounds[i][0][1], stallBounds[i][1][0], stallBounds[i][1][1]);
		cout << "avgRGB for first before: " << originalAvg[0] << ", " << originalAvg[1] << ", " << originalAvg[2] << "\n";
		cout << "avgRGB for first after: " << currentAvg[0] << ", " << currentAvg[1] << ", " << currentAvg[2] << "\n";
		if(!(similarColour(originalAvg, currentAvg, PAVEMENT_RATIO_DIFF, PAVEMENT_VALUE_DIFF)
			&& similarIntensity(originalAvg, currentAvg, PAVEMENT_INTENSITY_DIFF))) {
			outStalls[i] = 1;
			cout << "test stall full\n";
		}
		else if(!(similarColour(originalAvg, currentAvg, PAVEMENT_RATIO_DIFF, PAVEMENT_VALUE_DIFF)
			&& similarIntensity(originalAvg, currentAvg, PAVEMENT_INTENSITY_DIFF_NIGHT))) {
			outStalls[i] = 2;
			cout << "test stall full\n";
		}
		else {
			outStalls[i] = 0;
		}
		
	}
	return;
}

void findEdgePoints(short ***pixels, int rows, int cols, short targetRGB[], short maxIntensityDiff, double maxRGBRatioDiff) {
	for(int r = 0; r < rows; r++) {
		for(int c = 0; c < cols; c++) {
			/*if(similarColour(pixels[leftIndex][0], targetRGB, maxRGBRatioDiff, MAX_RGB_DIFF)
					&& similarIntensity(pixels[leftIndex][0], targetRGB, maxIntensityDiff)) {
				// Full stall
			}*/
		}
	}
	return;
}

void avgRGB(short *avgRGB, short **pixels, int startRow, int endRow) {
	int numSamples = 0;
	int tempAvg[3] = {};
	if(endRow - startRow <= 0) {
		cerr << "ERROR: Averaging 0 or negative pixels.\n";
		return;
	}
	for(int r = startRow; r < endRow; r++) {
			numSamples++;
			tempAvg[0] += (int)pixels[r][0];
			tempAvg[1] += (int)pixels[r][1];
			tempAvg[2] += (int)pixels[r][2];
	}
	avgRGB[0] = tempAvg[0]/numSamples;
	avgRGB[1] = tempAvg[1]/numSamples;
	avgRGB[2] = tempAvg[2]/numSamples;
	return;
}

void avgRGB(short *avgRGB, short ***pixels, int startRow, int startCol, int endRow, int endCol) {
	int numSamples = 0;
	int tempAvg[3] = {};
	if(endRow - startRow <= 0 || endCol - startCol <= 0) {
		cerr << "ERROR: Averaging 0 or negative pixels.\n";
		return;
	}
	for(int r = startRow; r < endRow; r++) {
		for(int c = startCol; c < endCol; c++) {
			numSamples++;
			tempAvg[0] += (int)pixels[r][c][0];
			tempAvg[1] += (int)pixels[r][c][1];
			tempAvg[2] += (int)pixels[r][c][2];
		}
	}
	avgRGB[0] = tempAvg[0]/numSamples;
	avgRGB[1] = tempAvg[1]/numSamples;
	avgRGB[2] = tempAvg[2]/numSamples;
	return;
}

/*
 * Returns 1 if the relative individual RGB values of each pixel (what colour the pixel is) are within given tolerance ratio (percent
 * of error or the amount off as decimal eg. up to 10% off same colour ratio = 0.1)
 * of each other (so we can have a darker red with a lower intensity/brightness and similar relative values of red, green, and blue).
 * Otherwise returns 0.
 */
bool similarColour(short *pixel1, short *pixel2, double maxRGBRatioDiff, const short *maxRGBDiff) {
	if( // Compare differences of RGB values for each pixel to see if they are acceptable distance apart
		abs(pixel1[0] - pixel2[0]) <= maxRGBDiff[0]
		&& abs(pixel1[1] - pixel2[1]) <= maxRGBDiff[1]
		&& abs(pixel1[2] - pixel2[2]) <= maxRGBDiff[2]
	) {
		return 1;
	}
	double redRatio = (double)(max(pixel1[0], pixel2[0]) + 1.0) / (double)(min(pixel1[0], pixel2[0]) + 1.0);
	double greenRatio = (double)(max(pixel1[1], pixel2[1]) + 1.0) / (double)(min(pixel1[1], pixel2[1]) + 1.0);
	double blueRatio = (double)(max(pixel1[2], pixel2[2]) + 1.0) / (double)(min(pixel1[2], pixel2[2]) + 1.0);
	if( // Compare relative ratios of RGB for each pixel to see if they are within maxRGBRatioDiff
		abs(max(redRatio, greenRatio) / min(redRatio, greenRatio) - 1.0) <= maxRGBRatioDiff
		&& abs(max(greenRatio, blueRatio) / min(greenRatio, blueRatio) - 1.0) <= maxRGBRatioDiff
		&& abs(max(blueRatio, redRatio) / min(blueRatio, redRatio) - 1.0) <= maxRGBRatioDiff
	) {
		return 1;
	}
	return 0;
}

/*
 * Returns 1 if sum of RGB values (intensity) in each pixel are within given intensityTolerance. Otherwise returns 0.
 */
bool similarIntensity(short *pixel1, short *pixel2, short maxIntensityDiff) {
	short pixel1Sum = pixel1[0] + pixel1[1] + pixel1[2];
	short pixel2Sum = pixel2[0] + pixel2[1] + pixel2[2];
	if(abs(pixel1Sum - pixel2Sum) / 3 <= maxIntensityDiff) { // Compare brightness of pixels
		return 1;
	}
	return 0;
}
