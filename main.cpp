#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

cv::Point2f computeIntersect(cv::Vec4i a, cv::Vec4i b, int min_d);

void blah(const std::vector<cv::Vec4i> &lines, const cv::Mat &img2);

bool check_dist(const std::map<int,std::vector<int>> &dists, const cv::Point2f &p);

int main(int argc, char *argv[]) {

	if(argc != 2) {
		std::cerr << "No input file. Bailing out...\n";
		return -1;
	}

	std::ifstream fs;
	fs.open(argv[1]); //, std::fstream::in);

	cv::Mat image;
	image = cv::imread(argv[1], CV_8UC1);

//	cv::GaussianBlur(image,image,cv::Size(3,3),0);
	adaptiveThreshold(image, image,255,CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY,75,10);
	cv::bitwise_not(image, image);


	std::vector<cv::Vec4i> lines;
	HoughLinesP(image, lines, 2, CV_PI/180, 180, 300, 9);

	if(!image.data) {
		std::cerr << "Could not read the image\n";
		return -2;
	}


	// std::cout << "lines: " << lines.size() << std::endl;
	// for(const auto &c : lines)
	// {
	//	  line(image,
	//		   cv::Point2i(c[0],c[1]),
	//		   cv::Point2i(c[2], c[3]),
	//		   cv::Scalar(0, 255, 255));
	// }


	blah(lines, image);

	cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE );// Create a window for display.
	cv::imshow( "Display window", image );					 // Show our image inside it.

	cv::waitKey(0);

	/*
	  char c;
	  while (fs.get(c))			 // loop getting single characters
	  std::cout << c;

	*/
	fs.close();

	return 0;
}

cv::Point2f computeIntersect(cv::Vec4i a, cv::Vec4i b, int d)
{
	int x1 = a[0], y1 = a[1], x2 = a[2], y2 = a[3];
	int x3 = b[0], y3 = b[1], x4 = b[2], y4 = b[3];
	if (float d = ((float)(x1-x2) * (y3-y4)) - ((y1-y2) * (x3-x4)))
	{
		cv::Point2f pt;
		pt.x = ((x1*y2 - y1*x2) * (x3-x4) - (x1-x2) * (x3*y4 - y3*x4)) / d;
		pt.y = ((x1*y2 - y1*x2) * (y3-y4) - (y1-y2) * (x3*y4 - y3*x4)) / d;
		//-10 is a threshold, the POI can be off by at most 10 pixels
		if(pt.x<std::min(x1,x2)-d||pt.x>std::max(x1,x2)+d||pt.y<std::min(y1,y2)-d||pt.y>std::max(y1,y2)+d){
			return cv::Point2f(-1,-1);
		}
		if(pt.x<std::min(x3,x4)-d||pt.x>std::max(x3,x4)+d||pt.y<std::min(y3,y4)-d||pt.y>std::max(y3,y4)+d){
			return cv::Point2f(-1,-1);
		}
		return pt;
	}
	else
		return cv::Point2f(-1, -1);
}

void blah(const std::vector<cv::Vec4i> &lines, const cv::Mat &img2)
{
	int* poly = new int[lines.size()];
	for(int i=0;i<lines.size();i++)poly[i] = - 1;
	int curPoly = 0;
	std::vector<std::vector<cv::Point2f> > corners;

	std::map<int,std::vector<int>> dists;

	for (int i = 0; i < lines.size(); i++)
	{
		for (int j = i+1; j < lines.size(); j++)
		{

			cv::Point2f pt = computeIntersect(lines[i], lines[j], 10);
			if (pt.x >= 0 && pt.y >= 0&&pt.x<img2.size().width&&pt.y<img2.size().height){

				if(poly[i]==-1&&poly[j] == -1){
					std::vector<cv::Point2f> v;
					v.push_back(pt);
					if(!check_dist(dists, pt)) {
						corners.push_back(v);
						dists[pt.x].push_back(pt.y);
					}
					poly[i] = curPoly;
					poly[j] = curPoly;
					curPoly++;
					continue;
				}
				if(poly[i]==-1&&poly[j]>=0){
					if(!check_dist(dists, pt)) {
						corners[poly[j]].push_back(pt);
						dists[pt.x].push_back(pt.y);
					}
					poly[i] = poly[j];
					continue;
				}
				if(poly[i]>=0&&poly[j]==-1){
					if(!check_dist(dists, pt)) {
						corners[poly[i]].push_back(pt);
						dists[pt.x].push_back(pt.y);
					}
					poly[j] = poly[i];
					continue;
				}
				if(poly[i]>=0&&poly[j]>=0){
					if(poly[i]==poly[j]){
						if(!check_dist(dists, pt)) {
							corners[poly[i]].push_back(pt);
							dists[pt.x].push_back(pt.y);
						}
						continue;
					}

					for(int k=0;k<corners[poly[j]].size();k++){
						auto val = corners[poly[j]][k];
						if(!check_dist(dists, val)) {
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
			circle(img2, c, 5, cv::Scalar(255,255,255), CV_FILLED, 8,0);

			std::cout << "[" << c.x << ", " << c.y << "]" << std::endl;
		}

	std::cout << "number of corners: " << nc << "(map x: " << dists.size() << ")\n";
}

bool check_dist(const std::map<int,std::vector<int>> &dists, const cv::Point2f &p)
{
	for(int pi = std::max(0, static_cast<int>(p.x - 10)); pi < p.x + 10; pi++)
	{
		if(dists.find(pi) != dists.end())
		{
			for(int ys : dists.at(pi))
			{
				if((ys < (p.y + 10)) &&
				   (ys > static_cast<int>(std::max(0, static_cast<int>(p.y - 10)))))
				{
					return true;
				}
			}
		}
	}

	return false;
}
