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

extern int get_day_of_week(const std::string &hypothesis);

extern std::string smooth_text(const std::string &hypothesis);

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

	Mat orig;
	Mat image;
	orig = imread(argv[1], CV_8UC1);
	
	adaptiveThreshold(orig, image,255,ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV,75,10);

	vector<Vec4i> lines;
	vector<Point2f> corners;

	auto begin = chrono::high_resolution_clock::now();
	compute_corners(corners, image);
	auto end = chrono::high_resolution_clock::now();

	auto quads = generate_quads(sort_corners(corners));

	int day_of_week = -1;
	vector<vector<string>> list(5);
	for(const auto &q : quads) {		
		Rect quad_crop(q[0].x + 2, q[0].y - 2, abs(q[1].x - q[0].x) - 2, abs(q[3].y - q[0].y) - 2);
		Mat crop = orig(quad_crop);
				
		ocr->SetImage(crop.data, crop.cols, crop.rows, 1, crop.step);
		auto ocr_text = string(ocr->GetUTF8Text());

		
		int tmp_day  = get_day_of_week(smooth_text(ocr_text));
		if(tmp_day < 0)
		{
			if(day_of_week >= 0 && day_of_week < 5) {
				
				list[day_of_week].push_back(ocr_text);
			}
		} else
		{
			day_of_week = min(tmp_day, 5);
		}
	}

	ofstream ff("hrbek.txt");
	for(int i = 0; i < list.size(); ++i)
	{
		for(auto meal : list[i])
		{
		    replace(meal.begin(), meal.end(), '\n', ' ');
			ff << to_string(i+1) + ";" + meal + ";\n";
		}
	}
	ff.close();
	

#ifdef DEBUG
	for(const auto &c : list) 
		cout << "jidel: " << c.size() << endl;
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
