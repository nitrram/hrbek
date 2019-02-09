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

Point2f computeIntersect(Vec4i a, Vec4i b, int min_d);

void blah(const vector<Vec4i> &lines, const Mat &img2);

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
//	HoughLinesP(image, lines, 2, CV_PI/180, 180, 300, 9);


	for(const auto &c : corners) {
		circle(image, c, 5, Scalar(255,255,255), FILLED, 8,0);
	}

	if(!image.data) {
		cerr << "Could not read the image\n";
		return -2;
	}


	// cout << "lines: " << lines.size() << endl;
	// for(const auto &c : lines)
	// {
	//	  line(image,
	//		   Point2i(c[0],c[1]),
	//		   Point2i(c[2], c[3]),
	//		   Scalar(0, 255, 255));
	// }


//	blah(lines, image);

	namedWindow( "Display window", WINDOW_AUTOSIZE );// Create a window for display.
	imshow( "Display window", image );					 // Show our image inside it.

	waitKey(0);

	/*
	  char c;
	  while (fs.get(c))			 // loop getting single characters
	  cout << c;

	*/
	fs.close();

	return 0;
}

Point2f computeIntersect(Vec4i a, Vec4i b, int d)
{
	int x1 = a[0], y1 = a[1], x2 = a[2], y2 = a[3];
	int x3 = b[0], y3 = b[1], x4 = b[2], y4 = b[3];
	if (float d = ((float)(x1-x2) * (y3-y4)) - ((y1-y2) * (x3-x4)))
	{
		Point2f pt;
		pt.x = ((x1*y2 - y1*x2) * (x3-x4) - (x1-x2) * (x3*y4 - y3*x4)) / d;
		pt.y = ((x1*y2 - y1*x2) * (y3-y4) - (y1-y2) * (x3*y4 - y3*x4)) / d;
		//-10 is a threshold, the POI can be off by at most 10 pixels
		if(pt.x<min(x1,x2)-d||pt.x>max(x1,x2)+d||pt.y<min(y1,y2)-d||pt.y>max(y1,y2)+d){
			return Point2f(-1,-1);
		}
		if(pt.x<min(x3,x4)-d||pt.x>max(x3,x4)+d||pt.y<min(y3,y4)-d||pt.y>max(y3,y4)+d){
			return Point2f(-1,-1);
		}
		return pt;
	}
	else
		return Point2f(-1, -1);
}

void blah(const vector<Vec4i> &lines, const Mat &img2)
{
	int* poly = new int[lines.size()];
	for(int i=0;i<lines.size();i++)poly[i] = - 1;
	int curPoly = 0;
	vector<vector<Point2f> > corners;

	map<int,vector<int>> dists;
	const int d = 10;

	for (int i = 0; i < lines.size(); i++)
	{
		for (int j = i+1; j < lines.size(); j++)
		{

			Point2f pt = computeIntersect(lines[i], lines[j], 10);
			if (pt.x >= 0 && pt.y >= 0&&pt.x<img2.size().width&&pt.y<img2.size().height){

				if(poly[i]==-1&&poly[j] == -1){
					vector<Point2f> v;
					v.push_back(pt);
					if(!check_dist(dists, pt, d)) {
						corners.push_back(v);
						dists[pt.x].push_back(pt.y);
					}
					poly[i] = curPoly;
					poly[j] = curPoly;
					curPoly++;
					continue;
				}
				if(poly[i]==-1&&poly[j]>=0){
					if(!check_dist(dists, pt, d)) {
						corners[poly[j]].push_back(pt);
						dists[pt.x].push_back(pt.y);
					}
					poly[i] = poly[j];
					continue;
				}
				if(poly[i]>=0&&poly[j]==-1){
					if(!check_dist(dists, pt, d)) {
						corners[poly[i]].push_back(pt);
						dists[pt.x].push_back(pt.y);
					}
					poly[j] = poly[i];
					continue;
				}
				if(poly[i]>=0&&poly[j]>=0){
					if(poly[i]==poly[j]){
						if(!check_dist(dists, pt, d)) {
							corners[poly[i]].push_back(pt);
							dists[pt.x].push_back(pt.y);
						}
						continue;
					}

					for(int k=0;k<corners[poly[j]].size();k++){
						auto val = corners[poly[j]][k];
						if(!check_dist(dists, val, d)) {
							corners[poly[i]].push_back(val);
							dists[val.x].push_back(val.y);
						}
					}

					corners[poly[j]].clear();
					poly[j] = poly[i];
					continue;
				}
			}
		}
	}

	size_t nc = 0;

	for(const auto &v : corners)
		for(const auto &c : v) {
			nc++;
			circle(img2, c, 5, Scalar(255,255,255), FILLED, 8,0);

			cout << "[" << c.x << ", " << c.y << "]" << endl;
		}

	cout << "number of corners: " << nc << "(map x: " << dists.size() << ")\n";
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
