/*
 * Parking lot detection image processing code.
 */
#include "ImageAnalyzer.h"
#include <chrono>
#include <thread>

static void onMouseClick( int event, int x, int y, int flags, void* param );
void addPoint(ofstream &outFile, int row, int col);

// Global Variables
ofstream stallWriteFile = {};
int stallPoints[MAX_STALLS][2] = {};
int calibrationPoints[MAX_STALLS][2] = {};

int main() {
	// Original Image
	string originalImagePath;
	int originalImageRows = 0;
	int originalImageCols = 0;
	short ***originalImagePixels = NULL;
	
	// Current Image
	string currentImagePath;
	int currentImageRows = 0;
	int currentImageCols = 0;
	short ***currentImagePixels = NULL;
	
	// Centers and bounds of every stall
	string stallPointsPath;
	string stallPointsWritePath;
	string calibrationPointsPath;
	int numStalls = 0;
	int stallBounds[MAX_STALLS][2][2] = {};
	ifstream stallPointsInFile;
	ifstream calibrationPointsInFile;
	
	// Prompt for image file name
	cout << "Please enter path of original image to open: ";
	getline(cin, originalImagePath); // Original image for comparison
	if(!loadImage(originalImagePath, originalImagePixels, originalImageRows, originalImageCols)) {
		return -1;
	}
	cout << "Please enter path of current image to open: ";
	getline(cin, currentImagePath); // Current image taken every cycle
	
	// Prompt for file name of points list
	cout << "Enter name of points file: ";
	getline(cin, stallPointsPath);
	stallPointsInFile.open(stallPointsPath);
	
	// Create window for user input
	namedWindow("Input Window", CV_WINDOW_NORMAL);
	resizeWindow("Input Window", 800, 450);
	// Prompt for creating "patches" for stall colour when lot is not empty (not advised) ///////////////////////////////////
	cout << "Press any key to create and use road colour for stall comparison, waiting 3 seconds.\n";
	if(cvWaitKey(3000) != -1) {		
		// Open image for user input
		Mat guiImage = imread(originalImagePath);
		if(!guiImage.data) {
			cerr << "ERROR: No image data in original image at \"" << originalImagePath << "\".\n";
			return 0;
		}
		flipImage(guiImage, guiImage.rows, guiImage.cols, FLIP_AXIS);
		
		// Create file and place user selected points in file
		stallWriteFile.open(stallPointsPath);
		if(stallWriteFile.fail()) {
			cerr << "ERROR: Unable to create points file.\n";
			return -1;
		}
		imshow("Input Window", guiImage);
		// Prompt for file name of points list
		setMouseCallback("Input Window", onMouseClick, 0);
		cvWaitKey(0);
		
		stallWriteFile << ';';
		stallWriteFile.close();
	}
	/*
	// Prompt for file name of points list
	cout << "Enter name of points file: ";
	getline(cin, calibrationPointsPath);
	calibrationPointsFile.open(calibrationPointsPath);
	
	// Create window for user input
	namedWindow("Input Window", CV_WINDOW_NORMAL);
	resizeWindow("Input Window", 800, 450);
	// Prompt for creating points file input
	cout << "Press any key to create points, waiting 3 seconds.\n";
	if(cvWaitKey(3000) != -1) {		
		// Open image for user input
		Mat guiImage = imread(originalImagePath);
		if(!guiImage.data) {
			cerr << "ERROR: No image data in original image at \"" << originalImagePath << "\".\n";
			return 0;
		}
		flipImage(guiImage, guiImage.rows, guiImage.cols, FLIP_AXIS);
		
		// Create file and place user selected points in file
		calibrationPointsFile.open(calibrationPointsPath);
		if(calibrationPointsFile.fail()) {
			cerr << "ERROR: Unable to create points file.\n";
			return -1;
		}
		imshow("Input Window", guiImage);
		// Prompt for file name of points list
		setMouseCallback("Input Window", onMouseClick, 0);
		cvWaitKey(0);
		
		calibrationPointsFile << ';';
		calibrationPointsFile.close();
	}*/
	destroyAllWindows();
	///////////////////////////////////
	if(!readPointsFile(stallPoints, numStalls, stallPointsInFile)) {
		stallPointsInFile.close();
		return -2;
	};
	stallPointsInFile.close();
	/*if(!readPointsFile(calibrationPoints, numStalls, stallPointsFile)) {
		stallPointsFile.close();
		return -2;
	};
	stallPointsInFile.close();*/
	///////////////////////////////////
	
	// Find stall bounds
	createStallBounds(stallBounds, originalImagePixels, originalImageRows, originalImageCols, stallPoints, numStalls);
	
	// Window for displaying map
	namedWindow("Parking Lot", CV_WINDOW_NORMAL);
	resizeWindow("Parking Lot", 800, 450);
	
	//int now = std::time_t;
	int sleepTime = 2000;
	bool finished = false;
	int count = 0;
	while(!finished && count < 600) {
		cout << "loops: " << count << "\n";
		if(cvWaitKey(sleepTime / 2) != -1) {
			break;
		}
		//this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
		
		count++;
		if(!loadImage(currentImagePath, currentImagePixels, currentImageRows, currentImageCols)) {
			cout << currentImagePath;
			continue;
			//return -1;
		};
		
		int outStalls[MAX_STALLS] = {};
		checkStalls(outStalls, originalImagePixels, originalImageRows, originalImageCols,
			currentImagePixels, currentImageRows, currentImageCols, stallBounds, numStalls);
		
		cout << "Spots full at index: ";
		int fullStallCount = 0;
		for(int i = 0; i < numStalls; i++) {
			if(outStalls[i]) {
				fullStallCount++;
				cout << i << " ";
			}
		}
		cout << "\nTotal full spots: " << fullStallCount << "\n";
		
		////////////////////////////
		// Display new Map
		try {
		int mapRows = currentImageRows;
		int mapCols = currentImageCols;
		short ***currentMap = currentImagePixels;
		Mat currentMapImg(mapRows, mapCols, CV_8UC3, Scalar(0,0,0));
		short fullColour[3] = {0, 255, 0};
		short emptyColour[3] = {255, 0, 0};
		createMap(currentMapImg, currentMap, mapRows, mapCols, outStalls, stallBounds, numStalls, fullColour, emptyColour);
		//closeImage(currentMap, mapRows, mapCols);
		currentMap = NULL;
		
		imshow("Parking Lot", currentMapImg);
		if(cvWaitKey(sleepTime / 2) != -1) {
			break;
		}
		////////////////////////////
		} catch (const std::exception& e) {
			cerr << "ERROR: Displaying map failed.\n";
			break;
		}
		if(currentImagePixels != NULL) {
			closeImage(currentImagePixels, currentImageRows, currentImageCols);
		}
	}
	
	destroyAllWindows();
	// End program and free memory. Current image will already be deleted.
	if(currentImagePixels != NULL) {
		closeImage(currentImagePixels, currentImageRows, currentImageCols);
	}
	closeImage(originalImagePixels, originalImageRows, originalImageCols);
	return 0;
}

static void onMouseClick(int event, int x, int y, int flags, void* param) {
	if(event == EVENT_LBUTTONDOWN) {
		addPoint(stallWriteFile, y, x);
	}
}

void addPoint(ofstream &outFile, int row, int col) {
	cout << "Added point at " << row << ", " << col << "\n";
	outFile << row << " " << col << "\n";
}
