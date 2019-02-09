#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <chrono>

using namespace cv;
using namespace std;

bool check_dist(const map<int,vector<int>> &dists, const Point2f &p, int d);

void compute_corners(vector<Point2f> &corners, const Mat &img);

int main(int argc, char *argv[]) {

	if(argc != 2) {
		cerr << "No input file. Bailing out...\n";
		return -1;
	}

	ifstream fs;
	fs.open(argv[1]); //, fstream::in);

	Mat image;
	image = imread(argv[1], CV_8UC1);

//	GaussianBlur(image,image,Size(3,3),0);
	adaptiveThreshold(image, image,255,ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY,75,10);
	bitwise_not(image, image);


	vector<Vec4i> lines;
	vector<Point2f> corners;

	auto begin = chrono::high_resolution_clock::now();
	compute_corners(corners, image);
	auto end = chrono::high_resolution_clock::now();

	cout << "corners: " << corners.size() << " in " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << " [ms]\n";

	for(const auto &c : corners) {
		circle(image, c, 5, Scalar(255,255,255), FILLED, 8,0);
	}

	if(!image.data) {
		cerr << "Could not read the image\n";
		return -2;
	}

	namedWindow( "Display window", WINDOW_AUTOSIZE );// Create a window for display.
	imshow( "Display window", image );					 // Show our image inside it.

	waitKey(0);

	fs.close();

	return 0;
}

bool check_dist(const map<int,vector<int>> &dists, const Point2f &p, int d)
{
	for(int pi = max(0, static_cast<int>(p.x - d)); pi < p.x + d; pi++)
	{
		if(dists.find(pi) != dists.end())
		{
			for(int ys : dists.at(pi))
			{
				if((ys < (p.y + d)) &&
				   (ys > static_cast<int>(max(0, static_cast<int>(p.y - d)))))
				{
					return true;
				}
			}
		}
	}

	return false;
}

void compute_corners(vector<Point2f> &corners, const Mat &img)
{
	vector<pair<int,int>> last_row = vector<pair<int,int>>(img.cols);
	pair<int,int> last_col;

	map<int, vector<int>> dists;

	const int d = 30;
	const int anchor = 10;


	for(int i =0; i < img.rows; ++i) {
		for(int j = 0; j < img.cols; ++j) {

			int col = 0;
			int row = 0;
			if((img.at<uchar>(i,j) & 0xff) > 0) {
				col = 1+last_col.first;
				row = 1+last_row[j].second;
			}
			//TODO
			//<1,0><2,0><3,1><4,0><5,0><6,0><7,0>
			//<0,0><0,0><0,2>

			bool irow,icol,icross;
			if(
				(irow = (((row == 0) || (i == (img.rows-1))) &&
						 (last_row[j].second > 100))) ||
				(icross = ((last_row[j].second > d) && (last_col.first > d)))
			) {

				if(irow) {
					auto p = Point2f(j,i-last_row[j].second);
					if(!check_dist(dists, p, d))
					{
						cout << "[" << p.x << ", " << p.y << "]\n";
						corners.push_back(p);
						dists[p.x].push_back(p.y);
					}

					if(j >= img.cols-anchor) {
						p = Point2f(img.cols-j, p.y);
						if(!check_dist(dists, p, d)) {
							corners.push_back(p);
							dists[p.x].push_back(p.y);
						}
					}
				}
				else if(icross) {
					auto p = Point2f(j,i);
					if(!check_dist(dists, p, d)) {
						corners.push_back(p);
						dists[p.x].push_back(p.y);

						p = Point2f(j,i-last_row[j].second);
						if(!check_dist(dists, p, d))
						{
							corners.push_back(p);
							dists[p.x].push_back(p.y);
						}
					}

					if(j >= img.cols-anchor) {
						p = Point2f(img.cols-j, p.y);
						if(!check_dist(dists, p, d)) {
							corners.push_back(p);
							dists[p.x].push_back(p.y);
						}
					}
				}
			}

			last_col = std::make_pair(col, row);
			last_row[j] = last_col;
		}
		last_col = make_pair(0,0); // reset
	}
}
