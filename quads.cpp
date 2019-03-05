#include <vector>
#include <algorithm>
#include <map>
#include <iostream>

#include <opencv2/core/core.hpp>

using namespace cv;
using namespace std;

extern bool check_dist(const map<int,vector<int>> &dists, const Point2f &p, int d)
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

extern map<int,vector<int>> sort_corners(vector<Point2f> &corners)
{
	map<int, vector<int>> out;
	sort(corners.begin(), corners.end(), [](const Point2f &p0, const Point2f &p1)
		 {
			 return (p0.y < p1.y || (p0.y == p1.y && p0.x < p1.x));
		 });

	// 2px y-axis correction
	int y = -1;
	for(auto &p : corners) {
		if(p.y - 2 <= y) p.y = y;

		out[p.y].push_back(p.x);
		y = p.y;
	}

	for(auto &a : out)
	{
		sort(a.second.begin(), a.second.end());
	}

	return out;
}

extern void compute_corners(vector<Point2f> &corners, const Mat &img)
{
	vector<pair<int,int>> last_row = vector<pair<int,int>>(img.cols);
	pair<int,int> last_col;

	map<int, vector<int>> dists;

	const int d = 40;
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

extern vector<vector<Point2f>> generate_quads(map<int, vector<int>> &&corners)
{
	vector<vector<Point2f>> quads;
	for(auto it = corners.begin(); it != corners.end() && next(it) != corners.end(); ++it)
	{
		auto cur = it->second;
		auto nex = next(it)->second;

		int ix = -1;
		for(int i : cur) {

			bool cntr = false;
			for(int n : nex) {

				if(abs(i - n) < 2) {
					cntr = true;
					break;
				}
			}

			if(cntr) {
				if(ix > 0) {
					quads.push_back(
						{
							Point2f(ix, it->first), //left-top
							Point2f(i, it->first), //right-top
							Point2f(i, next(it)->first), //right-bottom
							Point2f(ix, next(it)->first) //left-bottom
						}
					);
//					cout << "i: " << i << " qu[" << quads.size() << "]\n";
				}

				ix = i;
			}
		}
	}

	return quads;
}
