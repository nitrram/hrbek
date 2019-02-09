#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <tesseract/baseapi.h>

#include <chrono>

using namespace cv;
using namespace std;
using namespace tesseract;

extern map<int,vector<int>> sort_corners(vector<Point2f> &corners);

extern bool check_dist(const map<int,vector<int>> &dists, const Point2f &p, int d);

extern void compute_corners(vector<Point2f> &corners, const Mat &img);

extern vector<vector<Point2f>> generate_quads(map<int, vector<int>> &&corners);

int main(int argc, char *argv[]) {

	if(argc != 2) {
		cerr << "No input file. Bailing out...\n";
		exit(1);
	}

	TessBaseAPI *ocr = new TessBaseAPI();
	if (ocr->Init(NULL, "ces")) {
		cerr << "Could not initialize tesseract.\n";
		exit(2);
	}

	ifstream fs;
	fs.open(argv[1]); //, fstream::in);

	Mat image;
	image = imread(argv[1], CV_8UC1);

	adaptiveThreshold(image, image,255,ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY,75,10);
	bitwise_not(image, image);


	vector<Vec4i> lines;
	vector<Point2f> corners;

	auto begin = chrono::high_resolution_clock::now();
	compute_corners(corners, image);
	auto end = chrono::high_resolution_clock::now();

	auto quads = generate_quads(sort_corners(corners));

	for(const auto &q : quads) {
		Mat im;
		ocr->SetImage(im.data, im.cols, im.rows, 1, im.step);
		auto outText = string(ocr->GetUTF8Text());
	}

	/*
	cout << "quads: " << quads.size() << endl;
	cout << "corners: " << corners.size() << " in " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " [ms]\n";

	for(const auto &c : corners) {
		//cout << "[" << c.x << ", " << c.y << "]\n";
		circle(image, c, 5, Scalar(255,255,255), FILLED, 8,0);
	}
	*/

	if(!image.data) {
		cerr << "Could not read the image\n";
		return -2;
	}

	namedWindow( "Display window", WINDOW_AUTOSIZE );// Create a window for display.
	imshow( "Display window", image );					 // Show our image inside it.

	waitKey(0);

	fs.close();

	api->End();

	delete api;

	return 0;
}
