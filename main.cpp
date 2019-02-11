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

	adaptiveThreshold(image, image,255,ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV,75,10);

	vector<Vec4i> lines;
	vector<Point2f> corners;

	auto begin = chrono::high_resolution_clock::now();
	compute_corners(corners, image);
	auto end = chrono::high_resolution_clock::now();

	auto quads = generate_quads(sort_corners(corners));

	for(const auto &q : quads) {		
		Rect quad_crop(q[0].x, q[0].y, abs(q[1].x - q[0].x), abs(q[3].y - q[0].y));

		//TODO: magnify for ocr to provide better output
		Mat crop = image(quad_crop);


		resize(crop, crop, Size(crop.cols * 4, crop.rows * 4));
		adaptiveThreshold(crop, crop, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 75, 10);
		
		
		ocr->SetImage(crop.data, crop.cols, crop.rows, 1, crop.step);
		auto outText = string(ocr->GetUTF8Text());

#ifdef DEBUG
		cout << outText << endl;
#endif
	}

#ifdef DEBUG
	cout << "quads: " << quads.size() << endl;
	cout << "corners: " << corners.size() << " in " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " [ms]\n";

	for(const auto &c : corners) {
		circle(image, c, 5, Scalar(255,255,255), FILLED, 8,0);
	}

	if(!image.data) {
		cerr << "Could not read the image\n";
		exit(3);
	}

	namedWindow( "Display window", WINDOW_AUTOSIZE );// Create a window for display.
	imshow( "Display window", image );					 // Show our image inside it.

	waitKey(0);
#endif

	fs.close();

	ocr->End();

	delete ocr;

	return 0;
}
